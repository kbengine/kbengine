/*
 * Copyright 2013 MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "mongoc-config.h"

#ifdef MONGOC_ENABLE_SASL
#include <sasl/sasl.h>
#include <sasl/saslutil.h>
#endif
#include <string.h>

#include "mongoc-cluster-private.h"
#include "mongoc-client-private.h"
#include "mongoc-counters-private.h"
#include "mongoc-config.h"
#include "mongoc-error.h"
#include "mongoc-host-list-private.h"
#include "mongoc-log.h"
#ifdef MONGOC_ENABLE_SASL
#include "mongoc-sasl-private.h"
#endif
#include "mongoc-b64-private.h"
#include "mongoc-scram-private.h"
#include "mongoc-set-private.h"
#include "mongoc-socket.h"
#include "mongoc-stream-private.h"
#include "mongoc-stream-socket.h"
#include "mongoc-thread-private.h"
#include "mongoc-topology-private.h"
#include "mongoc-trace.h"
#include "mongoc-util-private.h"
#include "mongoc-write-concern-private.h"
#include "mongoc-uri-private.h"
#ifdef MONGOC_ENABLE_SSL
#include "mongoc-stream-tls.h"
#endif

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "cluster"


#define CHECK_CLOSED_DURATION_MSEC 1000

#define DB_AND_CMD_FROM_COLLECTION(outstr, name) \
   do { \
      const char *dot = strchr(name, '.'); \
      if (!dot || ((dot - name) > (sizeof outstr - 6))) { \
         bson_snprintf(outstr, sizeof outstr, "admin.$cmd"); \
      } else { \
         memcpy(outstr, name, dot - name); \
         memcpy(outstr + (dot - name), ".$cmd", 6); \
      } \
   } while (0)

static mongoc_server_stream_t *
mongoc_cluster_fetch_stream_single (mongoc_cluster_t *cluster,
                                    mongoc_server_description_t *sd,
                                    bool reconnect_ok,
                                    bson_error_t *error);

static mongoc_server_stream_t *
mongoc_cluster_fetch_stream_pooled (mongoc_cluster_t *cluster,
                                    mongoc_server_description_t *sd,
                                    bool reconnect_ok,
                                    bson_error_t *error);

static void
_bson_error_message_printf (bson_error_t *error,
                            const char *format,
                            ...) BSON_GNUC_PRINTF (2, 3);

/* Allows caller to safely overwrite error->message with a formatted string,
 * even if the formatted string includes original error->message. */
