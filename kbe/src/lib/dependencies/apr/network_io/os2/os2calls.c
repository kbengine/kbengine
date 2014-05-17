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
#include "apr_portable.h"
#include "apr_general.h"
#include "apr_lib.h"

static int os2_socket_init(int, int ,int);

int (*apr_os2_socket)(int, int, int) = os2_socket_init;
int (*apr_os2_select)(int *, int, int, int, long) = NULL;
int (*apr_os2_sock_errno)() = NULL;
int (*apr_os2_accept)(int, struct sockaddr *, int *) = NULL;
int (*apr_os2_bind)(int, struct sockaddr *, int) = NULL;
int (*apr_os2_connect)(int, struct sockaddr *, int) = NULL;
int (*apr_os2_getpeername)(int, struct sockaddr *, int *) = NULL;
int (*apr_os2_getsockname)(int, struct sockaddr *, int *) = NULL;
int (*apr_os2_getsockopt)(int, int, int, char *, int *) = NULL;
int (*apr_os2_ioctl)(int, int, caddr_t, int) = NULL;
int (*apr_os2_listen)(int, int) = NULL;
int (*apr_os2_recv)(int, char *, int, int) = NULL;
int (*apr_os2_send)(int, const char *, int, int) = NULL;
int (*apr_os2_setsockopt)(int, int, int, char *, int) = NULL;
int (*apr_os2_shutdown)(int, int) = NULL;
int (*apr_os2_soclose)(int) = NULL;
int (*apr_os2_writev)(int, struct iovec *, int) = NULL;
int (*apr_os2_sendto)(int, const char *, int, int, const struct sockaddr *, int);
int (*apr_os2_recvfrom)(int, char *, int, int, struct sockaddr *, int *);

static HMODULE hSO32DLL;

static int os2_fn_link()
{
    DosEnterCritSec(); /* Stop two threads doing this at the same time */

    if (apr_os2_socket == os2_socket_init) {
        ULONG rc;
        char errorstr[200];

        rc = DosLoadModule(errorstr, sizeof(errorstr), "SO32DLL", &hSO32DLL);

        if (rc)
            return APR_OS2_STATUS(rc);

        rc = DosQueryProcAddr(hSO32DLL, 0, "SOCKET", &apr_os2_socket);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SELECT", &apr_os2_select);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SOCK_ERRNO", &apr_os2_sock_errno);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "ACCEPT", &apr_os2_accept);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "BIND", &apr_os2_bind);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "CONNECT", &apr_os2_connect);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "GETPEERNAME", &apr_os2_getpeername);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "GETSOCKNAME", &apr_os2_getsockname);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "GETSOCKOPT", &apr_os2_getsockopt);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "IOCTL", &apr_os2_ioctl);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "LISTEN", &apr_os2_listen);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "RECV", &apr_os2_recv);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SEND", &apr_os2_send);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SETSOCKOPT", &apr_os2_setsockopt);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SHUTDOWN", &apr_os2_shutdown);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SOCLOSE", &apr_os2_soclose);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "WRITEV", &apr_os2_writev);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "SENDTO", &apr_os2_sendto);

        if (!rc)
            rc = DosQueryProcAddr(hSO32DLL, 0, "RECVFROM", &apr_os2_recvfrom);

        if (rc)
            return APR_OS2_STATUS(rc);
    }

    DosExitCritSec();
    return APR_SUCCESS;
}



static int os2_socket_init(int domain, int type, int protocol)
{
    int rc = os2_fn_link();
    if (rc == APR_SUCCESS)
        return apr_os2_socket(domain, type, protocol);
    return rc;
}
