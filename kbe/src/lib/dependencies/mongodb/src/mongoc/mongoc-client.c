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


#include <bson.h>
#ifndef _WIN32
# include <netdb.h>
# include <netinet/tcp.h>
#endif

#include "mongoc-cursor-array-private.h"
#include "mongoc-client-private.h"
#include "mongoc-collection-private.h"
#include "mongoc-config.h"
#include "mongoc-counters-private.h"
#include "mongoc-database-private.h"
#include "mongoc-gridfs-private.h"
#include "mongoc-error.h"
#include "mongoc-log.h"
#include "mongoc-queue-private.h"
#include "mongoc-socket.h"
#include "mongoc-stream-buffered.h"
#include "mongoc-stream-socket.h"
#include "mongoc-thread-private.h"
#include "mongoc-trace.h"
#include "mongoc-uri-private.h"
#include "mongoc-util-private.h"

#ifdef MONGOC_ENABLE_SSL
#include "mongoc-stream-tls.h"
#include "mongoc-ssl-private.h"
#endif


#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "client"


static void
_mongoc_client_op_killcursors (mongoc_cluster_t       *cluster,
                               mongoc_server_stream_t *server_stream,
                               int64_t                 cursor_id);

static void
_mongoc_client_killcursors_command (mongoc_cluster_t       *cluster,
                                    mongoc_server_stream_t *server_stream,
                                    int64_t                 cursor_id,
                                    const char             *db,
                                    const char             *collection);



/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_connect_tcp --
 *
 *       Connect to a host using a TCP socket.
 *
 *       This will be performed synchronously and return a mongoc_stream_t
 *       that can be used to connect with the remote host.
 *
 * Returns:
 *       A newly allocated mongoc_stream_t if successful; otherwise
 *       NULL and @error is set.
 *
 * Side effects:
 *       @error is set if return value is NULL.
 *
 *--------------------------------------------------------------------------
 */

static mongoc_stream_t *
mongoc_client_connect_tcp (const mongoc_uri_t       *uri,
                           const mongoc_host_list_t *host,
                           bson_error_t             *error)
{
   mongoc_socket_t *sock = NULL;
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int32_t connecttimeoutms;
   int64_t expire_at;
   char portstr [8];
   int s;

   ENTRY;

   BSON_ASSERT (uri);
   BSON_ASSERT (host);

   connecttimeoutms = mongoc_uri_get_option_as_int32 (
      uri, "connecttimeoutms", MONGOC_DEFAULT_CONNECTTIMEOUTMS);

   BSON_ASSERT (connecttimeoutms);
   expire_at = bson_get_monotonic_time () + (connecttimeoutms * 1000L);

   bson_snprintf (portstr, sizeof portstr, "%hu", host->port);

   memset (&hints, 0, sizeof hints);
   hints.ai_family = host->family;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = 0;
   hints.ai_protocol = 0;

   s = getaddrinfo (host->host, portstr, &hints, &result);

   if (s != 0) {
      mongoc_counter_dns_failure_inc ();
      bson_set_error(error,
                     MONGOC_ERROR_STREAM,
                     MONGOC_ERROR_STREAM_NAME_RESOLUTION,
                     "Failed to resolve %s",
                     host->host);
      RETURN (NULL);
   }

   mongoc_counter_dns_success_inc ();

   for (rp = result; rp; rp = rp->ai_next) {
      /*
       * Create a new non-blocking socket.
       */
      if (!(sock = mongoc_socket_new (rp->ai_family,
                                      rp->ai_socktype,
                                      rp->ai_protocol))) {
         continue;
      }

      /*
       * Try to connect to the peer.
       */
      if (0 != mongoc_socket_connect (sock,
                                      rp->ai_addr,
                                      (socklen_t)rp->ai_addrlen,
                                      expire_at)) {
         char *errmsg;
         char errmsg_buf[BSON_ERROR_BUFFER_SIZE];
         char ip[255];

         mongoc_socket_inet_ntop (rp, ip, sizeof ip);
         errmsg = bson_strerror_r (
            mongoc_socket_errno (sock), errmsg_buf, sizeof errmsg_buf);
         MONGOC_WARNING ("Failed to connect to: %s:%d, error: %d, %s\n",
                         ip,
                         host->port,
                         mongoc_socket_errno(sock),
                         errmsg);
         mongoc_socket_destroy (sock);
         sock = NULL;
         continue;
      }

      break;
   }

   if (!sock) {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_CONNECT,
                      "Failed to connect to target host: %s",
                      host->host_and_port);
      freeaddrinfo (result);
      RETURN (NULL);
   }

   freeaddrinfo (result);

   return mongoc_stream_socket_new (sock);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_connect_unix --
 *
 *       Connect to a MongoDB server using a UNIX domain socket.
 *
 * Returns:
 *       A newly allocated mongoc_stream_t if successful; otherwise
 *       NULL and @error is set.
 *
 * Side effects:
 *       @error is set if return value is NULL.
 *
 *--------------------------------------------------------------------------
 */

