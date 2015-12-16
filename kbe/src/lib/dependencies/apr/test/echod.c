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

/* Simple echo daemon, designed to be used for network throughput
 * benchmarks. The aim is to allow us to monitor changes in performance
 * of APR networking code, nothing more.
 */

#include <stdio.h>
#include <stdlib.h>  /* for atexit() */

#include "apr.h"
#include "apr_network_io.h"
#include "apr_strings.h"

#define BUF_SIZE 4096

static void reportError(const char *msg, apr_status_t rv, 
                        apr_pool_t *pool)
{
    fprintf(stderr, "%s\nError: %d\n'%s'\n", msg, rv,
            apr_psprintf(pool, "%pm", &rv));
}

static apr_status_t talkTalk(apr_socket_t *socket, apr_pool_t *parent)
{
    apr_pool_t *pool;
    apr_size_t len;
    char *buf;
    apr_status_t rv;

    if (apr_pool_create(&pool, parent) != APR_SUCCESS)
        return APR_ENOPOOL;


    buf = apr_palloc(pool, BUF_SIZE);
    if (!buf)
        return ENOMEM;

    do {
        len = BUF_SIZE;
        rv = apr_socket_recv(socket, buf, &len);
        if (APR_STATUS_IS_EOF(rv) || len == 0 || rv != APR_SUCCESS)
            break;
        rv = apr_socket_send(socket, buf, &len);
        if (len == 0 || rv != APR_SUCCESS)
            break;
    } while (rv == APR_SUCCESS);

    apr_pool_clear(pool);
    return APR_SUCCESS;
}

static apr_status_t glassToWall(apr_port_t port, apr_pool_t *parent)
{
    apr_sockaddr_t *sockAddr;
    apr_socket_t *listener, *accepted;
    apr_status_t rv;

    rv = apr_socket_create(&listener, APR_INET, SOCK_STREAM, APR_PROTO_TCP,
                           parent);
    if (rv != APR_SUCCESS) {
        reportError("Unable to create socket", rv, parent);
        return rv;
    }

    rv = apr_sockaddr_info_get(&sockAddr, "127.0.0.1", APR_UNSPEC,
                               port, 0, parent);
    if (rv != APR_SUCCESS) {
        reportError("Unable to get socket info", rv, parent);
        apr_socket_close(listener);
        return rv;
    }

    if ((rv = apr_socket_bind(listener, sockAddr)) != APR_SUCCESS ||
        (rv = apr_socket_listen(listener, 5)) != APR_SUCCESS) {
        reportError("Unable to bind or listen to socket", rv, parent);
        apr_socket_close(listener);
        return rv;
    }

    for (;;) {
        rv = apr_socket_accept(&accepted, listener, parent);
        if (rv != APR_SUCCESS) {
            reportError("Error accepting on socket", rv, parent);
            break;
        }
        printf("\tAnswering connection\n");
        rv = talkTalk(accepted, parent);
        apr_socket_close(accepted);
        printf("\tConnection closed\n");
        if (rv != APR_SUCCESS)
            break;
    }

    apr_socket_close(listener);
    return APR_SUCCESS;
}

int main(int argc, char **argv)
{
    apr_pool_t *pool;
    apr_port_t theport = 4747;

    printf("APR Test Application: echod\n");

    apr_initialize();
    atexit(apr_terminate);

    apr_pool_create(&pool, NULL);

    if (argc >= 2) {
        printf("argc = %d, port = '%s'\n", argc, argv[1]);
        theport = atoi(argv[1]);
    }

    fprintf(stdout, "Starting to listen on port %d\n", theport);
    glassToWall(theport, pool);

    return 0;
}
