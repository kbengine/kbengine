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
#include "apr_arch_misc.h" /* apr_os_level */
#include "apr_network_io.h"
#include "apr_general.h"
#include "apr_strings.h"
#include <string.h>

/* IPV6_V6ONLY is missing from pre-Windows 2008 SDK as well as MinGW
 * (at least up through 1.0.16).
 * Runtime support is a separate issue.
 */
#ifndef IPV6_V6ONLY
#define IPV6_V6ONLY 27
#endif

static apr_status_t soblock(SOCKET sd)
{
    u_long zero = 0;

    if (ioctlsocket(sd, FIONBIO, &zero) == SOCKET_ERROR) {
        return apr_get_netos_error();
    }
    return APR_SUCCESS;
}

static apr_status_t sononblock(SOCKET sd)
{
    u_long one = 1;

    if (ioctlsocket(sd, FIONBIO, &one) == SOCKET_ERROR) {
        return apr_get_netos_error();
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_timeout_set(apr_socket_t *sock, apr_interval_time_t t)
{
    apr_status_t stat;

    if (t == 0) {
        /* Set the socket non-blocking if it was previously blocking */
        if (sock->timeout != 0) {
            if ((stat = sononblock(sock->socketdes)) != APR_SUCCESS)
                return stat;
        }
    }
    else if (t > 0) {
        /* Set the socket to blocking if it was previously non-blocking */
        if (sock->timeout == 0 || apr_is_option_set(sock, APR_SO_NONBLOCK)) {
            if ((stat = soblock(sock->socketdes)) != APR_SUCCESS)
                return stat;
            apr_set_option(sock, APR_SO_NONBLOCK, 0);
        }
        /* Reset socket timeouts if the new timeout differs from the old timeout */
        if (sock->timeout != t) 
        {
            /* Win32 timeouts are in msec, represented as int */
            sock->timeout_ms = (int)apr_time_as_msec(t);
            setsockopt(sock->socketdes, SOL_SOCKET, SO_RCVTIMEO, 
                       (char *) &sock->timeout_ms, 
                       sizeof(sock->timeout_ms));
            setsockopt(sock->socketdes, SOL_SOCKET, SO_SNDTIMEO, 
                       (char *) &sock->timeout_ms, 
                       sizeof(sock->timeout_ms));
        }
    }
    else if (t < 0) {
        int zero = 0;
        /* Set the socket to blocking with infinite timeouts */
        if ((stat = soblock(sock->socketdes)) != APR_SUCCESS)
            return stat;
        setsockopt(sock->socketdes, SOL_SOCKET, SO_RCVTIMEO, 
                   (char *) &zero, sizeof(zero));
        setsockopt(sock->socketdes, SOL_SOCKET, SO_SNDTIMEO, 
                   (char *) &zero, sizeof(zero));
    }
    sock->timeout = t;
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_opt_set(apr_socket_t *sock,
                                             apr_int32_t opt, apr_int32_t on)
{
    int one;
    apr_status_t stat;

    one = on ? 1 : 0;

    switch (opt) {
    case APR_SO_KEEPALIVE:
        if (on != apr_is_option_set(sock, APR_SO_KEEPALIVE)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_KEEPALIVE, 
                           (void *)&one, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_SO_KEEPALIVE, on);
        }
        break;
    case APR_SO_DEBUG:
        if (on != apr_is_option_set(sock, APR_SO_DEBUG)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_DEBUG, 
                           (void *)&one, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_SO_DEBUG, on);
        }
        break;
    case APR_SO_SNDBUF:
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_SNDBUF,
                       (void *)&on, sizeof(int)) == -1) {
            return apr_get_netos_error();
        }
        break;
    case APR_SO_RCVBUF:
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_RCVBUF,
                       (void *)&on, sizeof(int)) == -1) {
            return apr_get_netos_error();
        }
        break;
    case APR_SO_BROADCAST:
        if (on != apr_is_option_set(sock, APR_SO_BROADCAST)) {
           if (setsockopt(sock->socketdes, SOL_SOCKET, SO_BROADCAST, 
                           (void *)&one, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_SO_BROADCAST, on);
        }
        break;
    case APR_SO_REUSEADDR:
        if (on != apr_is_option_set(sock, APR_SO_REUSEADDR)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_REUSEADDR, 
                           (void *)&one, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_SO_REUSEADDR, on);
        }
        break;
    case APR_SO_NONBLOCK:
        if (apr_is_option_set(sock, APR_SO_NONBLOCK) != on) {
            if (on) {
                if ((stat = sononblock(sock->socketdes)) != APR_SUCCESS) 
                    return stat;
            }
            else {
                if ((stat = soblock(sock->socketdes)) != APR_SUCCESS)
                    return stat;
            }
            apr_set_option(sock, APR_SO_NONBLOCK, on);
        }
        break;
    case APR_SO_LINGER:
    {
        if (apr_is_option_set(sock, APR_SO_LINGER) != on) {
            struct linger li;
            li.l_onoff = on;
            li.l_linger = APR_MAX_SECS_TO_LINGER;
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_LINGER, 
                           (char *) &li, sizeof(struct linger)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_SO_LINGER, on);
        }
        break;
    }
    case APR_TCP_DEFER_ACCEPT:
