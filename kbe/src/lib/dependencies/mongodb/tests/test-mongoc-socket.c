#include <fcntl.h>
#include <mongoc.h>

#include "mongoc-tests.h"
#include "mongoc-socket-private.h"
#include "mongoc-thread-private.h"
#include "mongoc-errno-private.h"
#include "TestSuite.h"

#include "test-libmongoc.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "socket-test"

#define TIMEOUT 10000
#define WAIT 1000


static size_t gFourMB = 1024 * 1024 * 4;

typedef struct
{
   unsigned short server_port;
   mongoc_cond_t  cond;
   mongoc_mutex_t cond_mutex;
   bool           closed_socket;
   int            amount;
} socket_test_data_t;


static void *
socket_test_server (void *data_)
{
   socket_test_data_t *data = (socket_test_data_t *)data_;
   struct sockaddr_in server_addr = { 0 };
   mongoc_socket_t *listen_sock;
   mongoc_socket_t *conn_sock;
   mongoc_stream_t *stream;
   mongoc_iovec_t iov;
   socklen_t sock_len;
   ssize_t r;
   char buf[5];

   iov.iov_base = buf;
   iov.iov_len = sizeof (buf);

   listen_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (listen_sock);

   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
   server_addr.sin_port = htons (0);

   r = mongoc_socket_bind (listen_sock,
                           (struct sockaddr *)&server_addr,
                           sizeof server_addr);
   assert (r == 0);

   sock_len = sizeof(server_addr);
   r = mongoc_socket_getsockname (listen_sock, (struct sockaddr *)&server_addr, &sock_len);
   assert(r == 0);

   r = mongoc_socket_listen (listen_sock, 10);
   assert(r == 0);

   mongoc_mutex_lock(&data->cond_mutex);
   data->server_port = ntohs(server_addr.sin_port);
   mongoc_cond_signal(&data->cond);
   mongoc_mutex_unlock(&data->cond_mutex);

   conn_sock = mongoc_socket_accept (listen_sock, -1);
   assert (conn_sock);

   stream = mongoc_stream_socket_new (conn_sock);
   assert (stream);

   r = mongoc_stream_readv (stream, &iov, 1, 5, TIMEOUT);
   assert (r == 5);
   assert (strcmp (buf, "ping") == 0);

   strcpy (buf, "pong");

   r = mongoc_stream_writev (stream, &iov, 1, TIMEOUT);
   assert (r == 5);

   mongoc_stream_destroy (stream);

   mongoc_mutex_lock(&data->cond_mutex);
   data->closed_socket = true;
   mongoc_cond_signal(&data->cond);
   mongoc_mutex_unlock(&data->cond_mutex);

   mongoc_socket_destroy (listen_sock);

   return NULL;
}


static void *
socket_test_client (void *data_)
{
   socket_test_data_t *data = (socket_test_data_t *)data_;
   mongoc_socket_t *conn_sock;
   char buf[5];
   ssize_t r;
   bool closed;
   struct sockaddr_in server_addr = { 0 };
   mongoc_stream_t *stream;
   mongoc_iovec_t iov;

   iov.iov_base = buf;
   iov.iov_len = sizeof (buf);

   conn_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (conn_sock);

   mongoc_mutex_lock(&data->cond_mutex);
   while (! data->server_port) {
      mongoc_cond_wait(&data->cond, &data->cond_mutex);
   }
   mongoc_mutex_unlock(&data->cond_mutex);

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(data->server_port);
   server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

   r = mongoc_socket_connect (conn_sock, (struct sockaddr *)&server_addr, sizeof(server_addr), -1);
   assert (r == 0);

   stream = mongoc_stream_socket_new (conn_sock);

   strcpy (buf, "ping");

   closed = mongoc_stream_check_closed (stream);
   assert (closed == false);

   r = mongoc_stream_writev (stream, &iov, 1, TIMEOUT);
   assert (r == 5);

   closed = mongoc_stream_check_closed (stream);
   assert (closed == false);

   r = mongoc_stream_readv (stream, &iov, 1, 5, TIMEOUT);
   assert (r == 5);
   assert (strcmp (buf, "pong") == 0);

   mongoc_mutex_lock(&data->cond_mutex);
   while (! data->closed_socket) {
      mongoc_cond_wait(&data->cond, &data->cond_mutex);
   }
   mongoc_mutex_unlock(&data->cond_mutex);

   closed = mongoc_stream_check_closed (stream);
   assert (closed == true);

   mongoc_stream_destroy (stream);

   return NULL;
}


