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
#include "apr_arch_inherit.h"
#include "apr_network_io.h"
#include "apr_general.h"
#include "apr_portable.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "apr_arch_os2calls.h"

static apr_status_t socket_cleanup(void *sock)
{
    apr_socket_t *thesocket = sock;

    if (thesocket->socketdes < 0) {
        return APR_EINVALSOCK;
    }

    if (soclose(thesocket->socketdes) == 0) {
        thesocket->socketdes = -1;
        return APR_SUCCESS;
    }
    else {
        return APR_OS2_STATUS(sock_errno());
    }
}

static void set_socket_vars(apr_socket_t *sock, int family, int type, int protocol)
{
    sock->type = type;
    sock->protocol = protocol;
    apr_sockaddr_vars_set(sock->local_addr, family, 0);
    apr_sockaddr_vars_set(sock->remote_addr, family, 0);
}

static void alloc_socket(apr_socket_t **new, apr_pool_t *p)
{
    *new = (apr_socket_t *)apr_pcalloc(p, sizeof(apr_socket_t));
    (*new)->pool = p;
    (*new)->local_addr = (apr_sockaddr_t *)apr_pcalloc((*new)->pool,
                                                       sizeof(apr_sockaddr_t));
    (*new)->local_addr->pool = p;

    (*new)->remote_addr = (apr_sockaddr_t *)apr_pcalloc((*new)->pool,
                                                        sizeof(apr_sockaddr_t));
    (*new)->remote_addr->pool = p;
    (*new)->remote_addr_unknown = 1;

    /* Create a pollset with room for one descriptor. */
    /* ### check return codes */
    (void) apr_pollset_create(&(*new)->pollset, 1, p, 0);
}

