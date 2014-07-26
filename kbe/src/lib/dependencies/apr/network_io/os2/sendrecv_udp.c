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
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_network_io.h"
#include "apr_support.h"
#include "apr_lib.h"
#include <sys/time.h>


APR_DECLARE(apr_status_t) apr_socket_sendto(apr_socket_t *sock, 
                                            apr_sockaddr_t *where,
                                            apr_int32_t flags, const char *buf,
                                            apr_size_t *len)
{
    apr_ssize_t rv;
    int serrno;

    do {
        rv = sendto(sock->socketdes, buf, (*len), flags, 
                    (struct sockaddr*)&where->sa,
                    where->salen);
    } while (rv == -1 && (serrno = sock_errno()) == EINTR);

    if (rv == -1 && serrno == SOCEWOULDBLOCK && sock->timeout != 0) {
        apr_status_t arv = apr_wait_for_io_or_timeout(NULL, sock, 0);

        if (arv != APR_SUCCESS) {
            *len = 0;
            return arv;
        } else {
            do {
                rv = sendto(sock->socketdes, buf, *len, flags,
                            (const struct sockaddr*)&where->sa,
                            where->salen);
            } while (rv == -1 && (serrno = sock_errno()) == SOCEINTR);
        }
    }

    if (rv == -1) {
        *len = 0;
        return APR_FROM_OS_ERROR(serrno);
    }

    *len = rv;
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_socket_recvfrom(apr_sockaddr_t *from,
                                              apr_socket_t *sock,
                                              apr_int32_t flags, char *buf,
                                              apr_size_t *len)
{
    apr_ssize_t rv;
    int serrno;

    do {
        rv = recvfrom(sock->socketdes, buf, (*len), flags, 
                      (struct sockaddr*)&from->sa, &from->salen);
    } while (rv == -1 && (serrno = sock_errno()) == EINTR);

    if (rv == -1 && serrno == SOCEWOULDBLOCK && sock->timeout != 0) {
        apr_status_t arv = apr_wait_for_io_or_timeout(NULL, sock, 1);

        if (arv != APR_SUCCESS) {
            *len = 0;
            return arv;
        } else {
            do {
                rv = recvfrom(sock->socketdes, buf, *len, flags,
                              (struct sockaddr*)&from->sa, &from->salen);
                } while (rv == -1 && (serrno = sock_errno()) == EINTR);
        }
    }

    if (rv == -1) {
        (*len) = 0;
        return APR_FROM_OS_ERROR(serrno);
    }

    (*len) = rv;

    if (rv == 0 && sock->type == SOCK_STREAM)
        return APR_EOF;

    return APR_SUCCESS;
}
