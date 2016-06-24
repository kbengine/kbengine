/*
 * Copyright 2014 MongoDB, Inc.
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

#include "mongoc-error.h"
#include "mongoc-trace.h"
#include "mongoc-topology-scanner-private.h"
#include "mongoc-stream-socket.h"

#ifdef MONGOC_ENABLE_SSL
#include "mongoc-stream-tls.h"
#endif

#include "mongoc-counters-private.h"
#include "utlist.h"
#include "mongoc-topology-private.h"
#include "mongoc-host-list-private.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "topology_scanner"

static void
mongoc_topology_scanner_ismaster_handler (mongoc_async_cmd_result_t async_status,
                                          const bson_t             *ismaster_response,
                                          int64_t                   rtt_msec,
                                          void                     *data,
                                          bson_error_t             *error);

mongoc_topology_scanner_t *
mongoc_topology_scanner_new (const mongoc_uri_t          *uri,
                             mongoc_topology_scanner_cb_t cb,
                             void                        *data)
{
   mongoc_topology_scanner_t *ts = (mongoc_topology_scanner_t *)bson_malloc0 (sizeof (*ts));

   ts->async = mongoc_async_new ();
   bson_init (&ts->ismaster_cmd);
   BSON_APPEND_INT32 (&ts->ismaster_cmd, "isMaster", 1);

   ts->cb = cb;
   ts->cb_data = data;
   ts->uri = uri;

   return ts;
}

#ifdef MONGOC_ENABLE_SSL
void
mongoc_topology_scanner_set_ssl_opts (mongoc_topology_scanner_t *ts,
                                      mongoc_ssl_opt_t          *opts)
{
   ts->ssl_opts = opts;
   ts->setup = mongoc_async_cmd_tls_setup;
}
#endif

void
mongoc_topology_scanner_set_stream_initiator (mongoc_topology_scanner_t *ts,
                                              mongoc_stream_initiator_t  si,
                                              void                      *ctx)
{
   ts->initiator = si;
   ts->initiator_context = ctx;
   ts->setup = NULL;
}

void
mongoc_topology_scanner_destroy (mongoc_topology_scanner_t *ts)
{
   mongoc_topology_scanner_node_t *ele, *tmp;

   DL_FOREACH_SAFE (ts->nodes, ele, tmp) {
      mongoc_topology_scanner_node_destroy (ele, false);
   }

   mongoc_async_destroy (ts->async);
   bson_destroy (&ts->ismaster_cmd);

   bson_free (ts);
}

mongoc_topology_scanner_node_t *
mongoc_topology_scanner_add (mongoc_topology_scanner_t *ts,
                             const mongoc_host_list_t  *host,
                             uint32_t                   id)
{
   mongoc_topology_scanner_node_t *node;

   node = (mongoc_topology_scanner_node_t *) bson_malloc0 (sizeof (*node));

   memcpy (&node->host, host, sizeof (*host));

   node->id = id;
   node->ts = ts;
   node->last_failed = -1;

   DL_APPEND(ts->nodes, node);

   return node;
}

void
mongoc_topology_scanner_add_and_scan (mongoc_topology_scanner_t *ts,
                                      const mongoc_host_list_t  *host,
                                      uint32_t                   id,
                                      int64_t                    timeout_msec)
{
   mongoc_topology_scanner_node_t *node;

   BSON_ASSERT (timeout_msec < INT32_MAX);

   node = mongoc_topology_scanner_add (ts, host, id);

   /* begin non-blocking connection, don't wait for success */
   if (node && mongoc_topology_scanner_node_setup (node, &node->last_error)) {
      node->cmd = mongoc_async_cmd (
         ts->async, node->stream, ts->setup,
         node->host.host, "admin",
         &ts->ismaster_cmd,
         &mongoc_topology_scanner_ismaster_handler,
         node, (int32_t) timeout_msec);
   }

   /* if setup fails the node stays in the scanner. destroyed after the scan. */
   return;
}