static mongoc_stream_t *
mongoc_client_connect_unix (const mongoc_uri_t       *uri,
                            const mongoc_host_list_t *host,
                            bson_error_t             *error)
{
#ifdef _WIN32
   ENTRY;
   bson_set_error (error,
                   MONGOC_ERROR_STREAM,
                   MONGOC_ERROR_STREAM_CONNECT,
                   "UNIX domain sockets not supported on win32.");
   RETURN (NULL);
#else
   struct sockaddr_un saddr;
   mongoc_socket_t *sock;
   mongoc_stream_t *ret = NULL;

   ENTRY;

   BSON_ASSERT (uri);
   BSON_ASSERT (host);

   memset (&saddr, 0, sizeof saddr);
   saddr.sun_family = AF_UNIX;
   bson_snprintf (saddr.sun_path, sizeof saddr.sun_path - 1,
                  "%s", host->host);

   sock = mongoc_socket_new (AF_UNIX, SOCK_STREAM, 0);

   if (sock == NULL) {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_SOCKET,
                      "Failed to create socket.");
      RETURN (NULL);
   }

   if (-1 == mongoc_socket_connect (sock,
                                    (struct sockaddr *)&saddr,
                                    sizeof saddr,
                                    -1)) {
      mongoc_socket_destroy (sock);
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_CONNECT,
                      "Failed to connect to UNIX domain socket.");
      RETURN (NULL);
   }

   ret = mongoc_stream_socket_new (sock);

   RETURN (ret);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_default_stream_initiator --
 *
 *       A mongoc_stream_initiator_t that will handle the various type
 *       of supported sockets by MongoDB including TCP and UNIX.
 *
 *       Language binding authors may want to implement an alternate
 *       version of this method to use their native stream format.
 *
 * Returns:
 *       A mongoc_stream_t if successful; otherwise NULL and @error is set.
 *
 * Side effects:
 *       @error is set if return value is NULL.
 *
 *--------------------------------------------------------------------------
 */

mongoc_stream_t *
mongoc_client_default_stream_initiator (const mongoc_uri_t       *uri,
                                        const mongoc_host_list_t *host,
                                        void                     *user_data,
                                        bson_error_t             *error)
{
   mongoc_stream_t *base_stream = NULL;
#ifdef MONGOC_ENABLE_SSL
   mongoc_client_t *client = (mongoc_client_t *)user_data;
   const char *mechanism;
   int32_t connecttimeoutms;
#endif

   BSON_ASSERT (uri);
   BSON_ASSERT (host);

#ifndef MONGOC_ENABLE_SSL
   if (mongoc_uri_get_ssl (uri)) {
      bson_set_error (error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_CLIENT_NO_ACCEPTABLE_PEER,
                      "SSL is not enabled in this build of mongo-c-driver.");
      return NULL;
   }
#endif


   switch (host->family) {
#if defined(AF_INET6)
   case AF_INET6:
#endif
   case AF_INET:
      base_stream = mongoc_client_connect_tcp (uri, host, error);
      break;
   case AF_UNIX:
      base_stream = mongoc_client_connect_unix (uri, host, error);
      break;
   default:
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_INVALID_TYPE,
                      "Invalid address family: 0x%02x", host->family);
      break;
   }

#ifdef MONGOC_ENABLE_SSL
   if (base_stream) {
      mechanism = mongoc_uri_get_auth_mechanism (uri);

      if (client->use_ssl ||
          (mechanism && (0 == strcmp (mechanism, "MONGODB-X509")))) {
         base_stream = mongoc_stream_tls_new (base_stream, &client->ssl_opts,
                                              true);

         if (!base_stream) {
            bson_set_error (error,
                            MONGOC_ERROR_STREAM,
                            MONGOC_ERROR_STREAM_SOCKET,
                            "Failed initialize TLS state.");
            return NULL;
         }

         connecttimeoutms = mongoc_uri_get_option_as_int32 (
            uri, "connecttimeoutms", MONGOC_DEFAULT_CONNECTTIMEOUTMS);

         if (!mongoc_stream_tls_do_handshake (base_stream, connecttimeoutms)) {
            bson_set_error (error,
                            MONGOC_ERROR_STREAM,
                            MONGOC_ERROR_STREAM_SOCKET,
                            "TLS handshake failed.");
            mongoc_stream_destroy (base_stream);
            return NULL;
         }

         if (!mongoc_stream_tls_check_cert (base_stream, host->host)) {
            bson_set_error (error,
                            MONGOC_ERROR_STREAM,
                            MONGOC_ERROR_STREAM_SOCKET,
                            "Failed to verify peer certificate.");
            mongoc_stream_destroy (base_stream);
            return NULL;
         }
      }
   }
