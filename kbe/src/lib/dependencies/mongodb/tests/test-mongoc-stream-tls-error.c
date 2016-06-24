#include <openssl/err.h>
#include <mongoc.h>
#include <mongoc-thread-private.h>
#include <mongoc-util-private.h>

#include "ssl-test.h"
#include "TestSuite.h"

#define TIMEOUT 10000 /* milliseconds */

/** run as a child thread by test_mongoc_tls_hangup
 *
 * It:
 *    1. spins up
 *    2. binds and listens to a random port
 *    3. notifies the client of its port through a condvar
 *    4. accepts a request
 *    5. reads a byte
 *    7. hangs up
 */
static void *
ssl_error_server (void *ptr)
{
   ssl_test_data_t *data = (ssl_test_data_t *)ptr;

   mongoc_stream_t *sock_stream;
   mongoc_stream_t *ssl_stream;
   mongoc_socket_t *listen_sock;
   mongoc_socket_t *conn_sock;
   socklen_t sock_len;
   char buf;
   ssize_t r;
   mongoc_iovec_t iov;
   struct sockaddr_in server_addr = { 0 };

   iov.iov_base = &buf;
   iov.iov_len = 1;

   listen_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (listen_sock);

   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
   server_addr.sin_port = htons (0);

   r = mongoc_socket_bind (listen_sock,
                           (struct sockaddr *)&server_addr,
                           sizeof server_addr);
   assert (r == 0);

   sock_len = sizeof (server_addr);
   r = mongoc_socket_getsockname (listen_sock, (struct sockaddr *)&server_addr,
                                  &sock_len);
   assert (r == 0);

   r = mongoc_socket_listen (listen_sock, 10);
   assert (r == 0);

   mongoc_mutex_lock (&data->cond_mutex);
   data->server_port = ntohs (server_addr.sin_port);
   mongoc_cond_signal (&data->cond);
   mongoc_mutex_unlock (&data->cond_mutex);

   conn_sock = mongoc_socket_accept (listen_sock, -1);
   assert (conn_sock);

   sock_stream = mongoc_stream_socket_new (conn_sock);
   assert (sock_stream);

   ssl_stream = mongoc_stream_tls_new (sock_stream, data->server, 0);
   assert (ssl_stream);

   switch (data->behavior) {
   case SSL_TEST_BEHAVIOR_STALL_BEFORE_HANDSHAKE:
      _mongoc_usleep (data->handshake_stall_ms * 1000);
      break;
   case SSL_TEST_BEHAVIOR_HANGUP_AFTER_HANDSHAKE:
      r = mongoc_stream_tls_do_handshake (ssl_stream, TIMEOUT);
      assert (r);

      r = mongoc_stream_readv (ssl_stream, &iov, 1, 1, TIMEOUT);
      assert (r == 1);
      break;
   case SSL_TEST_BEHAVIOR_NORMAL:
   default:
      fprintf (stderr, "unimplemented ssl_test_behavior_t\n");
      abort ();
   }

   data->server_result->result = SSL_TEST_SUCCESS;

   mongoc_stream_close (ssl_stream);
   mongoc_stream_destroy (ssl_stream);
   mongoc_socket_destroy (listen_sock);

   return NULL;
}


#define TRUST_DIR "tests/trust_dir"
#define PEMFILE_NOPASS TRUST_DIR "/keys/mongodb.com.pem"

#if !defined(__sun)
/** run as a child thread by test_mongoc_tls_hangup
 *
 * It:
 *    1. spins up
 *    2. waits on a condvar until the server is up
 *    3. connects to the server's port
 *    4. writes a byte
 *    5. confirms that the server hangs up promptly
 *    6. shuts down
 */
static void *
ssl_hangup_client (void *ptr)
{
   ssl_test_data_t *data = (ssl_test_data_t *)ptr;
   mongoc_stream_t *sock_stream;
   mongoc_stream_t *ssl_stream;
   mongoc_socket_t *conn_sock;
   char buf = 'b';
   ssize_t r;
   mongoc_iovec_t riov;
   mongoc_iovec_t wiov;
   struct sockaddr_in server_addr = { 0 };
   int64_t start_time;

   conn_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (conn_sock);

   mongoc_mutex_lock (&data->cond_mutex);
   while (!data->server_port) {
      mongoc_cond_wait (&data->cond, &data->cond_mutex);
   }
   mongoc_mutex_unlock (&data->cond_mutex);

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons (data->server_port);
   server_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

   r = mongoc_socket_connect (conn_sock, (struct sockaddr *)&server_addr,
                              sizeof (server_addr), -1);
   assert (r == 0);

   sock_stream = mongoc_stream_socket_new (conn_sock);
   assert (sock_stream);

   ssl_stream = mongoc_stream_tls_new (sock_stream, data->client, 1);
   assert (ssl_stream);

   r = mongoc_stream_tls_do_handshake (ssl_stream, TIMEOUT);
   assert (r);

   wiov.iov_base = (void *)&buf;
   wiov.iov_len = 1;
   r = mongoc_stream_writev (ssl_stream, &wiov, 1, TIMEOUT);
   assert (r == 1);

   riov.iov_base = (void *)&buf;
   riov.iov_len = 1;

   /* we should notice promptly that the server hangs up */
   start_time = bson_get_monotonic_time ();
   r = mongoc_stream_readv (ssl_stream, &riov, 1, 1, TIMEOUT);
   /* time is in microseconds */
   assert (bson_get_monotonic_time () - start_time < 1000 * 1000);
   assert (r == -1);
   mongoc_stream_destroy (ssl_stream);
   data->client_result->result = SSL_TEST_SUCCESS;
   return NULL;
}