APR_DECLARE(apr_status_t) apr_socket_protocol_get(apr_socket_t *sock, int *protocol)
{
    *protocol = sock->protocol;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_socket_create(apr_socket_t **new, int family, int type,
                                            int protocol, apr_pool_t *cont)
{
    int downgrade = (family == AF_UNSPEC);
    apr_pollfd_t pfd;

    if (family == AF_UNSPEC) {
#if APR_HAVE_IPV6
        family = AF_INET6;
#else
        family = AF_INET;
#endif
    }

    alloc_socket(new, cont);

    (*new)->socketdes = socket(family, type, protocol);
#if APR_HAVE_IPV6
    if ((*new)->socketdes < 0 && downgrade) {
        family = AF_INET;
        (*new)->socketdes = socket(family, type, protocol);
    }
#endif

    if ((*new)->socketdes < 0) {
        return APR_OS2_STATUS(sock_errno());
    }
    set_socket_vars(*new, family, type, protocol);

    (*new)->timeout = -1;
    (*new)->nonblock = FALSE;
    apr_pool_cleanup_register((*new)->pool, (void *)(*new), 
                        socket_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
} 

APR_DECLARE(apr_status_t) apr_socket_shutdown(apr_socket_t *thesocket, 
                                              apr_shutdown_how_e how)
{
    if (shutdown(thesocket->socketdes, how) == 0) {
        return APR_SUCCESS;
    }
    else {
        return APR_OS2_STATUS(sock_errno());
    }
}

APR_DECLARE(apr_status_t) apr_socket_close(apr_socket_t *thesocket)
{
    apr_pool_cleanup_kill(thesocket->pool, thesocket, socket_cleanup);
    return socket_cleanup(thesocket);
}

APR_DECLARE(apr_status_t) apr_socket_bind(apr_socket_t *sock,
                                          apr_sockaddr_t *sa)
{
    if (bind(sock->socketdes, 
             (struct sockaddr *)&sa->sa,
             sa->salen) == -1)
        return APR_OS2_STATUS(sock_errno());
    else {
        sock->local_addr = sa;
        /* XXX IPv6 - this assumes sin_port and sin6_port at same offset */
        if (sock->local_addr->sa.sin.sin_port == 0) { /* no need for ntohs() when comparing w/ 0 */
            sock->local_port_unknown = 1; /* kernel got us an ephemeral port */
        }
        return APR_SUCCESS;
    }
}

APR_DECLARE(apr_status_t) apr_socket_listen(apr_socket_t *sock, 
                                            apr_int32_t backlog)
{
    if (listen(sock->socketdes, backlog) == -1)
        return APR_OS2_STATUS(sock_errno());
    else
        return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_socket_accept(apr_socket_t **new, 
                                            apr_socket_t *sock,
                                            apr_pool_t *connection_context)
{
    alloc_socket(new, connection_context);
    set_socket_vars(*new, sock->local_addr->sa.sin.sin_family, SOCK_STREAM, sock->protocol);

    (*new)->timeout = -1;
    (*new)->nonblock = FALSE;

    (*new)->socketdes = accept(sock->socketdes, 
                               (struct sockaddr *)&(*new)->remote_addr->sa,
                               &(*new)->remote_addr->salen);

    if ((*new)->socketdes < 0) {
        return APR_OS2_STATUS(sock_errno());
    }

    *(*new)->local_addr = *sock->local_addr;
    (*new)->local_addr->pool = connection_context;
    (*new)->remote_addr->port = ntohs((*new)->remote_addr->sa.sin.sin_port);

    /* fix up any pointers which are no longer valid */
    if (sock->local_addr->sa.sin.sin_family == AF_INET) {
        (*new)->local_addr->ipaddr_ptr = &(*new)->local_addr->sa.sin.sin_addr;
    }

    apr_pool_cleanup_register((*new)->pool, (void *)(*new), 
                        socket_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_socket_connect(apr_socket_t *sock,
                                             apr_sockaddr_t *sa)
{
    if ((connect(sock->socketdes, (struct sockaddr *)&sa->sa.sin, 
                 sa->salen) < 0) &&
        (sock_errno() != SOCEINPROGRESS)) {
        return APR_OS2_STATUS(sock_errno());
    }
    else {
        int namelen = sizeof(sock->local_addr->sa.sin);
        getsockname(sock->socketdes, (struct sockaddr *)&sock->local_addr->sa.sin, 
                    &namelen);
        sock->remote_addr = sa;
        return APR_SUCCESS;
    }
}

APR_DECLARE(apr_status_t) apr_socket_type_get(apr_socket_t *sock, int *type)
{
    *type = sock->type;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_socket_data_get(void **data, const char *key,
                                     apr_socket_t *sock)
{
    sock_userdata_t *cur = sock->userdata;

    *data = NULL;

    while (cur) {
        if (!strcmp(cur->key, key)) {
            *data = cur->data;
            break;
        }
        cur = cur->next;
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_socket_data_set(apr_socket_t *sock, void *data, const char *key,
                                     apr_status_t (*cleanup) (void *))
{
    sock_userdata_t *new = apr_palloc(sock->pool, sizeof(sock_userdata_t));

    new->key = apr_pstrdup(sock->pool, key);
    new->data = data;
    new->next = sock->userdata;
    sock->userdata = new;

    if (cleanup) {
        apr_pool_cleanup_register(sock->pool, data, cleanup, cleanup);
    }

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_sock_get(apr_os_sock_t *thesock, apr_socket_t *sock)
{
    *thesock = sock->socketdes;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_sock_make(apr_socket_t **apr_sock, 
                                           apr_os_sock_info_t *os_sock_info, 
                                           apr_pool_t *cont)
{
    alloc_socket(apr_sock, cont);
    set_socket_vars(*apr_sock, os_sock_info->family, os_sock_info->type, os_sock_info->protocol);
    (*apr_sock)->timeout = -1;
    (*apr_sock)->socketdes = *os_sock_info->os_sock;
    if (os_sock_info->local) {
        memcpy(&(*apr_sock)->local_addr->sa.sin, 
               os_sock_info->local, 
               (*apr_sock)->local_addr->salen);
        /* XXX IPv6 - this assumes sin_port and sin6_port at same offset */
        (*apr_sock)->local_addr->port = ntohs((*apr_sock)->local_addr->sa.sin.sin_port);
    }
    else {
        (*apr_sock)->local_port_unknown = (*apr_sock)->local_interface_unknown = 1;
    }
    if (os_sock_info->remote) {
        memcpy(&(*apr_sock)->remote_addr->sa.sin, 
               os_sock_info->remote,
               (*apr_sock)->remote_addr->salen);
        /* XXX IPv6 - this assumes sin_port and sin6_port at same offset */
        (*apr_sock)->remote_addr->port = ntohs((*apr_sock)->remote_addr->sa.sin.sin_port);
    }
    else {
        (*apr_sock)->remote_addr_unknown = 1;
    }
        
    apr_pool_cleanup_register((*apr_sock)->pool, (void *)(*apr_sock), 
                        socket_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_sock_put(apr_socket_t **sock, apr_os_sock_t *thesock, apr_pool_t *cont)
{
    if (cont == NULL) {
        return APR_ENOPOOL;
    }
    if ((*sock) == NULL) {
        alloc_socket(sock, cont);
        set_socket_vars(*sock, AF_INET, SOCK_STREAM, 0);
        (*sock)->timeout = -1;
    }

    (*sock)->local_port_unknown = (*sock)->local_interface_unknown = 1;
    (*sock)->remote_addr_unknown = 1;
    (*sock)->socketdes = *thesock;
    return APR_SUCCESS;
}

APR_POOL_IMPLEMENT_ACCESSOR(socket);

APR_IMPLEMENT_INHERIT_SET(socket, inherit, pool, socket_cleanup)

APR_IMPLEMENT_INHERIT_UNSET(socket, inherit, pool, socket_cleanup)