static void
_bson_error_message_printf (bson_error_t *error,
                            const char *format,
                            ...)
{
   va_list args;
   char error_message[sizeof error->message];

   if (error) {
      va_start (args, format);
      bson_vsnprintf (error_message, sizeof error->message, format, args);
      va_end (args);

      bson_strncpy (error->message, error_message, sizeof error->message);
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_run_command_rpc --
 *
 *       Internal function to run a command on a given stream and
 *       read the response into @reply_rpc. @rpc and @reply_rpc can be
 *       the same to reuse storage. @buffer should be initialized before
 *       passing it in.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       On success, @buffer and @reply_rpc are filled out with the reply.
 *       On failure, @error is filled out. If this was a network error
 *       and server_id is nonzero, the cluster disconnects from the server.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_cluster_run_command_rpc (mongoc_cluster_t    *cluster,
                                mongoc_stream_t     *stream,
                                uint32_t             server_id,
                                const char          *command_name,
                                mongoc_rpc_t        *rpc,
                                mongoc_rpc_t        *reply_rpc,
                                mongoc_buffer_t     *buffer,
                                bson_error_t        *error)
{
   mongoc_array_t ar;
   int32_t msg_len;
   bool error_set = false;
   bool ret = false;
   char db[MONGOC_NAMESPACE_MAX];

   ENTRY;

   BSON_ASSERT(cluster);
   BSON_ASSERT(stream);

   _mongoc_array_init (&ar, sizeof (mongoc_iovec_t));

   if (cluster->client->in_exhaust) {
      bson_set_error(error,
                     MONGOC_ERROR_CLIENT,
                     MONGOC_ERROR_CLIENT_IN_EXHAUST,
                     "A cursor derived from this client is in exhaust.");
      error_set = true;
      GOTO (done);
   }

   rpc->query.request_id = ++cluster->request_id;
   _mongoc_rpc_gather (rpc, &ar);
   _mongoc_rpc_swab_to_le (rpc);

   if (!_mongoc_stream_writev_full (stream, (mongoc_iovec_t *)ar.data, ar.len,
                                   cluster->sockettimeoutms, error) ||
       !_mongoc_buffer_append_from_stream (buffer, stream, 4,
                                           cluster->sockettimeoutms, error)) {

      mongoc_cluster_disconnect_node (cluster, server_id);

      /* add info about the command to writev_full's error message */
      _mongoc_get_db_name (rpc->query.collection, db);
      _bson_error_message_printf (
         error,
         "Failed to send \"%s\" command with database \"%s\": %s",
         command_name, db, error->message);

      error_set = true;
      GOTO (done);
   }

   BSON_ASSERT (buffer->len == 4);

   memcpy (&msg_len, buffer->data, 4);
   msg_len = BSON_UINT32_FROM_LE(msg_len);
   if ((msg_len < 16) || (msg_len > MONGOC_DEFAULT_MAX_MSG_SIZE)) {
      GOTO (done);
   }

   if (!_mongoc_buffer_append_from_stream (buffer, stream, (size_t) msg_len - 4,
                                           cluster->sockettimeoutms, error)) {
      error_set = true;
      GOTO (done);
   }

   if (!_mongoc_rpc_scatter (reply_rpc, buffer->data, buffer->len)) {
      GOTO (done);
   }

   _mongoc_rpc_swab_from_le (reply_rpc);

   if (reply_rpc->header.opcode != MONGOC_OPCODE_REPLY) {
      GOTO (done);
   }

   ret = true;

done:
   _mongoc_array_destroy (&ar);

   if (!ret && !error_set) {
      /* generic error */
      bson_set_error (error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Invalid reply from server.");
   }

   RETURN (ret);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_run_command --
 *
 *       Internal function to run a command on a given stream.
 *       @error and @reply are optional out-pointers.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @reply is set and should ALWAYS be released with bson_destroy().
 *       On failure, @error is filled out. If this was a network error
 *       and server_id is nonzero, the cluster disconnects from the server.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_cluster_run_command (mongoc_cluster_t    *cluster,
                            mongoc_stream_t     *stream,
                            uint32_t             server_id,
                            mongoc_query_flags_t flags,
                            const char          *db_name,
                            const bson_t        *command,
                            bson_t              *reply,
                            bson_error_t        *error)
{
   char ns[MONGOC_NAMESPACE_MAX];
   mongoc_rpc_t rpc;
   bson_t reply_local;
   bool ret = false;
   bool reply_local_initialized = false;
   mongoc_buffer_t buffer;

   _mongoc_buffer_init (&buffer, NULL, 0, NULL, NULL);

   bson_snprintf (ns, sizeof ns, "%s.$cmd", db_name);

   _mongoc_rpc_prep_command (&rpc, ns, command, flags);

   /* we can reuse the query rpc for the reply */
   if (!mongoc_cluster_run_command_rpc (cluster, stream, server_id,
                                        _mongoc_get_command_name (command),
                                        &rpc, &rpc, &buffer, error)) {
      GOTO (done);
   }

   /* static-init reply_local to point into buffer */
   if (!_mongoc_rpc_reply_get_first(&rpc.reply, &reply_local)) {
      bson_set_error (error,
                      MONGOC_ERROR_BSON,
                      MONGOC_ERROR_BSON_INVALID,
                      "Failed to decode reply BSON document.");
      GOTO (done);
   }

   reply_local_initialized = true;

   if (_mongoc_rpc_parse_command_error (&rpc, error)) {
      GOTO (done);
   }

   ret = true;

done:
   if (reply && reply_local_initialized) {
      bson_copy_to (&reply_local, reply);
      bson_destroy (&reply_local);
   } else if (reply) {
      bson_init (reply);
   }

   _mongoc_buffer_destroy (&buffer);

   RETURN (ret);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_stream_run_ismaster --
 *
 *       Run an ismaster command on the given stream.
 *
 * Returns:
 *       True if ismaster ran successfully.
 *
 * Side effects:
 *       Makes a blocking I/O call and fills out @reply on success,
 *       or @error on failure.
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_stream_run_ismaster (mongoc_cluster_t *cluster,
                             mongoc_stream_t *stream,
                             bson_t *reply,
                             bson_error_t *error)
{
   bson_t command;
   bool ret;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);
   BSON_ASSERT (reply);
   BSON_ASSERT (error);

   bson_init (&command);
   bson_append_int32 (&command, "ismaster", 8, 1);

   ret = mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                     "admin", &command, reply, error);

   bson_destroy (&command);

   RETURN (ret);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_run_ismaster --
 *
 *       Run an ismaster command for the given node and handle result.
 *
 * Returns:
 *       True if ismaster ran successfully, false otherwise.
 *
 * Side effects:
 *       Makes a blocking I/O call.
 *
 *--------------------------------------------------------------------------
 */
static bool
_mongoc_cluster_run_ismaster (mongoc_cluster_t *cluster,
                              mongoc_cluster_node_t *node)
{
   bson_t reply;
   bson_error_t error;
   bson_iter_t iter;
   int num_fields = 0;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (node);
   BSON_ASSERT (node->stream);

   if (!_mongoc_stream_run_ismaster (cluster, node->stream, &reply, &error)) {
      GOTO (failure);
   }

   bson_iter_init (&iter, &reply);

   while (bson_iter_next (&iter)) {
      num_fields++;
      if (strcmp ("maxWriteBatchSize", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         node->max_write_batch_size = bson_iter_int32 (&iter);
      } else if (strcmp ("minWireVersion", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         node->min_wire_version = bson_iter_int32 (&iter);
      } else if (strcmp ("maxWireVersion", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         node->max_wire_version = bson_iter_int32 (&iter);
      } else if (strcmp ("maxBsonObjSize", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         node->max_bson_obj_size = bson_iter_int32 (&iter);
      } else if (strcmp ("maxMessageSizeBytes", bson_iter_key (&iter)) == 0) {
         if (! BSON_ITER_HOLDS_INT32 (&iter)) goto failure;
         node->max_msg_size = bson_iter_int32 (&iter);
      }
   }

   if (num_fields == 0) goto failure;

   /* TODO: run ismaster through the topology machinery? */
   bson_destroy (&reply);

   return true;

failure:

   bson_destroy (&reply);

   return false;
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_build_basic_auth_digest --
 *
 *       Computes the Basic Authentication digest using the credentials
 *       configured for @cluster and the @nonce provided.
 *
 *       The result should be freed by the caller using bson_free() when
 *       they are finished with it.
 *
 * Returns:
 *       A newly allocated string containing the digest.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static char *
_mongoc_cluster_build_basic_auth_digest (mongoc_cluster_t *cluster,
                                         const char       *nonce)
{
   const char *username;
   const char *password;
   char *password_digest;
   char *password_md5;
   char *digest_in;
   char *ret;

   ENTRY;

   /*
    * The following generates the digest to be used for basic authentication
    * with a MongoDB server. More information on the format can be found
    * at the following location:
    *
    * http://docs.mongodb.org/meta-driver/latest/legacy/
    *   implement-authentication-in-driver/
    */

   BSON_ASSERT(cluster);
   BSON_ASSERT(cluster->uri);

   username = mongoc_uri_get_username(cluster->uri);
   password = mongoc_uri_get_password(cluster->uri);
   password_digest = bson_strdup_printf("%s:mongo:%s", username, password);
   password_md5 = _mongoc_hex_md5(password_digest);
   digest_in = bson_strdup_printf("%s%s%s", nonce, username, password_md5);
   ret = _mongoc_hex_md5(digest_in);
   bson_free(digest_in);
   bson_free(password_md5);
   bson_free(password_digest);

   RETURN(ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_auth_node_cr --
 *
 *       Performs authentication of @node using the credentials provided
 *       when configuring the @cluster instance.
 *
 *       This is the Challenge-Response mode of authentication.
 *
 * Returns:
 *       true if authentication was successful; otherwise false and
 *       @error is set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_cluster_auth_node_cr (mongoc_cluster_t      *cluster,
                              mongoc_stream_t       *stream,
                              bson_error_t          *error)
{
   bson_iter_t iter;
   const char *auth_source;
   bson_t command = { 0 };
   bson_t reply = { 0 };
   char *digest;
   char *nonce;
   bool ret;

   ENTRY;

   BSON_ASSERT(cluster);
   BSON_ASSERT(stream);

   if (!(auth_source = mongoc_uri_get_auth_source(cluster->uri)) ||
       (*auth_source == '\0')) {
      auth_source = "admin";
   }

   /*
    * To authenticate a node using basic authentication, we need to first
    * get the nonce from the server. We use that to hash our password which
    * is sent as a reply to the server. If everything went good we get a
    * success notification back from the server.
    */

   /*
    * Execute the getnonce command to fetch the nonce used for generating
    * md5 digest of our password information.
    */
   bson_init (&command);
   bson_append_int32 (&command, "getnonce", 8, 1);
   if (!mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                    auth_source, &command, &reply, error)) {
      bson_destroy (&command);
      bson_destroy (&reply);
      RETURN (false);
   }
   bson_destroy (&command);
   if (!bson_iter_init_find_case (&iter, &reply, "nonce")) {
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_GETNONCE,
                      "Invalid reply from getnonce");
      bson_destroy (&reply);
      RETURN (false);
   }

   /*
    * Build our command to perform the authentication.
    */
   nonce = bson_iter_dup_utf8(&iter, NULL);
   digest = _mongoc_cluster_build_basic_auth_digest(cluster, nonce);
   bson_init(&command);
   bson_append_int32(&command, "authenticate", 12, 1);
   bson_append_utf8(&command, "user", 4,
                    mongoc_uri_get_username(cluster->uri), -1);
   bson_append_utf8(&command, "nonce", 5, nonce, -1);
   bson_append_utf8(&command, "key", 3, digest, -1);
   bson_destroy(&reply);
   bson_free(nonce);
   bson_free(digest);

   /*
    * Execute the authenticate command. mongoc_cluster_run_command
    * checks for {ok: 1} in the response.
    */
   ret = mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                     auth_source, &command, &reply, error);
   if (!ret) {
      /* error->message is already set */
      error->domain = MONGOC_ERROR_CLIENT;
      error->code = MONGOC_ERROR_CLIENT_AUTHENTICATE;
   }

   bson_destroy (&command);
   bson_destroy (&reply);

   RETURN (ret);
}


#ifdef MONGOC_ENABLE_SASL
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_get_canonicalized_name --
 *
 *       Query the node to get the canonicalized name. This may happen if
 *       the node has been accessed via an alias.
 *
 *       The gssapi code will use this if canonicalizeHostname is true.
 *
 *       Some underlying layers of krb might do this for us, but they can
 *       be disabled in krb.conf.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_cluster_get_canonicalized_name (mongoc_cluster_t      *cluster, /* IN */
                                        mongoc_stream_t       *node_stream,    /* IN */
                                        char                  *name,    /* OUT */
                                        size_t                 namelen, /* IN */
                                        bson_error_t          *error)   /* OUT */
{
   mongoc_stream_t *stream;
   mongoc_stream_t *tmp;
   mongoc_socket_t *sock = NULL;
   char *canonicalized;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (node_stream);
   BSON_ASSERT (name);

   /*
    * Find the underlying socket used in the stream chain.
    */
   for (stream = node_stream; stream;) {
      if ((tmp = mongoc_stream_get_base_stream (stream))) {
         stream = tmp;
         continue;
      }
      break;
   }

   BSON_ASSERT (stream);

   if (stream->type == MONGOC_STREAM_SOCKET) {
      sock = mongoc_stream_socket_get_socket ((mongoc_stream_socket_t *)stream);
      if (sock) {
         canonicalized = mongoc_socket_getnameinfo (sock);
         if (canonicalized) {
            bson_snprintf (name, namelen, "%s", canonicalized);
            bson_free (canonicalized);
            RETURN (true);
         }
      }
   }

   RETURN (false);
}
#endif


#ifdef MONGOC_ENABLE_SASL
/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_auth_node_sasl --
 *
 *       Perform authentication for a cluster node using SASL. This is
 *       only supported for GSSAPI at the moment.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       error may be set.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_cluster_auth_node_sasl (mongoc_cluster_t *cluster,
                                mongoc_stream_t  *stream,
                                const char       *hostname,
                                bson_error_t     *error)
{
   uint32_t buflen = 0;
   mongoc_sasl_t sasl;
   const bson_t *options;
   bson_iter_t iter;
   bool ret = false;
   char real_name [BSON_HOST_NAME_MAX + 1];
   const char *service_name;
   const char *mechanism;
   const char *tmpstr;
   uint8_t buf[4096] = { 0 };
   bson_t cmd;
   bson_t reply;
   int conv_id = 0;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);

   options = mongoc_uri_get_options (cluster->uri);

   _mongoc_sasl_init (&sasl);

   if ((mechanism = mongoc_uri_get_auth_mechanism (cluster->uri))) {
      _mongoc_sasl_set_mechanism (&sasl, mechanism);
   }

   if (bson_iter_init_find_case (&iter, options, "gssapiservicename") &&
       BSON_ITER_HOLDS_UTF8 (&iter) &&
       (service_name = bson_iter_utf8 (&iter, NULL))) {
      _mongoc_sasl_set_service_name (&sasl, service_name);
   }

   _mongoc_sasl_set_pass (&sasl, mongoc_uri_get_password (cluster->uri));
   _mongoc_sasl_set_user (&sasl, mongoc_uri_get_username (cluster->uri));

   /*
    * If the URI requested canonicalizeHostname, we need to resolve the real
    * hostname for the IP Address and pass that to the SASL layer. Some
    * underlying GSSAPI layers will do this for us, but can be disabled in
    * their config (krb.conf).
    *
    * This allows the consumer to specify canonicalizeHostname=true in the URI
    * and have us do that for them.
    *
    * See CDRIVER-323 for more information.
    */
   if (bson_iter_init_find_case (&iter, options, "canonicalizeHostname") &&
       BSON_ITER_HOLDS_BOOL (&iter) &&
       bson_iter_bool (&iter)) {
      if (_mongoc_cluster_get_canonicalized_name (cluster, stream, real_name,
                                                  sizeof real_name, error)) {
         _mongoc_sasl_set_service_host (&sasl, real_name);
      } else {
         _mongoc_sasl_set_service_host (&sasl, hostname);
      }
   } else {
      _mongoc_sasl_set_service_host (&sasl, hostname);
   }

   for (;;) {
      if (!_mongoc_sasl_step (&sasl, buf, buflen, buf, sizeof buf, &buflen, error)) {
         goto failure;
      }

      bson_init (&cmd);

      if (sasl.step == 1) {
         BSON_APPEND_INT32 (&cmd, "saslStart", 1);
         BSON_APPEND_UTF8 (&cmd, "mechanism", mechanism ? mechanism : "GSSAPI");
         bson_append_utf8 (&cmd, "payload", 7, (const char *)buf, buflen);
         BSON_APPEND_INT32 (&cmd, "autoAuthorize", 1);
      } else {
         BSON_APPEND_INT32 (&cmd, "saslContinue", 1);
         BSON_APPEND_INT32 (&cmd, "conversationId", conv_id);
         bson_append_utf8 (&cmd, "payload", 7, (const char *)buf, buflen);
      }

      MONGOC_INFO ("SASL: authenticating \"%s\" (step %d)",
                   mongoc_uri_get_username (cluster->uri),
                   sasl.step);

      if (!mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                       "$external", &cmd, &reply, error)) {
         bson_destroy (&cmd);
         bson_destroy (&reply);
         goto failure;
      }

      bson_destroy (&cmd);

      if (bson_iter_init_find (&iter, &reply, "done") &&
          bson_iter_as_bool (&iter)) {
         bson_destroy (&reply);
         break;
      }

      if (!bson_iter_init_find (&iter, &reply, "conversationId") ||
          !BSON_ITER_HOLDS_INT32 (&iter) ||
          !(conv_id = bson_iter_int32 (&iter)) ||
          !bson_iter_init_find (&iter, &reply, "payload") ||
          !BSON_ITER_HOLDS_UTF8 (&iter)) {
         MONGOC_INFO ("SASL: authentication failed for \"%s\"",
                      mongoc_uri_get_username (cluster->uri));
         bson_destroy (&reply);
         bson_set_error (error,
                         MONGOC_ERROR_CLIENT,
                         MONGOC_ERROR_CLIENT_AUTHENTICATE,
                         "Received invalid SASL reply from MongoDB server.");
         goto failure;
      }

      tmpstr = bson_iter_utf8 (&iter, &buflen);

      if (buflen > sizeof buf) {
         bson_set_error (error,
                         MONGOC_ERROR_CLIENT,
                         MONGOC_ERROR_CLIENT_AUTHENTICATE,
                         "SASL reply from MongoDB is too large.");

         bson_destroy (&reply);
         goto failure;
      }

      memcpy (buf, tmpstr, buflen);

      bson_destroy (&reply);
   }

   MONGOC_INFO ("SASL: \"%s\" authenticated",
                mongoc_uri_get_username (cluster->uri));

   ret = true;

failure:
   _mongoc_sasl_destroy (&sasl);

   return ret;
}
#endif


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_auth_node_plain --
 *
 *       Perform SASL PLAIN authentication for @node. We do this manually
 *       instead of using the SASL module because its rather simplistic.
 *
 * Returns:
 *       true if successful; otherwise false and error is set.
 *
 * Side effects:
 *       error may be set.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_cluster_auth_node_plain (mongoc_cluster_t      *cluster,
                                 mongoc_stream_t       *stream,
                                 bson_error_t          *error)
{
   char buf[4096];
   int buflen = 0;
   const char *username;
   const char *password;
   bson_t b = BSON_INITIALIZER;
   bson_t reply;
   size_t len;
   char *str;
   bool ret;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);

   username = mongoc_uri_get_username (cluster->uri);
   if (!username) {
      username = "";
   }

   password = mongoc_uri_get_password (cluster->uri);
   if (!password) {
      password = "";
   }

   str = bson_strdup_printf ("%c%s%c%s", '\0', username, '\0', password);
   len = strlen (username) + strlen (password) + 2;
   buflen = mongoc_b64_ntop ((const uint8_t *) str, len, buf, sizeof buf);
   bson_free (str);

   if (buflen == -1) {
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "failed base64 encoding message");
      return false;
   }

   BSON_APPEND_INT32 (&b, "saslStart", 1);
   BSON_APPEND_UTF8 (&b, "mechanism", "PLAIN");
   bson_append_utf8 (&b, "payload", 7, (const char *)buf, buflen);
   BSON_APPEND_INT32 (&b, "autoAuthorize", 1);

   ret = mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                     "$external", &b, &reply, error);

   if (!ret) {
      /* error->message is already set */
      error->domain = MONGOC_ERROR_CLIENT;
      error->code = MONGOC_ERROR_CLIENT_AUTHENTICATE;
   }

   bson_destroy (&b);
   bson_destroy (&reply);

   return ret;
}


#ifdef MONGOC_ENABLE_SSL
static bool
_mongoc_cluster_auth_node_x509 (mongoc_cluster_t      *cluster,
                                mongoc_stream_t       *stream,
                                bson_error_t          *error)
{
   const char *username;
   bson_t cmd;
   bson_t reply;
   bool ret;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);

   username = mongoc_uri_get_username (cluster->uri);
   if (username) {
      MONGOC_INFO ("X509: got username (%s) from URI", username);
   } else {
      if (!cluster->client->ssl_opts.pem_file) {
         bson_set_error (error,
                         MONGOC_ERROR_CLIENT,
                         MONGOC_ERROR_CLIENT_AUTHENTICATE,
                         "cannot determine username for "
                         "X-509 authentication.");
         return false;
      }

      if (cluster->client->pem_subject) {
         username = cluster->client->pem_subject;
         MONGOC_INFO ("X509: got username (%s) from certificate", username);
      }
   }

   bson_init (&cmd);
   BSON_APPEND_INT32 (&cmd, "authenticate", 1);
   BSON_APPEND_UTF8 (&cmd, "mechanism", "MONGODB-X509");
   BSON_APPEND_UTF8 (&cmd, "user", username);

   ret = mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                     "$external", &cmd, &reply, error);

   if (!ret) {
      /* error->message is already set */
      error->domain = MONGOC_ERROR_CLIENT;
      error->code = MONGOC_ERROR_CLIENT_AUTHENTICATE;
   }

   bson_destroy (&cmd);
   bson_destroy (&reply);

   return ret;
}
#endif


#ifdef MONGOC_ENABLE_SSL
static bool
_mongoc_cluster_auth_node_scram (mongoc_cluster_t      *cluster,
                                 mongoc_stream_t       *stream,
                                 bson_error_t          *error)
{
   uint32_t buflen = 0;
   mongoc_scram_t scram;
   bson_iter_t iter;
   bool ret = false;
   const char *tmpstr;
   const char *auth_source;
   uint8_t buf[4096] = { 0 };
   bson_t cmd;
   bson_t reply;
   int conv_id = 0;
   bson_subtype_t btype;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);

   if (!(auth_source = mongoc_uri_get_auth_source(cluster->uri)) ||
       (*auth_source == '\0')) {
      auth_source = "admin";
   }

   _mongoc_scram_init(&scram);

   _mongoc_scram_set_pass (&scram, mongoc_uri_get_password (cluster->uri));
   _mongoc_scram_set_user (&scram, mongoc_uri_get_username (cluster->uri));

   for (;;) {
      if (!_mongoc_scram_step (&scram, buf, buflen, buf, sizeof buf, &buflen, error)) {
         goto failure;
      }

      bson_init (&cmd);

      if (scram.step == 1) {
         BSON_APPEND_INT32 (&cmd, "saslStart", 1);
         BSON_APPEND_UTF8 (&cmd, "mechanism", "SCRAM-SHA-1");
         bson_append_binary (&cmd, "payload", 7, BSON_SUBTYPE_BINARY, buf, buflen);
         BSON_APPEND_INT32 (&cmd, "autoAuthorize", 1);
      } else {
         BSON_APPEND_INT32 (&cmd, "saslContinue", 1);
         BSON_APPEND_INT32 (&cmd, "conversationId", conv_id);
         bson_append_binary (&cmd, "payload", 7, BSON_SUBTYPE_BINARY, buf, buflen);
      }

      MONGOC_INFO ("SCRAM: authenticating \"%s\" (step %d)",
                   mongoc_uri_get_username (cluster->uri),
                   scram.step);

      if (!mongoc_cluster_run_command (cluster, stream, 0, MONGOC_QUERY_SLAVE_OK,
                                       auth_source, &cmd, &reply, error)) {
         bson_destroy (&cmd);
         bson_destroy (&reply);

         /* error->message is already set */
         error->domain = MONGOC_ERROR_CLIENT;
         error->code = MONGOC_ERROR_CLIENT_AUTHENTICATE;
         goto failure;
      }

      bson_destroy (&cmd);

      if (bson_iter_init_find (&iter, &reply, "done") &&
          bson_iter_as_bool (&iter)) {
         bson_destroy (&reply);
         break;
      }

      if (!bson_iter_init_find (&iter, &reply, "conversationId") ||
          !BSON_ITER_HOLDS_INT32 (&iter) ||
          !(conv_id = bson_iter_int32 (&iter)) ||
          !bson_iter_init_find (&iter, &reply, "payload") ||
          !BSON_ITER_HOLDS_BINARY(&iter)) {
         const char *errmsg = "Received invalid SCRAM reply from MongoDB server.";

         MONGOC_INFO ("SCRAM: authentication failed for \"%s\"",
                      mongoc_uri_get_username (cluster->uri));

         if (bson_iter_init_find (&iter, &reply, "errmsg") &&
               BSON_ITER_HOLDS_UTF8 (&iter)) {
            errmsg = bson_iter_utf8 (&iter, NULL);
         }

         bson_set_error (error,
                         MONGOC_ERROR_CLIENT,
                         MONGOC_ERROR_CLIENT_AUTHENTICATE,
                         "%s", errmsg);
         bson_destroy (&reply);
         goto failure;
      }

      bson_iter_binary (&iter, &btype, &buflen, (const uint8_t**)&tmpstr);

      if (buflen > sizeof buf) {
         bson_set_error (error,
                         MONGOC_ERROR_CLIENT,
                         MONGOC_ERROR_CLIENT_AUTHENTICATE,
                         "SCRAM reply from MongoDB is too large.");
         bson_destroy (&reply);
         goto failure;
      }

      memcpy (buf, tmpstr, buflen);

      bson_destroy (&reply);
   }

   MONGOC_INFO ("SCRAM: \"%s\" authenticated",
                mongoc_uri_get_username (cluster->uri));

   ret = true;

failure:
   _mongoc_scram_destroy (&scram);

   return ret;
}
#endif


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_auth_node --
 *
 *       Authenticate a cluster node depending on the required mechanism.
 *
 * Returns:
 *       true if authenticated. false on failure and @error is set.
 *
 * Side effects:
 *       @error is set on failure.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_cluster_auth_node (mongoc_cluster_t *cluster,
                           mongoc_stream_t  *stream,
                           const char       *hostname,
                           int32_t           max_wire_version,
                           bson_error_t     *error)
{
   bool ret = false;
   const char *mechanism;
   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (stream);

   mechanism = mongoc_uri_get_auth_mechanism (cluster->uri);

   /* Use cached max_wire_version, not value from sd */
   if (!mechanism) {
      if (max_wire_version < WIRE_VERSION_SCRAM_DEFAULT) {
         mechanism = "MONGODB-CR";
      } else {
         mechanism = "SCRAM-SHA-1";
      }
   }

   if (0 == strcasecmp (mechanism, "MONGODB-CR")) {
      ret = _mongoc_cluster_auth_node_cr (cluster, stream, error);
   } else if (0 == strcasecmp (mechanism, "MONGODB-X509")) {
#ifdef MONGOC_ENABLE_SSL
      ret = _mongoc_cluster_auth_node_x509 (cluster, stream, error);
#else
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "The \"%s\" authentication mechanism requires libmongoc built with --enable-ssl",
                      mechanism);
#endif
   } else if (0 == strcasecmp (mechanism, "SCRAM-SHA-1")) {
#ifdef MONGOC_ENABLE_SSL
      ret = _mongoc_cluster_auth_node_scram (cluster, stream, error);
#else
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "The \"%s\" authentication mechanism requires libmongoc built with --enable-ssl",
                      mechanism);
