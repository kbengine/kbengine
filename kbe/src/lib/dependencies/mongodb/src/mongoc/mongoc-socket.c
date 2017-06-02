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


#include <errno.h>
#include <string.h>

#include "mongoc-counters-private.h"
#include "mongoc-errno-private.h"
#include "mongoc-socket-private.h"
#include "mongoc-host-list.h"
#include "mongoc-socket-private.h"
#include "mongoc-trace.h"

#undef MONGOC_LOG_DOMAIN
#define MONGOC_LOG_DOMAIN "socket"


#define OPERATION_EXPIRED(expire_at) \
   ((expire_at >= 0) && (expire_at < (bson_get_monotonic_time())))


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_setnonblock --
 *
 *       A helper to set a socket in nonblocking mode.
 *
 * Returns:
 *       true if successful; otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
#ifdef _WIN32
_mongoc_socket_setnonblock (SOCKET sd)
#else
_mongoc_socket_setnonblock (int sd)
#endif
{
#ifdef _WIN32
   u_long io_mode = 1;
   return (NO_ERROR == ioctlsocket (sd, FIONBIO, &io_mode));
#else
   int flags;

   flags = fcntl (sd, F_GETFL, sd);
   return (-1 != fcntl (sd, F_SETFL, (flags | O_NONBLOCK)));
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_wait --
 *
 *       A single socket poll helper.
 *
 *       @events: in most cases should be POLLIN or POLLOUT.
 *
 *       @expire_at should be an absolute time at which to expire using
 *       the monotonic clock (bson_get_monotonic_time(), which is in
 *       microseconds). Or zero to not block at all. Or -1 to block
 *       forever.
 *
 * Returns:
 *       true if an event matched. otherwise false.
 *       a timeout will return false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
#ifdef _WIN32
_mongoc_socket_wait (SOCKET   sd,           /* IN */
#else
_mongoc_socket_wait (int      sd,           /* IN */
#endif
                     int      events,       /* IN */
                     int64_t  expire_at)    /* IN */
{
#ifdef _WIN32
   WSAPOLLFD pfd;
#else
   struct pollfd pfd;
#endif
   int ret;
   int timeout;
   int64_t now;

   ENTRY;

   BSON_ASSERT (events);

   pfd.fd = sd;
#ifdef _WIN32
   pfd.events = events;
#else
   pfd.events = events | POLLERR | POLLHUP;
#endif
   pfd.revents = 0;

   now = bson_get_monotonic_time();

   for (;;) {
      if (expire_at < 0) {
         timeout = -1;
      } else if (expire_at == 0) {
         timeout = 0;
      } else {
         timeout = (int)((expire_at - now) / 1000L);
         if (timeout < 0) {
            timeout = 0;
         }
      }

#ifdef _WIN32
      ret = WSAPoll (&pfd, 1, timeout);
      if (ret == SOCKET_ERROR) {
         errno = WSAGetLastError();
         ret = -1;
      }
#else
      ret = poll (&pfd, 1, timeout);
#endif

      if (ret > 0) {
         /* Something happened, so return that */
#ifdef _WIN32
         RETURN (0 != (pfd.revents & (events | POLLHUP | POLLERR)));
#else
         RETURN (0 != (pfd.revents & events));
#endif
      } else if (ret < 0) {
         /* poll itself failed */

         TRACE("errno is: %d", errno);
         if (MONGOC_ERRNO_IS_AGAIN(errno)) {
            now = bson_get_monotonic_time();

            if (expire_at < now) {
               RETURN (false);
            } else {
               continue;
            }
         } else {
            /* poll failed for some non-transient reason */
            RETURN (false);
         }
      } else {
         /* poll timed out */
         RETURN (false);
      }
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_poll --
 *
 *       A multi-socket poll helper.
 *
 *       @events: in most cases should be POLLIN or POLLOUT.
 *
 *       @expire_at should be an absolute time at which to expire using
 *       the monotonic clock (bson_get_monotonic_time(), which is in
 *       microseconds). Or zero to not block at all. Or -1 to block
 *       forever.
 *
 * Returns:
 *       true if an event matched. otherwise false.
 *       a timeout will return false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
mongoc_socket_poll (mongoc_socket_poll_t *sds,          /* IN */
                    size_t                nsds,         /* IN */
                    int32_t               timeout)      /* IN */
{
#ifdef _WIN32
   WSAPOLLFD *pfds;
#else
   struct pollfd *pfds;
#endif
   int ret;
   int i;

   ENTRY;

   BSON_ASSERT (sds);

#ifdef _WIN32
   pfds = (WSAPOLLFD *)bson_malloc(sizeof(*pfds) * nsds);
#else
   pfds = (struct pollfd *)bson_malloc(sizeof(*pfds) * nsds);
#endif

   for (i = 0; i < nsds; i++) {
      pfds[i].fd = sds[i].socket->sd;
#ifdef _WIN32
      pfds[i].events = sds[i].events;
#else
      pfds[i].events = sds[i].events | POLLERR | POLLHUP;
#endif
      pfds[i].revents = 0;
   }

#ifdef _WIN32
   ret = WSAPoll (pfds, nsds, timeout);
   if (ret == SOCKET_ERROR) {
      MONGOC_WARNING ("WSAGetLastError(): %d", WSAGetLastError ());
      ret = -1;
   }
#else
   ret = poll (pfds, nsds, timeout);
#endif

   for (i = 0; i < nsds; i++) {
      sds[i].revents = pfds[i].revents;
   }

   bson_free(pfds);

   return ret;
}


static bool
#ifdef _WIN32
_mongoc_socket_setnodelay (SOCKET sd) /* IN */
#else
_mongoc_socket_setnodelay (int sd)    /* IN */
#endif
{
#ifdef _WIN32
   BOOL optval = 1;
#else
   int optval = 1;
#endif
   int ret;

   ENTRY;

   errno = 0;
   ret = setsockopt (sd, IPPROTO_TCP, TCP_NODELAY,
                     (char *)&optval, sizeof optval);

#ifdef _WIN32
   if (ret == SOCKET_ERROR) {
      MONGOC_WARNING ("WSAGetLastError(): %d", (int)WSAGetLastError ());
   }
#endif

   RETURN (ret == 0);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_errno --
 *
 *       Returns the last error on the socket.
 *
 * Returns:
 *       An integer errno, or 0 on no error.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_errno (mongoc_socket_t *sock) /* IN */
{
   BSON_ASSERT (sock);
   TRACE("Current errno: %d", sock->errno_);
   return sock->errno_;
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_capture_errno --
 *
 *       Save the errno state for contextual use.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static void
_mongoc_socket_capture_errno (mongoc_socket_t *sock) /* IN */
{
#ifdef _WIN32
   errno = sock->errno_ = WSAGetLastError ();
#else
   sock->errno_ = errno;
#endif
   TRACE("setting errno: %d", sock->errno_);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_errno_is_again --
 *
 *       Check to see if we should attempt to make further progress
 *       based on the error of the last operation.
 *
 * Returns:
 *       true if we should try again. otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static bool
_mongoc_socket_errno_is_again (mongoc_socket_t *sock) /* IN */
{
   TRACE("errno is: %d", sock->errno_);
   return MONGOC_ERRNO_IS_AGAIN (sock->errno_);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_accept --
 *
 *       Wrapper for BSD socket accept(). Handles portability between
 *       BSD sockets and WinSock2 on Windows Vista and newer.
 *
 * Returns:
 *       NULL upon failure to accept or timeout.
 *       A newly allocated mongoc_socket_t on success.
 *
 * Side effects:
 *       *port contains the client port number.
 *
 *--------------------------------------------------------------------------
 */

mongoc_socket_t *
mongoc_socket_accept (mongoc_socket_t *sock,      /* IN */
                      int64_t          expire_at) /* IN */
{
   return mongoc_socket_accept_ex (sock, expire_at, NULL);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_accept_ex --
 *
 *       Private synonym for mongoc_socket_accept, returning client port.
 *
 * Returns:
 *       NULL upon failure to accept or timeout.
 *       A newly allocated mongoc_socket_t on success.
 *
 * Side effects:
 *       *port contains the client port number.
 *
 *--------------------------------------------------------------------------
 */

mongoc_socket_t *
mongoc_socket_accept_ex (mongoc_socket_t *sock,      /* IN */
                         int64_t          expire_at, /* IN */
                         uint16_t *port)             /* OUT */
{
   mongoc_socket_t *client;
   struct sockaddr_in addr;
   socklen_t addrlen = sizeof addr;
   bool try_again = false;
   bool failed = false;
#ifdef _WIN32
   SOCKET sd;
#else
   int sd;
#endif

   ENTRY;

   BSON_ASSERT (sock);

again:
   errno = 0;
   sd = accept (sock->sd, (struct sockaddr *) &addr, &addrlen);

   _mongoc_socket_capture_errno (sock);
#ifdef _WIN32
   failed = (sd == INVALID_SOCKET);
#else
   failed = (sd == -1);
#endif
   try_again = (failed && _mongoc_socket_errno_is_again (sock));

   if (failed && try_again) {
      if (_mongoc_socket_wait (sock->sd, POLLIN, expire_at)) {
         GOTO (again);
      }
      RETURN (NULL);
   } else if (failed) {
      RETURN (NULL);
   } else if (!_mongoc_socket_setnonblock (sd)) {
#ifdef _WIN32
      closesocket (sd);
#else
      close (sd);
#endif
      RETURN (NULL);
   }

   client = (mongoc_socket_t *)bson_malloc0 (sizeof *client);
   client->sd = sd;

   if (port) {
      *port = ntohs (addr.sin_port);
   }

   if (!_mongoc_socket_setnodelay (client->sd)) {
      MONGOC_WARNING ("Failed to enable TCP_NODELAY.");
   }

   RETURN (client);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongo_socket_bind --
 *
 *       A wrapper around bind().
 *
 * Returns:
 *       0 on success, -1 on failure and errno is set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_bind (mongoc_socket_t       *sock,    /* IN */
                    const struct sockaddr *addr,    /* IN */
                    socklen_t              addrlen) /* IN */
{
   int ret;

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (addr);
   BSON_ASSERT (addrlen);

   ret = bind (sock->sd, addr, addrlen);

   _mongoc_socket_capture_errno (sock);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_close --
 *
 *       Closes the underlying socket.
 *
 *       In general, you probably don't want to handle the result from
 *       this. That could cause race conditions in the case of preemtion
 *       during system call (EINTR).
 *
 * Returns:
 *       0 on success, -1 on failure.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_close (mongoc_socket_t *sock) /* IN */
{
   ENTRY;

   BSON_ASSERT (sock);

#ifdef _WIN32
   if (sock->sd != INVALID_SOCKET) {
      shutdown (sock->sd, SD_BOTH);
      if (0 == closesocket (sock->sd)) {
         sock->sd = INVALID_SOCKET;
      } else {
         _mongoc_socket_capture_errno (sock);
         RETURN(-1);
      }
   }
   RETURN(0);
#else
   if (sock->sd != -1) {
      shutdown (sock->sd, SHUT_RDWR);
      if (0 == close (sock->sd)) {
         sock->sd = -1;
      } else {
         _mongoc_socket_capture_errno (sock);
         RETURN(-1);
      }
   }
   RETURN(0);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_connect --
 *
 *       Performs a socket connection but will fail if @expire_at is
 *       reached by the monotonic clock.
 *
 * Returns:
 *       0 if success, otherwise -1 and errno is set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_connect (mongoc_socket_t       *sock,      /* IN */
                       const struct sockaddr *addr,      /* IN */
                       socklen_t              addrlen,   /* IN */
                       int64_t                expire_at) /* IN */
{
   bool try_again = false;
   bool failed = false;
   int ret;
   int optval;
   socklen_t optlen = sizeof optval;

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (addr);
   BSON_ASSERT (addrlen);

   ret = connect (sock->sd, addr, addrlen);

#ifdef _WIN32
   if (ret == SOCKET_ERROR) {
#else
   if (ret == -1) {
#endif
      _mongoc_socket_capture_errno (sock);

      failed = true;
      try_again = _mongoc_socket_errno_is_again (sock);
   }

   if (failed && try_again) {
      if (_mongoc_socket_wait (sock->sd, POLLOUT, expire_at)) {
         optval = -1;
         ret = getsockopt (sock->sd, SOL_SOCKET, SO_ERROR,
                           (char *)&optval, &optlen);
         if ((ret == 0) && (optval == 0)) {
            RETURN (0);
         } else {
            errno = sock->errno_ = optval;
         }
      }
      RETURN (-1);
   } else if (failed) {
      RETURN (-1);
   } else {
      RETURN (0);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_destroy --
 *
 *       Cleanup after a mongoc_socket_t structure, possibly closing
 *       underlying sockets.
 *
 * Returns:
 *       None.
 *
 * Side effects:
 *       @sock is freed and should be considered invalid.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_socket_destroy (mongoc_socket_t *sock) /* IN */
{
   if (sock) {
      mongoc_socket_close (sock);
      bson_free (sock);
   }
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_listen --
 *
 *       Listen for incoming requests with a backlog up to @backlog.
 *
 *       If @backlog is zero, a sensible default will be chosen.
 *
 * Returns:
 *       true if successful; otherwise false.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_listen (mongoc_socket_t *sock,    /* IN */
                      unsigned int     backlog) /* IN */
{
   int ret;

   ENTRY;

   BSON_ASSERT (sock);

   if (backlog == 0) {
      backlog = 10;
   }

   ret = listen (sock->sd, backlog);

   _mongoc_socket_capture_errno (sock);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_new --
 *
 *       Create a new socket.
 *
 *       Free the result mongoc_socket_destroy().
 *
 * Returns:
 *       A newly allocated socket.
 *       NULL on failure.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

mongoc_socket_t *
mongoc_socket_new (int domain,   /* IN */
                   int type,     /* IN */
                   int protocol) /* IN */
{
   mongoc_socket_t *sock;
#ifdef _WIN32
   SOCKET sd;
#else
   int sd;
#endif

   ENTRY;

   sd = socket (domain, type, protocol);

#ifdef _WIN32
   if (sd == INVALID_SOCKET) {
#else
   if (sd == -1) {
#endif
      RETURN (NULL);
   }

   if (!_mongoc_socket_setnonblock (sd)) {
      GOTO (fail);
   }

   if (domain != AF_UNIX && !_mongoc_socket_setnodelay (sd)) {
      MONGOC_WARNING ("Failed to enable TCP_NODELAY.");
   }

   sock = (mongoc_socket_t *)bson_malloc0 (sizeof *sock);
   sock->sd = sd;
   sock->domain = domain;

   RETURN (sock);

fail:
#ifdef _WIN32
   closesocket (sd);
#else
   close (sd);
#endif

   RETURN (NULL);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_recv --
 *
 *       A portable wrapper around recv() that also respects an absolute
 *       timeout.
 *
 *       @expire_at is 0 for no blocking, -1 for infinite blocking,
 *       or a time using the monotonic clock to expire. Calculate this
 *       using bson_get_monotonic_time() + N_MICROSECONDS.
 *
 * Returns:
 *       The number of bytes received on success.
 *       0 on end of stream.
 *       -1 on failure.
 *
 * Side effects:
 *       @buf will be read into.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
mongoc_socket_recv (mongoc_socket_t *sock,      /* IN */
                    void            *buf,       /* OUT */
                    size_t           buflen,    /* IN */
                    int              flags,     /* IN */
                    int64_t          expire_at) /* IN */
{
   ssize_t ret = 0;
   bool failed = false;

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (buf);
   BSON_ASSERT (buflen);

again:
   sock->errno_ = 0;
#ifdef _WIN32
   ret = recv (sock->sd, (char *)buf, (int)buflen, flags);
   failed = (ret == SOCKET_ERROR);
#else
   ret = recv (sock->sd, buf, buflen, flags);
   failed = (ret == -1);
#endif
   if (failed) {
      _mongoc_socket_capture_errno (sock);
      if (_mongoc_socket_errno_is_again (sock) &&
          _mongoc_socket_wait (sock->sd, POLLIN, expire_at)) {
         GOTO (again);
      }
   }

   if (failed) {
      RETURN (-1);
   }

   mongoc_counter_streams_ingress_add (ret);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_setsockopt --
 *
 *       A wrapper around setsockopt().
 *
 * Returns:
 *       0 on success, -1 on failure.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

int
mongoc_socket_setsockopt (mongoc_socket_t *sock,    /* IN */
                          int              level,   /* IN */
                          int              optname, /* IN */
                          const void      *optval,  /* IN */
                          socklen_t        optlen)  /* IN */
{
   int ret;

   ENTRY;

   BSON_ASSERT (sock);

   ret = setsockopt (sock->sd, level, optname, optval, optlen);

   _mongoc_socket_capture_errno (sock);

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_send --
 *
 *       A simplified wrapper around mongoc_socket_sendv().
 *
 *       @expire_at is 0 for no blocking, -1 for infinite blocking,
 *       or a time using the monotonic clock to expire. Calculate this
 *       using bson_get_monotonic_time() + N_MICROSECONDS.
 *
 * Returns:
 *       -1 on failure. number of bytes written on success.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
mongoc_socket_send (mongoc_socket_t *sock,      /* IN */
                    const void      *buf,       /* IN */
                    size_t           buflen,    /* IN */
                    int64_t          expire_at) /* IN */
{
   mongoc_iovec_t iov;

   BSON_ASSERT (sock);
   BSON_ASSERT (buf);
   BSON_ASSERT (buflen);

   iov.iov_base = (void *)buf;
   iov.iov_len = buflen;

   return mongoc_socket_sendv (sock, &iov, 1, expire_at);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_try_sendv_slow --
 *
 *       A slow variant of _mongoc_socket_try_sendv() that sends each
 *       iovec entry one by one. This can happen if we hit EMSGSIZE
 *       with sendmsg() on various POSIX systems or WSASend()+WSAEMSGSIZE
 *       on Windows.
 *
 * Returns:
 *       the number of bytes sent or -1 and errno is set.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static ssize_t
_mongoc_socket_try_sendv_slow (mongoc_socket_t *sock,   /* IN */
                               mongoc_iovec_t  *iov,    /* IN */
                               size_t           iovcnt) /* IN */
{
   ssize_t ret = 0;
   size_t i;
   ssize_t wrote;

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (iov);
   BSON_ASSERT (iovcnt);

   for (i = 0; i < iovcnt; i++) {
      wrote = send (sock->sd, iov [i].iov_base, iov [i].iov_len, 0);
#ifdef _WIN32
      if (wrote == SOCKET_ERROR) {
#else
      if (wrote == -1) {
#endif
         _mongoc_socket_capture_errno (sock);

         if (!_mongoc_socket_errno_is_again (sock)) {
            RETURN (-1);
         }
         RETURN (ret ? ret : -1);
      }

      ret += wrote;

      if (wrote != iov [i].iov_len) {
         RETURN (ret);
      }
   }

   RETURN (ret);
}


/*
 *--------------------------------------------------------------------------
 *
 * _mongoc_socket_try_sendv --
 *
 *       Helper used by mongoc_socket_sendv() to try to write as many
 *       bytes to the underlying socket until the socket buffer is full.
 *
 *       This is performed in a non-blocking fashion.
 *
 * Returns:
 *       -1 on failure. the number of bytes written on success.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

static ssize_t
_mongoc_socket_try_sendv (mongoc_socket_t *sock,   /* IN */
                          mongoc_iovec_t  *iov,    /* IN */
                          size_t           iovcnt) /* IN */
{
#ifdef _WIN32
   DWORD dwNumberofBytesSent = 0;
   int ret;
#else
   struct msghdr msg;
   ssize_t ret;
#endif

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (iov);
   BSON_ASSERT (iovcnt);

   DUMP_IOVEC (sendbuf, iov, iovcnt);

#ifdef _WIN32
   ret = WSASend (sock->sd, (LPWSABUF)iov, iovcnt, &dwNumberofBytesSent,
                  0, NULL, NULL);
   TRACE("WSASend sent: %ld (out of: %ld), ret: %d", dwNumberofBytesSent, iov->iov_len, ret);
#else
   memset (&msg, 0, sizeof msg);
   msg.msg_iov = iov;
   msg.msg_iovlen = (int) iovcnt;
   ret = sendmsg (sock->sd, &msg,
# ifdef MSG_NOSIGNAL
                  MSG_NOSIGNAL);
# else
                  0);
# endif
   TRACE("Send %ld out of %ld bytes", ret, iov->iov_len);
#endif


#ifdef _WIN32
   if (ret == SOCKET_ERROR) {
#else
   if (ret == -1) {
#endif
      _mongoc_socket_capture_errno (sock);

      /*
       * Check to see if we have sent an iovec too large for sendmsg to
       * complete. If so, we need to fallback to the slow path of multiple
       * send() commands.
       */
#ifdef _WIN32
      if (mongoc_socket_errno (sock) == WSAEMSGSIZE) {
#else
      if (mongoc_socket_errno (sock) == EMSGSIZE) {
#endif
         RETURN(_mongoc_socket_try_sendv_slow (sock, iov, iovcnt));
      }

      RETURN (-1);
   }

#ifdef _WIN32
   RETURN (dwNumberofBytesSent);
#else
   RETURN (ret);
#endif
}


/*
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_sendv --
 *
 *       A wrapper around using sendmsg() to send an iovec.
 *       This also deals with the structure differences between
 *       WSABUF and struct iovec.
 *
 *       @expire_at is 0 for no blocking, -1 for infinite blocking,
 *       or a time using the monotonic clock to expire. Calculate this
 *       using bson_get_monotonic_time() + N_MICROSECONDS.
 *
 * Returns:
 *       -1 on failure.
 *       the number of bytes written on success.
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

ssize_t
mongoc_socket_sendv (mongoc_socket_t  *sock,      /* IN */
                     mongoc_iovec_t   *in_iov,    /* IN */
                     size_t            iovcnt,    /* IN */
                     int64_t           expire_at) /* IN */
{
   ssize_t ret = 0;
   ssize_t sent;
   size_t cur = 0;
   mongoc_iovec_t *iov;

   ENTRY;

   BSON_ASSERT (sock);
   BSON_ASSERT (in_iov);
   BSON_ASSERT (iovcnt);

   iov = bson_malloc(sizeof(*iov) * iovcnt);
   memcpy(iov, in_iov, sizeof(*iov) * iovcnt);

   for (;;) {
      sent = _mongoc_socket_try_sendv (sock, &iov [cur], iovcnt - cur);
      TRACE("Sent %ld (of %ld) out of iovcnt=%ld", sent, iov[cur].iov_len, iovcnt);

      /*
       * If we failed with anything other than EAGAIN or EWOULDBLOCK,
       * we should fail immediately as there is another issue with the
       * underlying socket.
       */
      if (sent == -1) {
         if (!_mongoc_socket_errno_is_again (sock)) {
            ret = -1;
            GOTO(CLEANUP);
         }
      }

      /*
       * Update internal stream counters.
       */
      if (sent > 0) {
         ret += sent;
         mongoc_counter_streams_egress_add (sent);

         /*
          * Subtract the sent amount from what we still need to send.
          */
         while ((cur < iovcnt) && (sent >= (ssize_t)iov [cur].iov_len)) {
            TRACE("still got bytes left: sent -= iov_len: %ld -= %ld", sent, iov[cur+1].iov_len);
            sent -= iov [cur++].iov_len;
         }

         /*
          * Check if that made us finish all of the iovecs. If so, we are done
          * sending data over the socket.
          */
         if (cur == iovcnt) {
            TRACE("%s", "Finished the iovecs");
            break;
         }

         /*
          * Increment the current iovec buffer to its proper offset and adjust
          * the number of bytes to write.
          */
         TRACE("Seeked io_base+%ld", sent);
         TRACE("Subtracting iov_len -= sent; %ld -= %ld", iov[cur].iov_len, sent);
         iov [cur].iov_base = ((char *)iov [cur].iov_base) + sent;
         iov [cur].iov_len -= sent;
         TRACE("iov_len remaining %ld", iov[cur].iov_len);

         BSON_ASSERT (iovcnt - cur);
         BSON_ASSERT (iov [cur].iov_len);
      } else if (OPERATION_EXPIRED (expire_at)) {
         GOTO(CLEANUP);
      }

      /*
       * Block on poll() until our desired condition is met.
       */
      if (!_mongoc_socket_wait (sock->sd, POLLOUT, expire_at)) {
         GOTO(CLEANUP);
      }
   }

CLEANUP:
   bson_free(iov);

   RETURN (ret);
}


int
mongoc_socket_getsockname (mongoc_socket_t *sock,    /* IN */
                           struct sockaddr *addr,    /* OUT */
                           socklen_t       *addrlen) /* INOUT */
{
   int ret;

   ENTRY;

   BSON_ASSERT (sock);

   ret = getsockname (sock->sd, addr, addrlen);

   _mongoc_socket_capture_errno (sock);

   RETURN (ret);
}


char *
mongoc_socket_getnameinfo (mongoc_socket_t *sock) /* IN */
{
   struct sockaddr addr;
   socklen_t len = sizeof addr;
   char *ret;
   char host [BSON_HOST_NAME_MAX + 1];

   ENTRY;

   BSON_ASSERT (sock);

   if ((0 == getpeername (sock->sd, &addr, &len)) &&
       (0 == getnameinfo (&addr, len, host, sizeof host, NULL, 0, 0))) {
      ret = bson_strdup (host);
      RETURN (ret);
   }

   RETURN (NULL);
}


bool
mongoc_socket_check_closed (mongoc_socket_t *sock) /* IN */
{
   bool closed = false;
   char buf [1];
   ssize_t r;

   if (_mongoc_socket_wait (sock->sd, POLLIN, 0)) {
      sock->errno_ = 0;

      r = recv (sock->sd, buf, 1, MSG_PEEK);

      if (r < 0) {
         _mongoc_socket_capture_errno (sock);
      }

      if (r < 1) {
         closed = true;
      }
   }

   return closed;
}

/*
 *
 *--------------------------------------------------------------------------
 *
 * mongoc_socket_inet_ntop --
 *
 *       Convert the ip from addrinfo into a c string.
 *
 * Returns:
 *       The value is returned into 'buffer'. The memory has to be allocated
 *       by the caller
 *
 * Side effects:
 *       None.
 *
 *--------------------------------------------------------------------------
 */

void
mongoc_socket_inet_ntop (struct addrinfo *rp,        /* IN */
                         char            *buf,       /* INOUT */
                         size_t           buflen)    /* IN */
{
   void *ptr;
   char tmp[256];

   switch (rp->ai_family) {
   case AF_INET:
      ptr = &((struct sockaddr_in *)rp->ai_addr)->sin_addr;
      inet_ntop (rp->ai_family, ptr, tmp, sizeof (tmp));
      bson_snprintf (buf, buflen, "ipv4 %s", tmp);
      break;
   case AF_INET6:
      ptr = &((struct sockaddr_in6 *)rp->ai_addr)->sin6_addr;
      inet_ntop (rp->ai_family, ptr, tmp, sizeof (tmp));
      bson_snprintf (buf, buflen, "ipv6 %s", tmp);
      break;
   default:
      bson_snprintf (buf, buflen, "unknown ip %d", rp->ai_family);
      break;
   }
}