#if defined(TCP_DEFER_ACCEPT)
        if (apr_is_option_set(sock, APR_TCP_DEFER_ACCEPT) != on) {
            int optlevel = IPPROTO_TCP;
            int optname = TCP_DEFER_ACCEPT;

            if (setsockopt(sock->socketdes, optlevel, optname, 
                           (void *)&on, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_TCP_DEFER_ACCEPT, on);
        }
#else
        return APR_ENOTIMPL;
#endif
    case APR_TCP_NODELAY:
        if (apr_is_option_set(sock, APR_TCP_NODELAY) != on) {
            int optlevel = IPPROTO_TCP;
            int optname = TCP_NODELAY;

#if APR_HAVE_SCTP
            if (sock->protocol == IPPROTO_SCTP) {
                optlevel = IPPROTO_SCTP;
                optname = SCTP_NODELAY;
            }
#endif
            if (setsockopt(sock->socketdes, optlevel, optname,
                           (void *)&on, sizeof(int)) == -1) {
                return apr_get_netos_error();
            }
            apr_set_option(sock, APR_TCP_NODELAY, on);
        }
        break;
    case APR_IPV6_V6ONLY:
#if APR_HAVE_IPV6
        if (apr_os_level < APR_WIN_VISTA && 
            sock->local_addr->family == AF_INET6) {
            /* apr_set_option() called at socket creation */
            if (on) {
                return APR_SUCCESS;
            }
            else {
                return APR_ENOTIMPL;
            }
        }
        /* we don't know the initial setting of this option,
         * so don't check sock->options since that optimization
         * won't work
         */
        if (setsockopt(sock->socketdes, IPPROTO_IPV6, IPV6_V6ONLY,
                       (void *)&on, sizeof(int)) == -1) {
            return apr_get_netos_error();
        }
        apr_set_option(sock, APR_IPV6_V6ONLY, on);
#else
        return APR_ENOTIMPL;
#endif
        break;
    default:
        return APR_EINVAL;
        break;
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_timeout_get(apr_socket_t *sock, apr_interval_time_t *t)
{
    *t = sock->timeout;
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_opt_get(apr_socket_t *sock,
                                             apr_int32_t opt, apr_int32_t *on)
{
    switch (opt) {
    case APR_SO_DISCONNECTED:
        *on = sock->disconnected;
        break;
    case APR_SO_KEEPALIVE:
    case APR_SO_DEBUG:
    case APR_SO_REUSEADDR:
    case APR_SO_NONBLOCK:
    case APR_SO_LINGER:
    default:
        *on = apr_is_option_set(sock, opt);
        break;
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_socket_atmark(apr_socket_t *sock, int *atmark)
{
    u_long oobmark;

    if (ioctlsocket(sock->socketdes, SIOCATMARK, (void*) &oobmark) < 0)
        return apr_get_netos_error();

    *atmark = (oobmark != 0);

    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_gethostname(char *buf, int len,
                                          apr_pool_t *cont)
{
    if (gethostname(buf, len) == -1) {
        buf[0] = '\0';
        return apr_get_netos_error();
    }
    else if (!memchr(buf, '\0', len)) { /* buffer too small */
        buf[0] = '\0';
        return APR_ENAMETOOLONG;
    }
    return APR_SUCCESS;
}