#endif
   } else if (0 == strcasecmp (mechanism, "GSSAPI")) {
#ifdef MONGOC_ENABLE_SASL
      ret = _mongoc_cluster_auth_node_sasl (cluster, stream, hostname, error);
#else
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "The \"%s\" authentication mechanism requires libmongoc built with --enable-sasl",
                      mechanism);
#endif
   } else if (0 == strcasecmp (mechanism, "PLAIN")) {
      ret = _mongoc_cluster_auth_node_plain (cluster, stream, error);
   } else {
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_AUTHENTICATE,
                      "Unknown authentication mechanism \"%s\".",
                      mechanism);
   }

   if (!ret) {
      mongoc_counter_auth_failure_inc ();
      MONGOC_DEBUG("Authentication failed: %s", error->message);
   } else {
      mongoc_counter_auth_success_inc ();
      TRACE("%s", "Authentication succeeded");
   }

   RETURN(ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_disconnect_node --
 *
 *       Remove a node from the set of nodes. This should be done if
 *       a stream in the set is found to be invalid.
 *
 *       WARNING: pointers to a disconnected mongoc_cluster_node_t or
 *       its stream are now invalid, be careful of dangling pointers.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Removes node from cluster's set of nodes, and frees the
 *       mongoc_cluster_node_t if pooled.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_cluster_disconnect_node (mongoc_cluster_t *cluster, uint32_t server_id)
{
   mongoc_topology_t *topology = cluster->client->topology;
   ENTRY;

   if (topology->single_threaded) {
      mongoc_topology_scanner_node_t *scanner_node;

      scanner_node = mongoc_topology_scanner_get_node (topology->scanner, server_id);

      /* might never actually have connected */
      if (scanner_node && scanner_node->stream) {
         mongoc_topology_scanner_node_disconnect (scanner_node, true);
         EXIT;
      }
      EXIT;
   } else {
      mongoc_set_rm(cluster->nodes, server_id);
   }

   EXIT;
}

static void
_mongoc_cluster_node_destroy (mongoc_cluster_node_t *node)
{
   /* Failure, or Replica Set reconfigure without this node */
   mongoc_stream_failed (node->stream);

   bson_free (node);
}

static void
_mongoc_cluster_node_dtor (void *data_,
                           void *ctx_)
{
   mongoc_cluster_node_t *node = (mongoc_cluster_node_t *)data_;

   _mongoc_cluster_node_destroy (node);
}

static mongoc_cluster_node_t *
_mongoc_cluster_node_new (mongoc_stream_t *stream)
{
   mongoc_cluster_node_t *node;

   if (!stream) {
      return NULL;
   }

   node = (mongoc_cluster_node_t *)bson_malloc0(sizeof *node);

   node->stream = stream;
   node->timestamp = bson_get_monotonic_time ();

   node->max_wire_version = MONGOC_DEFAULT_WIRE_VERSION;
   node->min_wire_version = MONGOC_DEFAULT_WIRE_VERSION;

   node->max_write_batch_size = MONGOC_DEFAULT_WRITE_BATCH_SIZE;
   node->max_bson_obj_size = MONGOC_DEFAULT_BSON_OBJ_SIZE;
   node->max_msg_size = MONGOC_DEFAULT_MAX_MSG_SIZE;

   return node;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_add_node --
 *
 *       Add a new node to this cluster for the given server description.
 *
 *       NOTE: does NOT check if this server is already in the cluster.
 *
 * Returns:
 *       A stream connected to the server, or NULL on failure.
 *
 * Side effects:
 *       Adds a cluster node, or sets error on failure.
 *
 *--------------------------------------------------------------------------
 */
static mongoc_stream_t *
_mongoc_cluster_add_node (mongoc_cluster_t *cluster,
                          mongoc_server_description_t *sd,
                          bson_error_t *error /* OUT */)
{
   mongoc_cluster_node_t *cluster_node;
   mongoc_stream_t *stream;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (!cluster->client->topology->single_threaded);

   TRACE ("Adding new server to cluster: %s", sd->connection_address);

   stream = _mongoc_client_create_stream(cluster->client, &sd->host, error);
   if (!stream) {
      MONGOC_WARNING ("Failed connection to %s (%s)", sd->connection_address, error->message);
      RETURN (NULL);
   }

   /* take critical fields from a fresh ismaster */
   cluster_node = _mongoc_cluster_node_new (stream);
   if (!_mongoc_cluster_run_ismaster (cluster, cluster_node)) {
      _mongoc_cluster_node_destroy (cluster_node);
      MONGOC_WARNING ("Failed connection to %s (ismaster failed)", sd->connection_address);
      RETURN (NULL);
   }

   if (cluster->requires_auth) {
      if (!_mongoc_cluster_auth_node (cluster, cluster_node->stream, sd->host.host,
                                      cluster_node->max_wire_version, error)) {
         MONGOC_WARNING ("Failed authentication to %s (%s)", sd->connection_address, error->message);
         _mongoc_cluster_node_destroy (cluster_node);
         RETURN (NULL);
      }
   }

   mongoc_set_add (cluster->nodes, sd->id, cluster_node);

   RETURN (stream);
}

static void
node_not_found (mongoc_server_description_t *sd,
                bson_error_t *error /* OUT */)
{
   if (sd->error.code) {
      memcpy (error, &sd->error, sizeof *error);
   } else {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_NOT_ESTABLISHED,
                      "Could not find node %s",
                      sd->host.host_and_port);
   }
}

static void
stream_not_found (mongoc_server_description_t *sd,
                  bson_error_t *error /* OUT */)
{
   if (sd->error.code) {
      memcpy (error, &sd->error, sizeof *error);
   } else {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_NOT_ESTABLISHED,
                      "Could not find stream for node %s",
                      sd->host.host_and_port);
   }
}