void
mongoc_topology_scanner_node_retire (mongoc_topology_scanner_node_t *node)
{
   if (node->cmd) {
      node->cmd->state = MONGOC_ASYNC_CMD_CANCELED_STATE;
   }

   node->retired = true;
}

void
mongoc_topology_scanner_node_disconnect (mongoc_topology_scanner_node_t *node,
                                         bool failed)
{
   if (node->dns_results) {
      freeaddrinfo (node->dns_results);
      node->dns_results = NULL;
      node->current_dns_result = NULL;
   }

   if (node->cmd) {
      mongoc_async_cmd_destroy (node->cmd);
      node->cmd = NULL;
   }

   if (node->stream) {
      if (failed) {
         mongoc_stream_failed (node->stream);
      } else {
         mongoc_stream_destroy (node->stream);
      }

      node->stream = NULL;
   }
}

void
mongoc_topology_scanner_node_destroy (mongoc_topology_scanner_node_t *node, bool failed)
{
   DL_DELETE (node->ts->nodes, node);
   mongoc_topology_scanner_node_disconnect (node, failed);
   bson_free (node);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_get_node --
 *
 *      Return the scanner node with the given id.
 *
 *      NOTE: only use this method when single-threaded!
 *
 *--------------------------------------------------------------------------
 */
mongoc_topology_scanner_node_t *
mongoc_topology_scanner_get_node (mongoc_topology_scanner_t *ts,
                                  uint32_t                   id)
{
   mongoc_topology_scanner_node_t *ele, *tmp;

   DL_FOREACH_SAFE (ts->nodes, ele, tmp)
   {
      if (ele->id == id) {
         return ele;
      }

      if (ele->id > id) {
         break;
      }
   }

   return NULL;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_has_node_for_host --
 *
 *      Whether the scanner has a node for the given host and port.
 *
 *--------------------------------------------------------------------------
 */
bool
mongoc_topology_scanner_has_node_for_host (mongoc_topology_scanner_t *ts,
                                           mongoc_host_list_t        *host)
{
   mongoc_topology_scanner_node_t *ele, *tmp;

   DL_FOREACH_SAFE (ts->nodes, ele, tmp)
   {
      if (_mongoc_host_list_equal (&ele->host, host)) {
         return true;
      }
   }

   return false;
}

/*
 *-----------------------------------------------------------------------
 *
 * This is the callback passed to async_cmd when we're running
 * ismasters from within the topology monitor.
 *
 *-----------------------------------------------------------------------
 */

static void
mongoc_topology_scanner_ismaster_handler (mongoc_async_cmd_result_t async_status,
                                          const bson_t             *ismaster_response,
                                          int64_t                   rtt_msec,
                                          void                     *data,
                                          bson_error_t             *error)
{
   mongoc_topology_scanner_node_t *node;
   int64_t now;
   const char *message;

   BSON_ASSERT (data);

   node = (mongoc_topology_scanner_node_t *)data;
   node->cmd = NULL;

   if (node->retired) {
      return;
   }

   now = bson_get_monotonic_time ();

   /* if no ismaster response, async cmd had an error or timed out */
   if (!ismaster_response ||
       async_status == MONGOC_ASYNC_CMD_ERROR ||
       async_status == MONGOC_ASYNC_CMD_TIMEOUT) {
      mongoc_stream_failed (node->stream);
      node->stream = NULL;
      node->last_failed = now;
      message = async_status == MONGOC_ASYNC_CMD_TIMEOUT ?
                "connection error" :
                "connection timeout";
      bson_set_error (&node->last_error,
                      MONGOC_ERROR_CLIENT,
                      MONGOC_ERROR_STREAM_CONNECT,
                      "%s calling ismaster on \'%s\'",
                      message,
                      node->host.host_and_port);
   } else {
      node->last_failed = -1;
   }

   node->last_used = now;

   node->ts->cb (node->id, ismaster_response, rtt_msec,
                 node->ts->cb_data, error);
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_node_connect_tcp --
 *
 *      Create a socket stream for this node, begin a non-blocking
 *      connect and return.
 *
 * Returns:
 *      A stream. On failure, return NULL and fill out the error.
 *
 *--------------------------------------------------------------------------
 */

static mongoc_stream_t *
mongoc_topology_scanner_node_connect_tcp (mongoc_topology_scanner_node_t *node,
                                          bson_error_t                   *error)
{
   mongoc_socket_t *sock = NULL;
   struct addrinfo hints;
   struct addrinfo *rp;
   char portstr [8];
   mongoc_host_list_t *host;
   int s;

   ENTRY;

   host = &node->host;

   if (!node->dns_results) {
      bson_snprintf (portstr, sizeof portstr, "%hu", host->port);

      memset (&hints, 0, sizeof hints);
      hints.ai_family = host->family;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = 0;
      hints.ai_protocol = 0;

      s = getaddrinfo (host->host, portstr, &hints, &node->dns_results);

      if (s != 0) {
         mongoc_counter_dns_failure_inc ();
         bson_set_error (error,
                         MONGOC_ERROR_STREAM,
                         MONGOC_ERROR_STREAM_NAME_RESOLUTION,
                         "Failed to resolve '%s'",
                         host->host);
         RETURN (NULL);
      }

      node->current_dns_result = node->dns_results;

      mongoc_counter_dns_success_inc ();
   }

   for (; node->current_dns_result;
        node->current_dns_result = node->current_dns_result->ai_next) {
      rp = node->current_dns_result;

      /*
       * Create a new non-blocking socket.
       */
      if (!(sock = mongoc_socket_new (rp->ai_family,
                                      rp->ai_socktype,
                                      rp->ai_protocol))) {
         continue;
      }

      mongoc_socket_connect (sock, rp->ai_addr, (socklen_t)rp->ai_addrlen, 0);

      break;
   }

   if (!sock) {
      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_CONNECT,
                      "Failed to connect to target host: '%s'",
                      host->host_and_port);
      freeaddrinfo (node->dns_results);
      node->dns_results = NULL;
      node->current_dns_result = NULL;
      RETURN (NULL);
   }

   return mongoc_stream_socket_new (sock);
}

static mongoc_stream_t *
mongoc_topology_scanner_node_connect_unix (mongoc_topology_scanner_node_t *node,
                                           bson_error_t                   *error)
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
   mongoc_host_list_t *host;

   ENTRY;

   host = &node->host;

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
      char buf[128];
      char *errstr;

      errstr = bson_strerror_r(mongoc_socket_errno(sock), buf, sizeof(buf));

      bson_set_error (error,
                      MONGOC_ERROR_STREAM,
                      MONGOC_ERROR_STREAM_CONNECT,
                      "Failed to connect to UNIX domain socket: %s",
                      errstr);
      mongoc_socket_destroy (sock);
      RETURN (NULL);
   }

   ret = mongoc_stream_socket_new (sock);

   RETURN (ret);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_node_setup --
 *
 *      Create a stream and begin a non-blocking connect.
 *
 * Returns:
 *      true on success, or false and error is set.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_topology_scanner_node_setup (mongoc_topology_scanner_node_t *node,
                                    bson_error_t                   *error)
{
   mongoc_stream_t *sock_stream;

   if (node->stream) { return true; }

   BSON_ASSERT (!node->retired);

   if (node->ts->initiator) {
      sock_stream = node->ts->initiator (node->ts->uri, &node->host,
                                         node->ts->initiator_context, error);
   } else {
      if (node->host.family == AF_UNIX) {
         sock_stream = mongoc_topology_scanner_node_connect_unix (node, error);
      } else {
         sock_stream = mongoc_topology_scanner_node_connect_tcp (node, error);
      }

#ifdef MONGOC_ENABLE_SSL
      if (sock_stream && node->ts->ssl_opts) {
         sock_stream = mongoc_stream_tls_new (sock_stream, node->ts->ssl_opts, 1);
      }
#endif
   }

   if (!sock_stream) {
      /* Pass a rtt of -1 if we couldn't initialize a stream in node_setup */
      node->ts->cb (node->id, NULL, -1, node->ts->cb_data, error);
      return false;
   }

   node->stream = sock_stream;
   node->has_auth = false;
   node->timestamp = bson_get_monotonic_time ();

   return true;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_start --
 *
 *      Initializes the scanner and begins a full topology check. This
 *      should be called once before calling mongoc_topology_scanner_work()
 *      repeatedly to complete the scan.
 *
 *      If "obey_cooldown" is true, this is a single-threaded blocking scan
 *      that must obey the Server Discovery And Monitoring Spec's cooldownMS:
 *
 *      "After a single-threaded client gets a network error trying to check
 *      a server, the client skips re-checking the server until cooldownMS has
 *      passed.
 *
 *      "This avoids spending connectTimeoutMS on each unavailable server
 *      during each scan.
 *
 *      "This value MUST be 5000 ms, and it MUST NOT be configurable."
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_topology_scanner_start (mongoc_topology_scanner_t *ts,
                               int32_t timeout_msec,
                               bool obey_cooldown)
{
   mongoc_topology_scanner_node_t *node, *tmp;
   int64_t cooldown = INT64_MAX;
   BSON_ASSERT (ts);

   if (ts->in_progress) {
      return;
   }

   if (obey_cooldown) {
      /* when current cooldown period began */
      cooldown = bson_get_monotonic_time ()
                 - 1000 * MONGOC_TOPOLOGY_COOLDOWN_MS;
   }

   DL_FOREACH_SAFE (ts->nodes, node, tmp)
   {
      /* check node if it last failed before current cooldown period began */
      if (node->last_failed < cooldown) {
         if (mongoc_topology_scanner_node_setup (node, &node->last_error)) {

            BSON_ASSERT (!node->cmd);

            node->cmd = mongoc_async_cmd (
               ts->async, node->stream, ts->setup,
               node->host.host, "admin",
               &ts->ismaster_cmd,
               &mongoc_topology_scanner_ismaster_handler,
               node, timeout_msec);
         }
      }
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_work --
 *
 *      Crank the knob on the topology scanner state machine. This should
 *      be called only after mongoc_topology_scanner_start() has been used
 *      to begin the scan.
 *
 * Returns:
 *      true if there is more work to do, false if scan is done.
 *
 *--------------------------------------------------------------------------
 */

bool
mongoc_topology_scanner_work (mongoc_topology_scanner_t *ts,
                              int32_t                    timeout_msec)
{
   bool r;

   r = mongoc_async_run (ts->async, timeout_msec);

   if (! r) {
      ts->in_progress = false;
   }

   return r;
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_sum_errors --
 *
 *      Summarizes all scanner node errors into one error message
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_topology_scanner_sum_errors (mongoc_topology_scanner_t *ts,
                                    bson_error_t              *error)
{
   mongoc_topology_scanner_node_t *node, *tmp;

   DL_FOREACH_SAFE (ts->nodes, node, tmp) {
      if (node->last_error.code) {
         char *msg = NULL;

         if (error->code) {
            msg = bson_strdup(error->message);
         }

         bson_set_error(error,
                        MONGOC_ERROR_SERVER_SELECTION,
                        MONGOC_ERROR_SERVER_SELECTION_FAILURE,
                        "%s[%s] ",
                        msg ? msg : "", node->last_error.message);
         if (msg) {
            bson_free (msg);
         }
      }
   }
   if (error->code) {
      error->message[strlen(error->message)-1] = '\0';
   }
}

/*
 *--------------------------------------------------------------------------
 *
 * mongoc_topology_scanner_reset --
 *
 *      Reset "retired" nodes that failed or were removed in the previous
 *      scan.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_topology_scanner_reset (mongoc_topology_scanner_t *ts)
{
   mongoc_topology_scanner_node_t *node, *tmp;

   DL_FOREACH_SAFE (ts->nodes, node, tmp) {
      if (node->retired) {
         mongoc_topology_scanner_node_destroy (node, true);
      }
   }
}