static void *
sendv_test_server (void *data_)
{
   socket_test_data_t *data = (socket_test_data_t *)data_;
   struct sockaddr_in server_addr = { 0 };
   mongoc_socket_t *listen_sock;
   mongoc_socket_t *conn_sock;
   mongoc_stream_t *stream;
   mongoc_iovec_t iov;
   socklen_t sock_len;
   int amount = 0;
   ssize_t r;
   char *buf = (char *)bson_malloc (gFourMB);

   iov.iov_base = buf;
   iov.iov_len = gFourMB;

   listen_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (listen_sock);

   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
   server_addr.sin_port = htons (0);

   r = mongoc_socket_bind (listen_sock,
                           (struct sockaddr *)&server_addr,
                           sizeof server_addr);
   assert (r == 0);

   sock_len = sizeof(server_addr);
   r = mongoc_socket_getsockname (listen_sock, (struct sockaddr *)&server_addr, &sock_len);
   assert(r == 0);

   r = mongoc_socket_listen (listen_sock, 10);
   assert(r == 0);

   mongoc_mutex_lock(&data->cond_mutex);
   data->server_port = ntohs(server_addr.sin_port);
   mongoc_cond_signal(&data->cond);
   mongoc_mutex_unlock(&data->cond_mutex);

   conn_sock = mongoc_socket_accept (listen_sock, -1);
   assert (conn_sock);

   stream = mongoc_stream_socket_new (conn_sock);
   assert (stream);

   /* Wait until the client has pushed so much data he can't write more */
   mongoc_mutex_lock(&data->cond_mutex);
   while (! data->amount) {
      mongoc_cond_wait(&data->cond, &data->cond_mutex);
   }
   amount = data->amount;
   data->amount = 0;
   mongoc_mutex_unlock(&data->cond_mutex);

   /* Start reading everything off the socket to unblock the client */
   do {
      r = mongoc_stream_readv (stream, &iov, 1, amount, WAIT);
      if (r > 0) {
         amount -= r;
      } else if (MONGOC_ERRNO_IS_AGAIN(errno)) {
         continue;
      } else {
         fprintf(stderr, "r: %"PRIu64" , errno: %d", (uint64_t)r, conn_sock->errno_);
         assert (r == amount);
      }
   } while (amount > 0);


   /* Allow the client to finish all its writes */
   mongoc_mutex_lock(&data->cond_mutex);
   while (! data->amount) {
      mongoc_cond_wait(&data->cond, &data->cond_mutex);
   }
   /* amount is likely negative value now, we've read more then caused the original blocker */
   amount += data->amount;
   data->amount = 0;
   mongoc_mutex_unlock(&data->cond_mutex);

   do {
      r = mongoc_stream_readv (stream, &iov, 1, amount, WAIT);
      if (r > 0) {
         amount -= r;
      } else if (MONGOC_ERRNO_IS_AGAIN(errno)) {
         continue;
      } else {
         fprintf(stderr, "r: %"PRIu64" , errno: %d", (uint64_t)r, errno);
         assert (r == amount);
      }
   } while (amount > 0);
   ASSERT_CMPINT(0, ==, amount);


   mongoc_stream_destroy (stream);

   mongoc_socket_destroy (listen_sock);

   return NULL;
}


static void *
sendv_test_client (void *data_)
{
   socket_test_data_t *data = (socket_test_data_t *)data_;
   mongoc_socket_t *conn_sock;
   ssize_t r;
   int i;
   int amount = 0;
   struct sockaddr_in server_addr = { 0 };
   mongoc_stream_t *stream;
   mongoc_iovec_t iov;
   bool done = false;
   char *buf = (char *)bson_malloc (gFourMB);

   memset (buf, 'a', (gFourMB)- 1);
   buf[gFourMB - 1] = '\0';

   iov.iov_base = buf;
   iov.iov_len = gFourMB;

   conn_sock = mongoc_socket_new (AF_INET, SOCK_STREAM, 0);
   assert (conn_sock);

   mongoc_mutex_lock(&data->cond_mutex);
   while (! data->server_port) {
      mongoc_cond_wait(&data->cond, &data->cond_mutex);
   }
   mongoc_mutex_unlock(&data->cond_mutex);

   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(data->server_port);
   server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

   r = mongoc_socket_connect (conn_sock, (struct sockaddr *)&server_addr, sizeof(server_addr), -1);
   assert (r == 0);

   stream = mongoc_stream_socket_new (conn_sock);

   for (i = 0; i < 5; i++)
   {
      r = mongoc_stream_writev (stream, &iov, 1, WAIT);
      if (r > 0) {
         amount += r;
      }
      if (r != gFourMB) {
         if (MONGOC_ERRNO_IS_AGAIN(conn_sock->errno_)) {
            if (!done) {
               mongoc_mutex_lock(&data->cond_mutex);
               data->amount = amount;
               amount = 0;
               mongoc_cond_signal(&data->cond);
               mongoc_mutex_unlock(&data->cond_mutex);
               done = true;
            }
         } else {
            assert (r == gFourMB);
         }
      }
   }
   assert(true == done);
   mongoc_mutex_lock(&data->cond_mutex);
   data->amount = amount;
   mongoc_cond_signal(&data->cond);
   mongoc_mutex_unlock(&data->cond_mutex);

   mongoc_stream_destroy (stream);

   return NULL;
}


static void
test_mongoc_socket_check_closed (void)
{
   socket_test_data_t data = { 0 };
   mongoc_thread_t threads[2];
   int i, r;

   mongoc_mutex_init (&data.cond_mutex);
   mongoc_cond_init (&data.cond);

   r = mongoc_thread_create (threads, &socket_test_server, &data);
   assert (r == 0);

   r = mongoc_thread_create (threads + 1, &socket_test_client, &data);
   assert (r == 0);

   for (i = 0; i < 2; i++) {
      r = mongoc_thread_join (threads[i]);
      assert (r == 0);
   }

   mongoc_mutex_destroy (&data.cond_mutex);
   mongoc_cond_destroy (&data.cond);
}

static void
test_mongoc_socket_sendv (void)
{
   socket_test_data_t data = { 0 };
   mongoc_thread_t threads[2];
   int i, r;

   mongoc_mutex_init (&data.cond_mutex);
   mongoc_cond_init (&data.cond);

   r = mongoc_thread_create (threads, &sendv_test_server, &data);
   assert (r == 0);

   r = mongoc_thread_create (threads + 1, &sendv_test_client, &data);
   assert (r == 0);

   for (i = 0; i < 2; i++) {
      r = mongoc_thread_join (threads[i]);
      assert (r == 0);
   }

   mongoc_mutex_destroy (&data.cond_mutex);
   mongoc_cond_destroy (&data.cond);
}

void
test_socket_install (TestSuite *suite)
{
   TestSuite_Add (suite, "/Socket/check_closed", test_mongoc_socket_check_closed);
   TestSuite_Add (suite, "/Socket/sendv", test_mongoc_socket_sendv);
}