#endif

   return base_stream ? mongoc_stream_buffered_new (base_stream, 1024) : NULL;
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_client_create_stream --
 *
 *       INTERNAL API
 *
 *       This function is used by the mongoc_cluster_t to initiate a
 *       new stream. This is done because cluster is private API and
 *       those using mongoc_client_t may need to override this process.
 *
 *       This function calls the default initiator for new streams.
 *
 * Returns:
 *       A newly allocated mongoc_stream_t if successful; otherwise
 *       NULL and @error is set.
 *
 * Side effects:
 *       @error is set if return value is NULL.
 *
 *--------------------------------------------------------------------------
 */

mongoc_stream_t *
_mongoc_client_create_stream (mongoc_client_t          *client,
                              const mongoc_host_list_t *host,
                              bson_error_t             *error)
{
   BSON_ASSERT (client);
   BSON_ASSERT (host);

   return client->initiator (client->uri, host, client->initiator_data, error);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_client_recv --
 *
 *       Receives a RPC from a remote MongoDB cluster node.
 *
 * Returns:
 *       true if successful; otherwise false and @error is set.
 *
 * Side effects:
 *       @error is set if return value is false.
 *
 *--------------------------------------------------------------------------
 */

bool
_mongoc_client_recv (mongoc_client_t        *client,
                     mongoc_rpc_t           *rpc,
                     mongoc_buffer_t        *buffer,
                     mongoc_server_stream_t *server_stream,
                     bson_error_t           *error)
{
   BSON_ASSERT (client);
   BSON_ASSERT (rpc);
   BSON_ASSERT (buffer);
   BSON_ASSERT (server_stream);

   if (!mongoc_cluster_try_recv (&client->cluster, rpc, buffer,
                                 server_stream, error)) {
      mongoc_topology_invalidate_server (client->topology,
                                         server_stream->sd->id);
      return false;
   }
   return true;
}


/*
 *--------------------------------------------------------------------------
 *
 * _bson_to_error --
 *
 *       A helper routine to convert a bson document to a bson_error_t.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @error is set if non-null.
 *
 *--------------------------------------------------------------------------
 */

static void
_bson_to_error (const bson_t *b,
                bson_error_t *error)
{
   bson_iter_t iter;
   int code = 0;

   BSON_ASSERT (b);

   if (!error) {
      return;
   }

   if (bson_iter_init_find(&iter, b, "code") && BSON_ITER_HOLDS_INT32(&iter)) {
      code = bson_iter_int32(&iter);
   }

   if (bson_iter_init_find(&iter, b, "$err") && BSON_ITER_HOLDS_UTF8(&iter)) {
      bson_set_error(error,
                     MONGOC_ERROR_QUERY,
                     code,
                     "%s",
                     bson_iter_utf8(&iter, NULL));
      return;
   }

   if (bson_iter_init_find(&iter, b, "errmsg") && BSON_ITER_HOLDS_UTF8(&iter)) {
      bson_set_error(error,
                     MONGOC_ERROR_QUERY,
                     code,
                     "%s",
                     bson_iter_utf8(&iter, NULL));
      return;
   }

   bson_set_error(error,
                  MONGOC_ERROR_QUERY,
                  MONGOC_ERROR_QUERY_FAILURE,
                  "An unknown error ocurred on the server.");
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_recv_gle --
 *
 *       INTERNAL API
 *
 *       This function is used to receive the next RPC from a cluster
 *       node, expecting it to be the response to a getlasterror command.
 *
 *       The RPC is parsed into @error if it is an error and false is
 *       returned.
 *
 *       If the operation was successful, true is returned.
 *
 *       if @gle_doc is not NULL, then the actual response document for
 *       the gle command will be stored as an out parameter. The caller
 *       is responsible for freeing it in this case.
 *
 * Returns:
 *       true if getlasterror was success; otherwise false.
 *
 * Side effects:
 *       @gle_doc will be set if non NULL and a reply was received.
 *       @error if return value is false, and @gle_doc is set to NULL.
 *
 *--------------------------------------------------------------------------
 */

bool
_mongoc_client_recv_gle (mongoc_client_t        *client,
                         mongoc_server_stream_t *server_stream,
                         bson_t                **gle_doc,
                         bson_error_t           *error)
{
   mongoc_buffer_t buffer;
   mongoc_rpc_t rpc;
   bson_iter_t iter;
   bool ret = false;
   bson_t b;

   ENTRY;

   BSON_ASSERT (client);
   BSON_ASSERT (server_stream);

   if (gle_doc) {
      *gle_doc = NULL;
   }

   _mongoc_buffer_init (&buffer, NULL, 0, NULL, NULL);

   if (!mongoc_cluster_try_recv (&client->cluster, &rpc, &buffer,
                                 server_stream, error)) {

      mongoc_topology_invalidate_server (client->topology,
                                         server_stream->sd->id);

      GOTO (cleanup);
   }

   if (rpc.header.opcode != MONGOC_OPCODE_REPLY) {
      bson_set_error (error,
                      MONGOC_ERROR_PROTOCOL,
                      MONGOC_ERROR_PROTOCOL_INVALID_REPLY,
                      "Received message other than OP_REPLY.");
      GOTO (cleanup);
   }

   if (_mongoc_rpc_reply_get_first (&rpc.reply, &b)) {
      if ((rpc.reply.flags & MONGOC_REPLY_QUERY_FAILURE)) {
         _bson_to_error (&b, error);
         bson_destroy (&b);
         GOTO (cleanup);
      }

      if (gle_doc) {
         *gle_doc = bson_copy (&b);
      }

      if (!bson_iter_init_find (&iter, &b, "ok") ||
          BSON_ITER_HOLDS_DOUBLE (&iter)) {
        if (bson_iter_double (&iter) == 0.0) {
          _bson_to_error (&b, error);
        }
      }

      bson_destroy (&b);
      ret = true;
   }

cleanup:
   _mongoc_buffer_destroy (&buffer);

   RETURN (ret);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_new --
 *
 *       Create a new mongoc_client_t using the URI provided.
 *
 *       @uri should be a MongoDB URI string such as "mongodb://localhost/"
 *       More information on the format can be found at
 *       http://docs.mongodb.org/manual/reference/connection-string/
 *
 * Returns:
 *       A newly allocated mongoc_client_t or NULL if @uri_string is
 *       invalid.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */
mongoc_client_t *
mongoc_client_new(const char *uri_string)
{
   mongoc_topology_t *topology;
   mongoc_client_t   *client;
   mongoc_uri_t      *uri;


   if (!uri_string) {
      uri_string = "mongodb://127.0.0.1/";
   }

   if (!(uri = mongoc_uri_new (uri_string))) {
      return NULL;
   }

   topology = mongoc_topology_new(uri, true);

   client = _mongoc_client_new_from_uri (uri, topology);
   mongoc_uri_destroy (uri);

   return client;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_set_ssl_opts
 *
 *       set ssl opts for a client
 *
 * Returns:
 *       Nothing
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

#ifdef MONGOC_ENABLE_SSL
void
mongoc_client_set_ssl_opts (mongoc_client_t        *client,
                            const mongoc_ssl_opt_t *opts)
{

   BSON_ASSERT (client);
   BSON_ASSERT (opts);

   client->use_ssl = true;
   memcpy (&client->ssl_opts, opts, sizeof client->ssl_opts);

   bson_free (client->pem_subject);
   client->pem_subject = NULL;

   if (opts->pem_file) {
      client->pem_subject = _mongoc_ssl_extract_subject (opts->pem_file);
   }

   if (client->topology->single_threaded) {
      mongoc_topology_scanner_set_ssl_opts (client->topology->scanner,
                                            &client->ssl_opts);
   }
}
#endif


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_new_from_uri --
 *
 *       Create a new mongoc_client_t for a mongoc_uri_t.
 *
 * Returns:
 *       A newly allocated mongoc_client_t.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_client_t *
mongoc_client_new_from_uri (const mongoc_uri_t *uri)
{
   mongoc_topology_t *topology;

   topology = mongoc_topology_new (uri, true);

   return _mongoc_client_new_from_uri (uri, topology);
}

/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_client_new_from_uri --
 *
 *       Create a new mongoc_client_t for a mongoc_uri_t and a given
 *       topology object.
 *
 * Returns:
 *       A newly allocated mongoc_client_t.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_client_t *
_mongoc_client_new_from_uri (const mongoc_uri_t *uri, mongoc_topology_t *topology)
{
   mongoc_client_t *client;
   const mongoc_read_prefs_t *read_prefs;
   const mongoc_read_concern_t *read_concern;
   const mongoc_write_concern_t *write_concern;

   BSON_ASSERT (uri);

#ifndef MONGOC_ENABLE_SSL
   if (mongoc_uri_get_ssl (uri)) {
      MONGOC_ERROR ("Can't create SSL client, SSL not enabled in this build.");
      return NULL;
   }
#endif

   client = (mongoc_client_t *)bson_malloc0(sizeof *client);
   client->uri = mongoc_uri_copy (uri);
   client->request_id = rand ();
   client->initiator = mongoc_client_default_stream_initiator;
   client->initiator_data = client;
   client->topology = topology;

   write_concern = mongoc_uri_get_write_concern (client->uri);
   client->write_concern = mongoc_write_concern_copy (write_concern);

   read_concern = mongoc_uri_get_read_concern (client->uri);
   client->read_concern = mongoc_read_concern_copy (read_concern);

   read_prefs = mongoc_uri_get_read_prefs_t (client->uri);
   client->read_prefs = mongoc_read_prefs_copy (read_prefs);

   mongoc_cluster_init (&client->cluster, client->uri, client);

#ifdef MONGOC_ENABLE_SSL
   client->use_ssl = false;
   if (mongoc_uri_get_ssl (client->uri)) {
      /* sets use_ssl = true */
      mongoc_client_set_ssl_opts (client, mongoc_ssl_opt_get_default ());
   }
#endif

   mongoc_counter_clients_active_inc ();

   return client;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_destroy --
 *
 *       Destroys a mongoc_client_t and cleans up all resources associated
 *       with the client instance.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @client is destroyed.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_client_destroy (mongoc_client_t *client)
{
   if (client) {
#ifdef MONGOC_ENABLE_SSL
      bson_free (client->pem_subject);
#endif

      if (client->topology->single_threaded) {
         mongoc_topology_destroy(client->topology);
      }

      mongoc_write_concern_destroy (client->write_concern);
      mongoc_read_concern_destroy (client->read_concern);
      mongoc_read_prefs_destroy (client->read_prefs);
      mongoc_cluster_destroy (&client->cluster);
      mongoc_uri_destroy (client->uri);
      bson_free (client);

      mongoc_counter_clients_active_dec ();
      mongoc_counter_clients_disposed_inc ();
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_uri --
 *
 *       Fetch the URI used for @client.
 *
 * Returns:
 *       A mongoc_uri_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_uri_t *
mongoc_client_get_uri (const mongoc_client_t *client)
{
   BSON_ASSERT (client);

   return client->uri;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_database --
 *
 *       Fetches a newly allocated database structure to communicate with
 *       a database over @client.
 *
 *       @database should be a db name such as "test".
 *
 *       This structure should be freed when the caller is done with it
 *       using mongoc_database_destroy().
 *
 * Returns:
 *       A newly allocated mongoc_database_t.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_database_t *
mongoc_client_get_database (mongoc_client_t *client,
                            const char      *name)
{
   BSON_ASSERT (client);
   BSON_ASSERT (name);

   return _mongoc_database_new(client, name, client->read_prefs,
                               client->read_concern, client->write_concern);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_default_database --
 *
 *       Get the database named in the MongoDB connection URI, or NULL
 *       if none was specified in the URI.
 *
 *       This structure should be freed when the caller is done with it
 *       using mongoc_database_destroy().
 *
 * Returns:
 *       A newly allocated mongoc_database_t or NULL.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_database_t *
mongoc_client_get_default_database (mongoc_client_t *client)
{
   const char *db;

   BSON_ASSERT (client);
   db = mongoc_uri_get_database (client->uri);

   if (db) {
      return mongoc_client_get_database (client, db);
   }

   return NULL;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_collection --
 *
 *       This function returns a newly allocated collection structure.
 *
 *       @db should be the name of the database, such as "test".
 *       @collection should be the name of the collection such as "test".
 *
 *       The above would result in the namespace "test.test".
 *
 *       You should free this structure when you are done with it using
 *       mongoc_collection_destroy().
 *
 * Returns:
 *       A newly allocated mongoc_collection_t that should be freed with
 *       mongoc_collection_destroy().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_collection_t *
mongoc_client_get_collection (mongoc_client_t *client,
                              const char      *db,
                              const char      *collection)
{
   BSON_ASSERT (client);
   BSON_ASSERT (db);
   BSON_ASSERT (collection);

   return _mongoc_collection_new(client, db, collection, client->read_prefs,
                                 client->read_concern, client->write_concern);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_gridfs --
 *
 *       This function returns a newly allocated collection structure.
 *
 *       @db should be the name of the database, such as "test".
 *
 *       @prefix optional prefix for GridFS collection names, or NULL. Default
 *       is "fs", thus the default collection names for GridFS are "fs.files"
 *       and "fs.chunks".
 *
 * Returns:
 *       A newly allocated mongoc_gridfs_t that should be freed with
 *       mongoc_gridfs_destroy().
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_gridfs_t *
mongoc_client_get_gridfs (mongoc_client_t *client,
                          const char      *db,
                          const char      *prefix,
                          bson_error_t    *error)
{
   BSON_ASSERT (client);
   BSON_ASSERT (db);

   if (! prefix) {
      prefix = "fs";
   }

   return _mongoc_gridfs_new(client, db, prefix, error);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_write_concern --
 *
 *       Fetches the default write concern for @client.
 *
 * Returns:
 *       A mongoc_write_concern_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_write_concern_t *
mongoc_client_get_write_concern (const mongoc_client_t *client)
{
   BSON_ASSERT (client);

   return client->write_concern;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_set_write_concern --
 *
 *       Sets the default write concern for @client.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_client_set_write_concern (mongoc_client_t              *client,
                                 const mongoc_write_concern_t *write_concern)
{
   BSON_ASSERT (client);

   if (write_concern != client->write_concern) {
      if (client->write_concern) {
         mongoc_write_concern_destroy(client->write_concern);
      }
      client->write_concern = write_concern ?
         mongoc_write_concern_copy(write_concern) :
         mongoc_write_concern_new();
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_read_concern --
 *
 *       Fetches the default read concern for @client.
 *
 * Returns:
 *       A mongoc_read_concern_t that should not be modified or freed.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_read_concern_t *
mongoc_client_get_read_concern (const mongoc_client_t *client)
{
   BSON_ASSERT (client);

   return client->read_concern;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_set_read_concern --
 *
 *       Sets the default read concern for @client.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_client_set_read_concern (mongoc_client_t             *client,
                                const mongoc_read_concern_t *read_concern)
{
   BSON_ASSERT (client);

   if (read_concern != client->read_concern) {
      if (client->read_concern) {
         mongoc_read_concern_destroy (client->read_concern);
      }
      client->read_concern = read_concern ?
         mongoc_read_concern_copy (read_concern) :
         mongoc_read_concern_new ();
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_get_read_prefs --
 *
 *       Fetch the default read preferences for @client.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

const mongoc_read_prefs_t *
mongoc_client_get_read_prefs (const mongoc_client_t *client)
{
   BSON_ASSERT (client);

   return client->read_prefs;
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_set_read_prefs --
 *
 *       Set the default read preferences for @client.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_client_set_read_prefs (mongoc_client_t           *client,
                              const mongoc_read_prefs_t *read_prefs)
{
   BSON_ASSERT (client);

   if (read_prefs != client->read_prefs) {
      if (client->read_prefs) {
         mongoc_read_prefs_destroy(client->read_prefs);
      }
      client->read_prefs = read_prefs ?
         mongoc_read_prefs_copy(read_prefs) :
         mongoc_read_prefs_new(MONGOC_READ_PRIMARY);
   }
}

mongoc_cursor_t *
mongoc_client_command (mongoc_client_t           *client,
                       const char                *db_name,
                       mongoc_query_flags_t       flags,
                       uint32_t                   skip,
                       uint32_t                   limit,
                       uint32_t                   batch_size,
                       const bson_t              *query,
                       const bson_t              *fields,
                       const mongoc_read_prefs_t *read_prefs)
{
   char ns[MONGOC_NAMESPACE_MAX];

   BSON_ASSERT (client);
   BSON_ASSERT (db_name);
   BSON_ASSERT (query);

   if (!read_prefs) {
      read_prefs = client->read_prefs;
   }

   /*
    * Allow a caller to provide a fully qualified namespace
    */
   if (NULL == strstr (db_name, "$cmd")) {
      bson_snprintf (ns, sizeof ns, "%s.$cmd", db_name);
      db_name = ns;
   }

   return _mongoc_cursor_new (client, db_name, flags, skip, limit, batch_size,
                              true, query, fields, read_prefs, NULL);
}


/**
 * mongoc_client_command_simple:
 * @client: A mongoc_client_t.
 * @db_name: The namespace, such as "admin".
 * @command: The command to execute.
 * @read_prefs: The read preferences or NULL.
 * @reply: A location for the reply document or NULL.
 * @error: A location for the error, or NULL.
 *
 * This wrapper around mongoc_client_command() aims to make it simpler to
 * run a command and check the output result.
 *
 * false is returned if the command failed to be delivered or if the execution
 * of the command failed. For example, a command that returns {'ok': 0} will
 * result in this function returning false.
 *
 * To allow the caller to disambiguate between command execution failure and
 * failure to send the command, reply will always be set if non-NULL. The
 * caller should release this with bson_destroy().
 *
 * Returns: true if the command executed and resulted in success. Otherwise
 *   false and @error is set. @reply is always set, either to the resulting
 *   document or an empty bson document upon failure.
 */
bool
mongoc_client_command_simple (mongoc_client_t           *client,
                              const char                *db_name,
                              const bson_t              *command,
                              const mongoc_read_prefs_t *read_prefs,
                              bson_t                    *reply,
                              bson_error_t              *error)
{
   mongoc_cluster_t *cluster;
   mongoc_server_stream_t *server_stream;
   mongoc_apply_read_prefs_result_t result = READ_PREFS_RESULT_INIT;
   bool ret = false;

   ENTRY;

   BSON_ASSERT (client);
   BSON_ASSERT (db_name);
   BSON_ASSERT (command);

   cluster = &client->cluster;

   server_stream = mongoc_cluster_stream_for_reads (cluster, read_prefs, error);

   if (!server_stream) {
      if (reply) {
         bson_init (reply);
      }
      GOTO (done);
   }

   apply_read_preferences (read_prefs, server_stream, command,
                           MONGOC_QUERY_NONE, &result);

   ret = mongoc_cluster_run_command (cluster, server_stream->stream,
                                     server_stream->sd->id,
                                     result.flags, db_name,
                                     result.query_with_read_prefs,
                                     reply, error);

done:
   mongoc_server_stream_cleanup (server_stream);
   apply_read_prefs_result_cleanup (&result);

   RETURN (ret);
}

void
_mongoc_client_kill_cursor (mongoc_client_t *client,
                            uint32_t         server_id,
                            int64_t          cursor_id,
                            const char      *db,
                            const char      *collection)
{
   mongoc_server_stream_t *server_stream;

   ENTRY;

   BSON_ASSERT (client);
   BSON_ASSERT (cursor_id);

   /* don't attempt reconnect if server unavailable, and ignore errors */
   server_stream = mongoc_cluster_stream_for_server (&client->cluster,
                                                     server_id,
                                                     false /* reconnect_ok */,
                                                     NULL  /* error */);

   if (!server_stream) {
      return;
   }

   if (db && collection &&
       server_stream->sd->max_wire_version >=
       WIRE_VERSION_KILLCURSORS_CMD) {
      _mongoc_client_killcursors_command (&client->cluster, server_stream,
                                          cursor_id, db, collection);
   } else {
      _mongoc_client_op_killcursors (&client->cluster,
                                     server_stream,
                                     cursor_id);
   }

   mongoc_server_stream_cleanup (server_stream);

   EXIT;
}

static void
_mongoc_client_op_killcursors (mongoc_cluster_t       *cluster,
                               mongoc_server_stream_t *server_stream,
                               int64_t                 cursor_id)
{
   mongoc_rpc_t rpc = { { 0 } };

   rpc.kill_cursors.msg_len = 0;
   rpc.kill_cursors.request_id = 0;
   rpc.kill_cursors.response_to = 0;
   rpc.kill_cursors.opcode = MONGOC_OPCODE_KILL_CURSORS;
   rpc.kill_cursors.zero = 0;
   rpc.kill_cursors.cursors = &cursor_id;
   rpc.kill_cursors.n_cursors = 1;

   mongoc_cluster_sendv_to_server (cluster, &rpc, 1, server_stream,
                                   NULL, NULL);
}

static void
_mongoc_client_killcursors_command (mongoc_cluster_t       *cluster,
                                    mongoc_server_stream_t *server_stream,
                                    int64_t                 cursor_id,
                                    const char             *db,
                                    const char             *collection)
{
   bson_t command = BSON_INITIALIZER;
   bson_t child;

   bson_append_utf8 (&command, "killCursors", 11, collection, -1);
   bson_append_array_begin (&command, "cursors", 7, &child);
   bson_append_int64 (&child, "0", 1, cursor_id);
   bson_append_array_end (&command, &child);

   /* Find, getMore And killCursors Commands Spec: "The result from the
    * killCursors command MAY be safely ignored."
    */
   mongoc_cluster_run_command (cluster, server_stream->stream,
                               server_stream->sd->id,
                               MONGOC_QUERY_SLAVE_OK, db, &command,
                               NULL, NULL);

   bson_destroy (&command);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_client_kill_cursor --
 *
 *       Destroy a cursor on the server.
 *
 *       NOTE: this is only reliable when connected to a single mongod or
 *       mongos. If connected to a replica set, the driver attempts to
 *       kill the cursor on the primary. If connected to multiple mongoses
 *       the kill-cursors message is sent to a *random* mongos.
 *
 *       If no primary, mongos, or standalone server is known, return
 *       without attempting to reconnect.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_client_kill_cursor (mongoc_client_t *client,
                           int64_t          cursor_id)
{
   mongoc_topology_t *topology;
   mongoc_server_description_t *selected_server;
   mongoc_read_prefs_t *read_prefs;
   uint32_t server_id = 0;

   topology = client->topology;
   read_prefs = mongoc_read_prefs_new (MONGOC_READ_PRIMARY);

   mongoc_mutex_lock (&topology->mutex);

   /* see if there's a known writable server - do no I/O or retries */
   selected_server = mongoc_topology_description_select(&topology->description,
                                                        MONGOC_SS_WRITE,
                                                        read_prefs,
                                                        15);

   if (selected_server) {
      server_id = selected_server->id;
   }

   mongoc_mutex_unlock (&topology->mutex);

   if (server_id) {
      _mongoc_client_kill_cursor (client, selected_server->id, cursor_id,
                                  NULL /* db */,
                                  NULL /* collection */);
   } else {
      MONGOC_INFO ("No server available for mongoc_client_kill_cursor");
   }

   mongoc_read_prefs_destroy (read_prefs);
}



char **
mongoc_client_get_database_names (mongoc_client_t *client,
                                  bson_error_t    *error)
{
   bson_iter_t iter;
   const char *name;
   char **ret = NULL;
   int i = 0;
   mongoc_cursor_t *cursor;
   const bson_t *doc;

   BSON_ASSERT (client);

   cursor = mongoc_client_find_databases (client, error);

   while (mongoc_cursor_next (cursor, &doc)) {
      if (bson_iter_init (&iter, doc) &&
          bson_iter_find (&iter, "name") &&
          BSON_ITER_HOLDS_UTF8 (&iter) &&
          (name = bson_iter_utf8 (&iter, NULL)) &&
          (0 != strcmp (name, "local"))) {
            ret = (char **)bson_realloc (ret, sizeof(char*) * (i + 2));
            ret [i] = bson_strdup (name);
            ret [++i] = NULL;
         }
   }

   if (!ret && !mongoc_cursor_error (cursor, error)) {
      ret = (char **)bson_malloc0 (sizeof (void*));
   }

   mongoc_cursor_destroy (cursor);

   return ret;
}


mongoc_cursor_t *
mongoc_client_find_databases (mongoc_client_t *client,
                              bson_error_t    *error)
{
   bson_t cmd = BSON_INITIALIZER;
   mongoc_cursor_t *cursor;

   BSON_ASSERT (client);

   BSON_APPEND_INT32 (&cmd, "listDatabases", 1);

   cursor = _mongoc_cursor_new (client, "admin", MONGOC_QUERY_SLAVE_OK,
                                0, 0, 0, true, NULL, NULL, NULL, NULL);

   _mongoc_cursor_array_init (cursor, &cmd, "databases");

   bson_destroy (&cmd);

   return cursor;
}


int32_t
mongoc_client_get_max_message_size (mongoc_client_t *client) /* IN */
{
   BSON_ASSERT (client);

   return mongoc_cluster_get_max_msg_size (&client->cluster);
}


int32_t
mongoc_client_get_max_bson_size (mongoc_client_t *client) /* IN */
{
   BSON_ASSERT (client);

   return mongoc_cluster_get_max_bson_obj_size (&client->cluster);
}


bool
mongoc_client_get_server_status (mongoc_client_t     *client,     /* IN */
                                 mongoc_read_prefs_t *read_prefs, /* IN */
                                 bson_t              *reply,      /* OUT */
                                 bson_error_t        *error)      /* OUT */
{
   bson_t cmd = BSON_INITIALIZER;
   bool ret = false;

   BSON_ASSERT (client);

   BSON_APPEND_INT32 (&cmd, "serverStatus", 1);
   ret = mongoc_client_command_simple (client, "admin", &cmd, read_prefs,
                                       reply, error);
   bson_destroy (&cmd);

   return ret;
}


void
mongoc_client_set_stream_initiator (mongoc_client_t           *client,
                                    mongoc_stream_initiator_t  initiator,
                                    void                      *user_data)
{
   BSON_ASSERT (client);

   if (!initiator) {
      initiator = mongoc_client_default_stream_initiator;
      user_data = client;
   } else {
      MONGOC_DEBUG ("Using custom stream initiator.");
   }

   client->initiator = initiator;
   client->initiator_data = user_data;

   if (client->topology->single_threaded) {
      mongoc_topology_scanner_set_stream_initiator (client->topology->scanner,
                                                    initiator, user_data);
   }
}