static void
test_mongoc_tls_hangup (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;
   ssl_test_data_t data = { 0 };
   mongoc_thread_t threads[2];
   int i, r;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.weak_cert_validation = 1;
   copt.weak_cert_validation = 1;

   data.server = &sopt;
   data.client = &copt;
   data.behavior = SSL_TEST_BEHAVIOR_HANGUP_AFTER_HANDSHAKE;
   data.server_result = &sr;
   data.client_result = &cr;
   data.host = "localhost";

   mongoc_mutex_init (&data.cond_mutex);
   mongoc_cond_init (&data.cond);

   r = mongoc_thread_create (threads, &ssl_error_server, &data);
   assert (r == 0);

   r = mongoc_thread_create (threads + 1, &ssl_hangup_client, &data);
   assert (r == 0);

   for (i = 0; i < 2; i++) {
      r = mongoc_thread_join (threads[i]);
      assert (r == 0);
   }

   mongoc_mutex_destroy (&data.cond_mutex);
   mongoc_cond_destroy (&data.cond);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}
#endif


/** run as a child thread by test_mongoc_tls_handshake_stall
 *
 * It:
 *    1. spins up
 *    2. waits on a condvar until the server is up
 *    3. connects to the server's port
 *    4. attempts handshake
 *    5. confirms that it times out
 *    6. shuts down
 */
static void *
handshake_stall_client (void *ptr)
{
   ssl_test_data_t *data = (ssl_test_data_t *)ptr;
   char *uri_str;
   mongoc_client_t *client;
   bson_t reply;
   bson_error_t error;
   int64_t connect_timeout_ms = data->handshake_stall_ms - 100;
   int64_t duration_ms;

   int64_t start_time;

   mongoc_mutex_lock (&data->cond_mutex);
   while (!data->server_port) {
      mongoc_cond_wait (&data->cond, &data->cond_mutex);
   }
   mongoc_mutex_unlock (&data->cond_mutex);

   uri_str = bson_strdup_printf (
      "mongodb://localhost:%u/?ssl=true&serverselectiontimeoutms=200&connecttimeoutms=%" PRId64,
      data->server_port, connect_timeout_ms);

   client = mongoc_client_new (uri_str);

   /* we should time out after about 200ms */
   start_time = bson_get_monotonic_time ();
   mongoc_client_get_server_status (client,
                                    NULL,
                                    &reply,
                                    &error);

   /* time is in microseconds */
   duration_ms = (bson_get_monotonic_time () - start_time) / 1000;

   if (llabs(duration_ms - connect_timeout_ms) > 100) {
      fprintf (stderr,
               "expected timeout after about 200ms, not %" PRId64 "\n",
               duration_ms);
      abort ();
   }

   data->client_result->result = SSL_TEST_SUCCESS;

   bson_destroy (&reply);
   mongoc_client_destroy (client);
   bson_free (uri_str);

   return NULL;
}


static void
test_mongoc_tls_handshake_stall (void)
{
   mongoc_ssl_opt_t sopt = { 0 };
   mongoc_ssl_opt_t copt = { 0 };
   ssl_test_result_t sr;
   ssl_test_result_t cr;
   ssl_test_data_t data = { 0 };
   mongoc_thread_t threads[2];
   int i, r;

   sopt.pem_file = PEMFILE_NOPASS;
   sopt.weak_cert_validation = 1;
   copt.weak_cert_validation = 1;

   data.server = &sopt;
   data.client = &copt;
   data.behavior = SSL_TEST_BEHAVIOR_STALL_BEFORE_HANDSHAKE;
   data.handshake_stall_ms = 300;
   data.server_result = &sr;
   data.client_result = &cr;
   data.host = "localhost";

   mongoc_mutex_init (&data.cond_mutex);
   mongoc_cond_init (&data.cond);

   r = mongoc_thread_create (threads, &ssl_error_server, &data);
   assert (r == 0);

   r = mongoc_thread_create (threads + 1, &handshake_stall_client, &data);
   assert (r == 0);

   for (i = 0; i < 2; i++) {
      r = mongoc_thread_join (threads[i]);
      assert (r == 0);
   }

   mongoc_mutex_destroy (&data.cond_mutex);
   mongoc_cond_destroy (&data.cond);

   ASSERT (cr.result == SSL_TEST_SUCCESS);
   ASSERT (sr.result == SSL_TEST_SUCCESS);
}

void
test_stream_tls_error_install (TestSuite *suite)
{
   /* TLS stream doesn't detect hangup promptly on Solaris for some reason */
#if !defined(__sun)
   TestSuite_Add (suite, "/TLS/hangup", test_mongoc_tls_hangup);
#endif

   TestSuite_Add (suite, "/TLS/handshake_stall",
                  test_mongoc_tls_handshake_stall);
}
