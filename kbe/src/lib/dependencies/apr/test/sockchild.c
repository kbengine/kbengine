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

#include <stdlib.h>
#include "testsock.h"
#include "apr_network_io.h"
#include "apr_pools.h"

int main(int argc, char *argv[])
{
    apr_pool_t *p;
    apr_socket_t *sock;
    apr_status_t rv;
    apr_sockaddr_t *remote_sa;

    apr_initialize();
    atexit(apr_terminate);
    apr_pool_create(&p, NULL);

    if (argc < 2) {
        exit(-1);
    }

    rv = apr_sockaddr_info_get(&remote_sa, "127.0.0.1", APR_UNSPEC, 8021, 0, p);
    if (rv != APR_SUCCESS) {
        exit(-1);
    }

    if (apr_socket_create(&sock, remote_sa->family, SOCK_STREAM, 0,
                p) != APR_SUCCESS) {
        exit(-1);
    }

    rv = apr_socket_timeout_set(sock, apr_time_from_sec(3));
    if (rv) {
        exit(-1);
    }

    apr_socket_connect(sock, remote_sa);
        
    if (!strcmp("read", argv[1])) {
        char datarecv[STRLEN];
        apr_size_t length = STRLEN;
        apr_status_t rv;

        memset(datarecv, 0, STRLEN);
        rv = apr_socket_recv(sock, datarecv, &length);
        apr_socket_close(sock);
        if (APR_STATUS_IS_TIMEUP(rv)) {
            exit(SOCKET_TIMEOUT); 
        }

        if (strcmp(datarecv, DATASTR)) {
            exit(-1);
        }
        
        exit((int)length);
    }
    else if (!strcmp("write", argv[1])
             || !strcmp("write_after_delay", argv[1])) {
        apr_size_t length = strlen(DATASTR);

        if (!strcmp("write_after_delay", argv[1])) {
            apr_sleep(apr_time_from_sec(2));
        }

        apr_socket_send(sock, DATASTR, &length);

        apr_socket_close(sock);
        exit((int)length);
    }
    else if (!strcmp("close", argv[1])) {
        apr_socket_close(sock);
        exit(0);
    }
    exit(-1);
}