static mongoc_server_stream_t *
_mongoc_cluster_stream_for_server_description (mongoc_cluster_t *cluster,
                                               mongoc_server_description_t *sd,
                                               bool reconnect_ok,
                                               bson_error_t *error)
{
   mongoc_topology_t *topology;
   mongoc_server_stream_t *server_stream;

   ENTRY;

   topology = cluster->client->topology;

   /* in the single-threaded use case we share topology's streams */
   if (topology->single_threaded) {
      server_stream = mongoc_cluster_fetch_stream_single (cluster,
                                                          sd,
                                                          reconnect_ok,
                                                          error);

   } else {
      server_stream = mongoc_cluster_fetch_stream_pooled (cluster,
                                                          sd,
                                                          reconnect_ok,
                                                          error);

   }

   if (!server_stream) {
      /* failed */
      mongoc_cluster_disconnect_node (cluster, sd->id);
   }

   RETURN (server_stream);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_stream_for_server --
 *
 *       Fetch the stream for @server_id. If @reconnect_ok and there is no
 *       valid stream, attempts to reconnect; if not @reconnect_ok then only
 *       an existing stream can be returned, or NULL.
 *
 * Returns:
 *       A mongoc_server_stream_t, or NULL
 *
 * Side effects:
 *       May add a node or reconnect one, if @reconnect_ok.
 *       Authenticates the stream if needed.
 *       May set @error.
 *
 *--------------------------------------------------------------------------
 */

mongoc_server_stream_t *
mongoc_cluster_stream_for_server (mongoc_cluster_t *cluster,
                                  uint32_t server_id,
                                  bool reconnect_ok,
                                  bson_error_t *error)
{
   mongoc_topology_t *topology;
   mongoc_server_description_t *sd;
   mongoc_server_stream_t *server_stream = NULL;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (server_id);

   topology = cluster->client->topology;

   if (!(sd = mongoc_topology_server_by_id (topology, server_id, error))) {
      RETURN (NULL);
   }

   server_stream = _mongoc_cluster_stream_for_server_description (cluster,
                                                                  sd,
                                                                  reconnect_ok,
                                                                  error);

   if (!server_stream) {
      /* failed */
      mongoc_cluster_disconnect_node (cluster, server_id);
      mongoc_server_description_destroy (sd);
   }

   RETURN (server_stream);
}


static mongoc_server_stream_t *
mongoc_cluster_fetch_stream_single (mongoc_cluster_t *cluster,
                                    mongoc_server_description_t *sd,
                                    bool reconnect_ok,
                                    bson_error_t *error)
{
   mongoc_topology_t *topology;
   mongoc_stream_t *stream;
   mongoc_topology_scanner_node_t *scanner_node;
   int64_t timeout_ms;
   int64_t expire_at;
   bson_t reply;

   topology = cluster->client->topology;
   timeout_ms = topology->connect_timeout_msec;

   scanner_node = mongoc_topology_scanner_get_node (topology->scanner, sd->id);
   BSON_ASSERT (scanner_node && !scanner_node->retired);
   stream = scanner_node->stream;

   if (!stream) {
      if (!reconnect_ok) {
         stream_not_found (sd, error);
         return NULL;
      }

      if (!mongoc_topology_scanner_node_setup (scanner_node, error)) {
         return NULL;
      }
      stream = scanner_node->stream;

      expire_at = bson_get_monotonic_time() + timeout_ms * 1000;

      if (!mongoc_stream_wait (stream, expire_at)) {
         bson_set_error (error,
                         MONGOC_ERROR_STREAM,
                         MONGOC_ERROR_STREAM_CONNECT,
                         "Failed to connect to target host: '%s'",
                         sd->host.host_and_port);
         memcpy (&sd->error, error, sizeof sd->error);
         mongoc_topology_scanner_node_disconnect (scanner_node, true);
         return NULL;
      }

#ifdef MONGOC_ENABLE_SSL
      if (scanner_node->ts->ssl_opts) {
         if (!mongoc_stream_tls_do_handshake (stream, (int32_t) timeout_ms)) {
            bson_set_error (error,
                            MONGOC_ERROR_STREAM,
                            MONGOC_ERROR_STREAM_SOCKET,
                            "TLS handshake failed.");
            mongoc_topology_scanner_node_disconnect (scanner_node, true);
            return NULL;
         }

         if (!mongoc_stream_tls_check_cert (stream, scanner_node->host.host)) {
            bson_set_error (error,
                            MONGOC_ERROR_STREAM,
                            MONGOC_ERROR_STREAM_SOCKET,
                            "Failed to verify peer certificate.");
            mongoc_topology_scanner_node_disconnect (scanner_node, true);
            return NULL;
         }
      }
#endif

      if (!_mongoc_stream_run_ismaster (cluster, stream, &reply, error)) {
         mongoc_topology_scanner_node_disconnect (scanner_node, true);
         return NULL;
      }

      /* TODO: run ismaster through the topology machinery? */
      bson_destroy (&reply);
   }

   /* if stream exists but isn't authed, a disconnect happened */
   if (cluster->requires_auth && !scanner_node->has_auth) {
      /* In single-threaded mode, we can use sd's max_wire_version */
      if (!_mongoc_cluster_auth_node (cluster, stream, sd->host.host,
                                      sd->max_wire_version, &sd->error)) {
         memcpy (error, &sd->error, sizeof *error);
         return NULL;
      }

      scanner_node->has_auth = true;
   }

   return mongoc_server_stream_new (topology->description.type, sd, stream);
}

static mongoc_server_stream_t *
mongoc_cluster_fetch_stream_pooled (mongoc_cluster_t *cluster,
                                    mongoc_server_description_t *sd,
                                    bool reconnect_ok,
                                    bson_error_t *error)
{
   mongoc_topology_t *topology;
   mongoc_stream_t *stream;
   mongoc_cluster_node_t *cluster_node;
   int64_t timestamp;

   cluster_node = (mongoc_cluster_node_t *) mongoc_set_get (cluster->nodes,
                                                            sd->id);

   topology = cluster->client->topology;

   if (cluster_node) {
      BSON_ASSERT (cluster_node->stream);

      /* existing cluster node, is it outdated? */
      timestamp = mongoc_topology_server_timestamp (topology, sd->id);
      if (timestamp == -1 || cluster_node->timestamp < timestamp) {
         mongoc_cluster_disconnect_node (cluster, sd->id);
      } else {
         /* TODO: thread safety! */
         return mongoc_server_stream_new (topology->description.type,
                                          sd, cluster_node->stream);
      }
   }

   /* no node, or out of date */
   if (!reconnect_ok) {
      node_not_found (sd, error);
      return NULL;
   }

   stream = _mongoc_cluster_add_node (cluster, sd, error);
   if (stream) {
      /* TODO: thread safety! */
      return mongoc_server_stream_new (topology->description.type,
                                       sd, stream);
   } else {
      return NULL;
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_init --
 *
 *       Initializes @cluster using the @uri and @client provided. The
 *       @uri is used to determine the "mode" of the cluster. Based on the
 *       uri we can determine if we are connected to a single host, a
 *       replicaSet, or a shardedCluster.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @cluster is initialized.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_cluster_init (mongoc_cluster_t   *cluster,
                     const mongoc_uri_t *uri,
                     void               *client)
{
   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (uri);

   memset (cluster, 0, sizeof *cluster);

   cluster->uri = mongoc_uri_copy(uri);
   cluster->client = (mongoc_client_t *)client;
   cluster->requires_auth = (mongoc_uri_get_username(uri) ||
                             mongoc_uri_get_auth_mechanism(uri));

   cluster->sockettimeoutms = mongoc_uri_get_option_as_int32(
      uri, "sockettimeoutms", MONGOC_DEFAULT_SOCKETTIMEOUTMS);

   cluster->socketcheckintervalms = mongoc_uri_get_option_as_int32(
      uri, "socketcheckintervalms", MONGOC_TOPOLOGY_SOCKET_CHECK_INTERVAL_MS);

   /* TODO for single-threaded case we don't need this */
   cluster->nodes = mongoc_set_new(8, _mongoc_cluster_node_dtor, NULL);

   _mongoc_array_init (&cluster->iov, sizeof (mongoc_iovec_t));

   EXIT;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_destroy --
 *
 *       Clean up after @cluster and destroy all active connections.
 *       All resources for @cluster are released.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       Everything.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_cluster_destroy (mongoc_cluster_t *cluster) /* INOUT */
{
   ENTRY;

   BSON_ASSERT (cluster);

   mongoc_uri_destroy(cluster->uri);

   mongoc_set_destroy(cluster->nodes);

   _mongoc_array_destroy(&cluster->iov);

   EXIT;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_stream_for_optype --
 *
 *       Internal server selection.
 *
 * Returns:
 *       A mongoc_server_stream_t on which you must call
 *       mongoc_server_stream_cleanup, or NULL on failure (sets @error)
 *
 * Side effects:
 *       May set @error.
 *       May add new nodes to @cluster->nodes.
 *
 *--------------------------------------------------------------------------
 */

static mongoc_server_stream_t *
_mongoc_cluster_stream_for_optype (mongoc_cluster_t *cluster,
                                   mongoc_ss_optype_t optype,
                                   const mongoc_read_prefs_t *read_prefs,
                                   bson_error_t *error)
{
   mongoc_server_stream_t *server_stream;
   mongoc_server_description_t *selected_server;
   mongoc_topology_t *topology = cluster->client->topology;

   ENTRY;

   BSON_ASSERT (cluster);

   /* this is a new copy of the server description */
   selected_server = mongoc_topology_select (topology,
                                            optype,
                                            read_prefs,
                                            15,
                                            error);

   if (!selected_server) {
      RETURN(NULL);
   }

   /* connect or reconnect to server if necessary */
   server_stream = _mongoc_cluster_stream_for_server_description (
      cluster, selected_server,
      true /* reconnect_ok */, error);

   if (!server_stream ) {
      mongoc_server_description_destroy (selected_server);
      RETURN (NULL);
   }

   RETURN (server_stream);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_stream_for_reads --
 *
 *       Internal server selection.
 *
 * Returns:
 *       A mongoc_server_stream_t on which you must call
 *       mongoc_server_stream_cleanup, or NULL on failure (sets @error)
 *
 * Side effects:
 *       May set @error.
 *       May add new nodes to @cluster->nodes.
 *
 *--------------------------------------------------------------------------
 */

mongoc_server_stream_t *
mongoc_cluster_stream_for_reads (mongoc_cluster_t *cluster,
                                 const mongoc_read_prefs_t *read_prefs,
                                 bson_error_t *error)
{
   return _mongoc_cluster_stream_for_optype (cluster, MONGOC_SS_READ,
                                             read_prefs, error);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_stream_for_writes --
 *
 *       Get a stream for write operations.
 *
 * Returns:
 *       A mongoc_server_stream_t on which you must call
 *       mongoc_server_stream_cleanup, or NULL on failure (sets @error)
 *
 * Side effects:
 *       May set @error.
 *       May add new nodes to @cluster->nodes.
 *
 *--------------------------------------------------------------------------
 */

mongoc_server_stream_t *
mongoc_cluster_stream_for_writes (mongoc_cluster_t *cluster,
                                  bson_error_t *error)
{
   return _mongoc_cluster_stream_for_optype (cluster, MONGOC_SS_WRITE,
                                             NULL, error);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_inc_egress_rpc --
 *
 *       Helper to increment the counter for a particular RPC based on
 *       it's opcode.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static BSON_INLINE void
_mongoc_cluster_inc_egress_rpc (const mongoc_rpc_t *rpc)
{
   mongoc_counter_op_egress_total_inc();

   switch (rpc->header.opcode) {
   case MONGOC_OPCODE_DELETE:
      mongoc_counter_op_egress_delete_inc();
      break;
   case MONGOC_OPCODE_UPDATE:
      mongoc_counter_op_egress_update_inc();
      break;
   case MONGOC_OPCODE_INSERT:
      mongoc_counter_op_egress_insert_inc();
      break;
   case MONGOC_OPCODE_KILL_CURSORS:
      mongoc_counter_op_egress_killcursors_inc();
      break;
   case MONGOC_OPCODE_GET_MORE:
      mongoc_counter_op_egress_getmore_inc();
      break;
   case MONGOC_OPCODE_REPLY:
      mongoc_counter_op_egress_reply_inc();
      break;
   case MONGOC_OPCODE_MSG:
      mongoc_counter_op_egress_msg_inc();
      break;
   case MONGOC_OPCODE_QUERY:
      mongoc_counter_op_egress_query_inc();
      break;
   default:
      BSON_ASSERT(false);
      break;
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_cluster_inc_ingress_rpc --
 *
 *       Helper to increment the counter for a particular RPC based on
 *       it's opcode.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static BSON_INLINE void
_mongoc_cluster_inc_ingress_rpc (const mongoc_rpc_t *rpc)
{
   mongoc_counter_op_ingress_total_inc ();

   switch (rpc->header.opcode) {
   case MONGOC_OPCODE_DELETE:
      mongoc_counter_op_ingress_delete_inc ();
      break;
   case MONGOC_OPCODE_UPDATE:
      mongoc_counter_op_ingress_update_inc ();
      break;
   case MONGOC_OPCODE_INSERT:
      mongoc_counter_op_ingress_insert_inc ();
      break;
   case MONGOC_OPCODE_KILL_CURSORS:
      mongoc_counter_op_ingress_killcursors_inc ();
      break;
   case MONGOC_OPCODE_GET_MORE:
      mongoc_counter_op_ingress_getmore_inc ();
      break;
   case MONGOC_OPCODE_REPLY:
      mongoc_counter_op_ingress_reply_inc ();
      break;
   case MONGOC_OPCODE_MSG:
      mongoc_counter_op_ingress_msg_inc ();
      break;
   case MONGOC_OPCODE_QUERY:
      mongoc_counter_op_ingress_query_inc ();
      break;
   default:
      BSON_ASSERT (false);
      break;
   }
}

static bool
_mongoc_cluster_min_of_max_obj_size_sds (void *item,
                                         void *ctx)
{
   mongoc_server_description_t *sd = (mongoc_server_description_t *)item;
   int32_t *current_min = (int32_t *)ctx;

   if (sd->max_bson_obj_size < *current_min) {
      *current_min = sd->max_bson_obj_size;
   }
   return true;
}

static bool
_mongoc_cluster_min_of_max_obj_size_nodes (void *item,
                                           void *ctx)
{
   mongoc_cluster_node_t *node = (mongoc_cluster_node_t *)item;
   int32_t *current_min = (int32_t *)ctx;

   if (node->max_bson_obj_size < *current_min) {
      *current_min = node->max_bson_obj_size;
   }
   return true;
}

static bool
_mongoc_cluster_min_of_max_msg_size_sds (void *item,
                                         void *ctx)
{
   mongoc_server_description_t *sd = (mongoc_server_description_t *)item;
   int32_t *current_min = (int32_t *)ctx;

   if (sd->max_msg_size < *current_min) {
      *current_min = sd->max_msg_size;
   }
   return true;
}

static bool
_mongoc_cluster_min_of_max_msg_size_nodes (void *item,
                                           void *ctx)
{
   mongoc_cluster_node_t *node = (mongoc_cluster_node_t *)item;
   int32_t *current_min = (int32_t *)ctx;

   if (node->max_msg_size < *current_min) {
      *current_min = node->max_msg_size;
   }
   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_get_max_bson_obj_size --
 *
 *      Return the minimum max_bson_obj_size across all servers in cluster.
 *
 *      NOTE: this method uses the topology's mutex.
 *
 * Returns:
 *      The minimum max_bson_obj_size.
 *
 * Side effects:
 *      None
 *
 *--------------------------------------------------------------------------
 */
int32_t
mongoc_cluster_get_max_bson_obj_size (mongoc_cluster_t *cluster)
{
   int32_t max_bson_obj_size = -1;

   max_bson_obj_size = MONGOC_DEFAULT_BSON_OBJ_SIZE;

   if (!cluster->client->topology->single_threaded) {
      mongoc_set_for_each (cluster->nodes,
                           _mongoc_cluster_min_of_max_obj_size_nodes,
                           &max_bson_obj_size);
   } else {
      mongoc_set_for_each (cluster->client->topology->description.servers,
                           _mongoc_cluster_min_of_max_obj_size_sds,
                           &max_bson_obj_size);
   }

   return max_bson_obj_size;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_get_max_msg_size --
 *
 *      Return the minimum max msg size across all servers in cluster.
 *
 *      NOTE: this method uses the topology's mutex.
 *
 * Returns:
 *      The minimum max_msg_size
 *
 * Side effects:
 *      None
 *
 *--------------------------------------------------------------------------
 */
int32_t
mongoc_cluster_get_max_msg_size (mongoc_cluster_t *cluster)
{
   int32_t max_msg_size = MONGOC_DEFAULT_MAX_MSG_SIZE;

   if (!cluster->client->topology->single_threaded) {
      mongoc_set_for_each (cluster->nodes,
                           _mongoc_cluster_min_of_max_msg_size_nodes,
                           &max_msg_size);
   } else {
      mongoc_set_for_each (cluster->client->topology->description.servers,
                           _mongoc_cluster_min_of_max_msg_size_sds,
                           &max_msg_size);
   }

   return max_msg_size;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_node_min_wire_version --
 *
 *      Return the min wire version for the given server.
 *
 * Returns:
 *      Min wire version, or -1 if server is not found.
 *
 *--------------------------------------------------------------------------
 */

int32_t
mongoc_cluster_node_min_wire_version (mongoc_cluster_t *cluster,
                                      uint32_t          server_id)
{
   mongoc_server_description_t *sd;
   mongoc_cluster_node_t *node;

   if (cluster->client->topology->single_threaded) {
      if ((sd = mongoc_topology_description_server_by_id (
         &cluster->client->topology->description, server_id, NULL))) {
         return sd->min_wire_version;
      }
   } else {
      if((node = (mongoc_cluster_node_t *)mongoc_set_get(cluster->nodes, server_id))) {
         return node->min_wire_version;
      }
   }

   return -1;
}

static bool
_mongoc_cluster_check_interval (mongoc_cluster_t *cluster,
                                uint32_t          server_id,
                                bson_error_t     *error)
{
   mongoc_topology_t *topology;
   mongoc_topology_scanner_node_t *scanner_node;
   mongoc_server_description_t *sd;
   mongoc_stream_t *stream;
   int64_t now;
   int64_t before_ismaster;
   bson_t command;
   bson_t reply;
   bool r;

   topology = cluster->client->topology;

   if (!topology->single_threaded) {
      return true;
   }

   scanner_node =
      mongoc_topology_scanner_get_node (topology->scanner, server_id);

   if (!scanner_node) {
      return false;
   }

   BSON_ASSERT (!scanner_node->retired);

   stream = scanner_node->stream;

   if (!stream) {
      return false;
   }

   now = bson_get_monotonic_time ();

   if (scanner_node->last_used + (1000 * CHECK_CLOSED_DURATION_MSEC) < now) {
      if (mongoc_stream_check_closed (stream)) {
         mongoc_cluster_disconnect_node (cluster, server_id);
         bson_set_error (error, MONGOC_ERROR_STREAM, MONGOC_ERROR_STREAM_SOCKET,
                         "Stream is closed");
         return false;
      }
   }

   if (scanner_node->last_used + (1000 * cluster->socketcheckintervalms) <
       now) {
      bson_init (&command);
      BSON_APPEND_INT32 (&command, "ismaster", 1);

      before_ismaster = now;
      r = mongoc_cluster_run_command (cluster, stream, server_id,
                                      MONGOC_QUERY_SLAVE_OK, "admin", &command,
                                      &reply, error);

      now = bson_get_monotonic_time ();

      bson_destroy (&command);

      if (r) {
         sd = mongoc_topology_description_server_by_id (&topology->description,
                                                        server_id, NULL);

         if (!sd) {
            bson_destroy (&reply);
            return false;
         }

         mongoc_topology_description_handle_ismaster (
            &topology->description, sd, &reply,
            (now - before_ismaster) / 1000,    /* RTT_MS */
            error);

         bson_destroy (&reply);
      } else {
         bson_destroy (&reply);
         return false;
      }
   }

   return true;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_sendv_to_server --
 *
 *       Sends the given RPCs to the given server.
 *
 * Returns:
 *       True if successful.
 *
 * Side effects:
 *       @rpcs may be mutated and should be considered invalid after calling
 *       this method.
 *
 *       @error may be set.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_cluster_sendv_to_server (mongoc_cluster_t              *cluster,
                                mongoc_rpc_t                  *rpcs,
                                size_t                         rpcs_len,
                                mongoc_server_stream_t        *server_stream,
                                const mongoc_write_concern_t  *write_concern,
                                bson_error_t                  *error)
{
   uint32_t server_id;
   mongoc_iovec_t *iov;
   mongoc_topology_scanner_node_t *scanner_node;
   const bson_t *b;
   mongoc_rpc_t gle;
   size_t iovcnt;
   size_t i;
   bool need_gle;
   char cmdname[140];
   int32_t max_msg_size;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (rpcs);
   BSON_ASSERT (rpcs_len);
   BSON_ASSERT (server_stream);

   server_id = server_stream->sd->id;

   if (cluster->client->in_exhaust) {
      bson_set_error(error,
                     MONGOC_ERROR_CLIENT,
                     MONGOC_ERROR_CLIENT_IN_EXHAUST,
                     "A cursor derived from this client is in exhaust.");
      RETURN(false);
   }

   if (! write_concern) {
      write_concern = cluster->client->write_concern;
   }

   if (!_mongoc_cluster_check_interval (cluster,
                                        server_stream->sd->id,
                                        error)) {
      RETURN (false);
   }

   _mongoc_array_clear(&cluster->iov);

   /*
    * TODO: We can probably remove the need for sendv and just do send since
    * we support write concerns now. Also, we clobber our getlasterror on
    * each subsequent mutation. It's okay, since it comes out correct anyway,
    * just useless work (and technically the request_id changes).
    */

   for (i = 0; i < rpcs_len; i++) {
      _mongoc_cluster_inc_egress_rpc (&rpcs[i]);
      rpcs[i].header.request_id = ++cluster->request_id;
      need_gle = _mongoc_rpc_needs_gle(&rpcs[i], write_concern);
      _mongoc_rpc_gather (&rpcs[i], &cluster->iov);

      max_msg_size = mongoc_server_stream_max_msg_size (server_stream);

      if (rpcs[i].header.msg_len > max_msg_size) {
         bson_set_error(error,
                        MONGOC_ERROR_CLIENT,
                        MONGOC_ERROR_CLIENT_TOO_BIG,
                        "Attempted to send an RPC larger than the "
                        "max allowed message size. Was %u, allowed %u.",
                        rpcs[i].header.msg_len,
                        max_msg_size);
         RETURN(false);
      }

      if (need_gle) {
         gle.query.msg_len = 0;
         gle.query.request_id = ++cluster->request_id;
         gle.query.response_to = 0;
         gle.query.opcode = MONGOC_OPCODE_QUERY;
         gle.query.flags = MONGOC_QUERY_NONE;

         switch (rpcs[i].header.opcode) {
         case MONGOC_OPCODE_INSERT:
            DB_AND_CMD_FROM_COLLECTION(cmdname, rpcs[i].insert.collection);
            break;
         case MONGOC_OPCODE_DELETE:
            DB_AND_CMD_FROM_COLLECTION(cmdname, rpcs[i].delete_.collection);
            break;
         case MONGOC_OPCODE_UPDATE:
            DB_AND_CMD_FROM_COLLECTION(cmdname, rpcs[i].update.collection);
            break;
         default:
            BSON_ASSERT(false);
            DB_AND_CMD_FROM_COLLECTION(cmdname, "admin.$cmd");
            break;
         }

         gle.query.collection = cmdname;
         gle.query.skip = 0;
         gle.query.n_return = 1;
         b = _mongoc_write_concern_get_gle((mongoc_write_concern_t *)write_concern);
         gle.query.query = bson_get_data(b);
         gle.query.fields = NULL;
         _mongoc_rpc_gather(&gle, &cluster->iov);
         _mongoc_rpc_swab_to_le(&gle);
      }

      _mongoc_rpc_swab_to_le(&rpcs[i]);
   }

   iov = (mongoc_iovec_t *)cluster->iov.data;
   iovcnt = cluster->iov.len;

   BSON_ASSERT (cluster->iov.len);

   if (!_mongoc_stream_writev_full (server_stream->stream, iov, iovcnt,
                                    cluster->sockettimeoutms, error)) {
      RETURN (false);
   }

   if (cluster->client->topology->single_threaded) {
      scanner_node =
         mongoc_topology_scanner_get_node (cluster->client->topology->scanner,
                                           server_id);

      if (scanner_node) {
         scanner_node->last_used = bson_get_monotonic_time ();
      }
   }

   RETURN (true);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_cluster_try_recv --
 *
 *       Tries to receive the next event from the MongoDB server.
 *       The contents are loaded into @buffer and then
 *       scattered into the @rpc structure. @rpc is valid as long as
 *       @buffer contains the contents read into it.
 *
 *       Callers that can optimize a reuse of @buffer should do so. It
 *       can save many memory allocations.
 *
 * Returns:
 *       True if successful.
 *
 * Side effects:
 *       @rpc is set on success, @error on failure.
 *       @buffer will be filled with the input data.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_cluster_try_recv (mongoc_cluster_t       *cluster,
                         mongoc_rpc_t           *rpc,
                         mongoc_buffer_t        *buffer,
                         mongoc_server_stream_t *server_stream,
                         bson_error_t           *error)
{
   uint32_t server_id;
   int32_t msg_len;
   int32_t max_msg_size;
   off_t pos;

   ENTRY;

   BSON_ASSERT (cluster);
   BSON_ASSERT (rpc);
   BSON_ASSERT (buffer);
   BSON_ASSERT (server_stream);

   server_id = server_stream->sd->id;

   TRACE ("Waiting for reply from server_id \"%u\"", server_id);

   /*
    * Buffer the message length to determine how much more to read.
    */
   pos = buffer->len;
   if (!_mongoc_buffer_append_from_stream (buffer, server_stream->stream, 4,
                                           cluster->sockettimeoutms, error)) {
      MONGOC_DEBUG("Could not read 4 bytes, stream probably closed or timed out");
      mongoc_counter_protocol_ingress_error_inc ();
      mongoc_cluster_disconnect_node(cluster, server_id);
      RETURN (false);
   }

   /*
    * Read the msg length from the buffer.
    */
   memcpy (&msg_len, &buffer->data[buffer->off + pos], 4);
   msg_len = BSON_UINT32_FROM_LE (msg_len);
   max_msg_size = mongoc_server_stream_max_msg_size (server_stream);
   if ((msg_len < 16) || (msg_len > max_msg_size)) {
      bson_set_error (error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Corrupt or malicious reply received.");
      mongoc_cluster_disconnect_node(cluster, server_id);
      mongoc_counter_protocol_ingress_error_inc ();
      RETURN (false);
   }

   /*
    * Read the rest of the message from the stream.
    */
   if (!_mongoc_buffer_append_from_stream (buffer, server_stream->stream,
                                           msg_len - 4,
                                           cluster->sockettimeoutms, error)) {
      mongoc_cluster_disconnect_node (cluster, server_id);
      mongoc_counter_protocol_ingress_error_inc ();
      RETURN (false);
   }

   /*
    * Scatter the buffer into the rpc structure.
    */
   if (!_mongoc_rpc_scatter (rpc, &buffer->data[buffer->off + pos], msg_len)) {
      bson_set_error (error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Failed to decode reply from server.");
      mongoc_cluster_disconnect_node (cluster, server_id);
      mongoc_counter_protocol_ingress_error_inc ();
      RETURN (false);
   }

   _mongoc_rpc_swab_from_le (rpc);

   _mongoc_cluster_inc_ingress_rpc (rpc);

   RETURN(true);
}
