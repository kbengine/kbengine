/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr_arch_networkio.h"
#include "apr_network_io.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/so_ioctl.h>


APR_DECLARE(apr_status_t) apr_socket_timeout_set(apr_socket_t *sock, 
                                                 apr_interval_time_t t)
{
    sock->timeout = t;
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_opt_set(apr_socket_t *sock, 
                                             apr_int32_t opt, apr_int32_t on)
{
    int one;
    struct linger li;

    if (on)
        one = 1;
    else
        one = 0;

    if (opt & APR_SO_KEEPALIVE) {
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_KEEPALIVE, (void *)&one, sizeof(int)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    if (opt & APR_SO_DEBUG) {
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_DEBUG, (void *)&one, sizeof(int)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    if (opt & APR_SO_BROADCAST) {
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_BROADCAST, (void *)&one, sizeof(int)) == -1) {
            return APR_FROM_OS_ERROR(sock_errno());
        }
    }
    if (opt & APR_SO_REUSEADDR) {
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_REUSEADDR, (void *)&one, sizeof(int)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    if (opt & APR_SO_SNDBUF) {
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_SNDBUF, (void *)&on, sizeof(int)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    if (opt & APR_SO_NONBLOCK) {
        if (ioctl(sock->socketdes, FIONBIO, (caddr_t)&one, sizeof(one)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        } else {
            sock->nonblock = one;
        }
    }
    if (opt & APR_SO_LINGER) {
        li.l_onoff = on;
        li.l_linger = APR_MAX_SECS_TO_LINGER;
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_LINGER, (char *) &li, sizeof(struct linger)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    if (opt & APR_TCP_NODELAY) {
        if (setsockopt(sock->socketdes, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(int)) == -1) {
            return APR_OS2_STATUS(sock_errno());
        }
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_timeout_get(apr_socket_t *sock, 
                                                 apr_interval_time_t *t)
{
    *t = sock->timeout;
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_opt_get(apr_socket_t *sock, 
                                             apr_int32_t opt, apr_int32_t *on)
{
    switch(opt) {
    default:
        return APR_EINVAL;
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_atmark(apr_socket_t *sock, int *atmark)
{
    int oobmark;

    if (ioctl(sock->socketdes, SIOCATMARK, (void*)&oobmark, sizeof(oobmark)) < 0) {
        return APR_OS2_STATUS(sock_errno());
    }

    *atmark = (oobmark != 0);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_gethostname(char *buf, apr_int32_t len, 
                                          apr_pool_t *cont)
{
    if (gethostname(buf, len) == -1) {
        buf[0] = '\0';
        return APR_OS2_STATUS(sock_errno());
    }
    else if (!memchr(buf, '\0', len)) { /* buffer too small */
        buf[0] = '\0';
        return APR_ENAMETOOLONG;
    }
    return APR_SUCCESS;
}
