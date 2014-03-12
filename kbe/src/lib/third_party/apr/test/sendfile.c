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

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "apr_network_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_poll.h"
#include "apr_thread_proc.h"

#include "testutil.h"

#if !APR_HAS_SENDFILE
int main(void)
{
    fprintf(stderr, 
            "This program won't work on this platform because there is no "
            "support for sendfile().\n");
    return 0;
}
#else /* !APR_HAS_SENDFILE */

#define FILE_LENGTH    200000

#define FILE_DATA_CHAR '0'

#define HDR1           "1234567890ABCD\n"
#define HDR2           "EFGH\n"
#define HDR3_LEN       80000
#define HDR3_CHAR      '^'
#define TRL1           "IJKLMNOPQRSTUVWXYZ\n"
#define TRL2           "!@#$%&*()\n"
#define TRL3_LEN       90000
#define TRL3_CHAR      '@'

#define TESTSF_PORT    8021

#define TESTFILE       "testsf.dat"

typedef enum {BLK, NONBLK, TIMEOUT} client_socket_mode_t;

static void aprerr(const char *fn, apr_status_t rv)
{
    char buf[120];

    fprintf(stderr, "%s->%d/%s\n",
            fn, rv, apr_strerror(rv, buf, sizeof buf));
    exit(1);
}

static void apr_setup(apr_pool_t *p, apr_socket_t **sock, int *family)
{
    apr_status_t rv;

    *sock = NULL;
    rv = apr_socket_create(sock, *family, SOCK_STREAM, 0, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_create()", rv);
    }

    if (*family == APR_UNSPEC) {
        apr_sockaddr_t *localsa;

        rv = apr_socket_addr_get(&localsa, APR_LOCAL, *sock);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_addr_get()", rv);
        }
        *family = localsa->family;
    }
}

static void create_testfile(apr_pool_t *p, const char *fname)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char buf[120];
    int i;
    apr_finfo_t finfo;

    printf("Creating a test file...\n");
    rv = apr_file_open(&f, fname, 
                 APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE | APR_FOPEN_BUFFERED,
                 APR_UREAD | APR_UWRITE, p);
    if (rv) {
        aprerr("apr_file_open()", rv);
    }
    
    buf[0] = FILE_DATA_CHAR;
    buf[1] = '\0';
    for (i = 0; i < FILE_LENGTH; i++) {
        /* exercise apr_file_putc() and apr_file_puts() on buffered files */
        if ((i % 2) == 0) {
            rv = apr_file_putc(buf[0], f);
            if (rv) {
                aprerr("apr_file_putc()", rv);
            }
        }
        else {
            rv = apr_file_puts(buf, f);
            if (rv) {
                aprerr("apr_file_puts()", rv);
            }
        }
    }

    rv = apr_file_close(f);
    if (rv) {
        aprerr("apr_file_close()", rv);
    }

    rv = apr_stat(&finfo, fname, APR_FINFO_NORM, p);
    if (rv != APR_SUCCESS && ! APR_STATUS_IS_INCOMPLETE(rv)) {
        aprerr("apr_stat()", rv);
    }

    if (finfo.size != FILE_LENGTH) {
        fprintf(stderr, 
                "test file %s should be %ld-bytes long\n"
                "instead it is %ld-bytes long\n",
                fname,
                (long int)FILE_LENGTH,
                (long int)finfo.size);
        exit(1);
    }
}

static void spawn_server(apr_pool_t *p, apr_proc_t *out_proc)
{
    apr_proc_t proc = {0};
    apr_procattr_t *procattr;
    apr_status_t rv;
    const char *args[3];

    rv = apr_procattr_create(&procattr, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_procattr_create()", rv);
    }

    rv = apr_procattr_io_set(procattr, APR_CHILD_BLOCK, APR_CHILD_BLOCK,
                             APR_CHILD_BLOCK);
    if (rv != APR_SUCCESS) {
        aprerr("apr_procattr_io_set()", rv);
    }

    rv = apr_procattr_cmdtype_set(procattr, APR_PROGRAM_ENV);
    if (rv != APR_SUCCESS) {
        aprerr("apr_procattr_cmdtype_set()", rv);
    }

    rv = apr_procattr_error_check_set(procattr, 1);
    if (rv != APR_SUCCESS) {
        aprerr("apr_procattr_error_check_set()", rv);
    }

    args[0] = "sendfile" EXTENSION;
    args[1] = "server";
    args[2] = NULL;
    rv = apr_proc_create(&proc, TESTBINPATH "sendfile" EXTENSION, args, NULL, procattr, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_proc_create()", rv);
    }

    *out_proc = proc;
}

static int client(apr_pool_t *p, client_socket_mode_t socket_mode,
                  const char *host, int start_server)
{
    apr_status_t rv, tmprv;
    apr_socket_t *sock;
    char buf[120];
    apr_file_t *f = NULL;
    apr_size_t len;
    apr_size_t expected_len;
    apr_off_t current_file_offset;
    apr_hdtr_t hdtr;
    struct iovec headers[3];
    struct iovec trailers[3];
    apr_size_t bytes_read;
    apr_pollset_t *pset;
    apr_int32_t nsocks;
    int connect_tries = 1;
    int i;
    int family;
    apr_sockaddr_t *destsa;
    apr_proc_t server;
    apr_interval_time_t connect_retry_interval = apr_time_from_msec(50);

    if (start_server) {
        spawn_server(p, &server);
        connect_tries = 5; /* give it a chance to start up */
    }

    create_testfile(p, TESTFILE);

    rv = apr_file_open(&f, TESTFILE, APR_FOPEN_READ, 0, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_file_open()", rv);
    }

    if (!host) {
        host = "127.0.0.1";
    }
    family = APR_INET;
    rv = apr_sockaddr_info_get(&destsa, host, family, TESTSF_PORT, 0, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_sockaddr_info_get()", rv);
    }

    while (connect_tries--) {
        apr_setup(p, &sock, &family);
        rv = apr_socket_connect(sock, destsa);
        if (connect_tries && APR_STATUS_IS_ECONNREFUSED(rv)) {
            apr_status_t tmprv = apr_socket_close(sock);
            if (tmprv != APR_SUCCESS) {
                aprerr("apr_socket_close()", tmprv);
            }
            apr_sleep(connect_retry_interval);
            connect_retry_interval *= 2;
        }
        else {
            break;
        }
    }
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_connect()", rv);
    }

    switch(socket_mode) {
    case BLK:
        /* leave it blocking */
        break;
    case NONBLK:
        /* set it non-blocking */
        rv = apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_opt_set(APR_SO_NONBLOCK)", rv);
        }
        break;
    case TIMEOUT:
        /* set a timeout */
        rv = apr_socket_timeout_set(sock, 100 * APR_USEC_PER_SEC);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_opt_set(APR_SO_NONBLOCK)", rv);
            exit(1);
        }
        break;
    default:
        assert(1 != 1);
    }

    printf("Sending the file...\n");

    hdtr.headers = headers;
    hdtr.numheaders = 3;
    hdtr.headers[0].iov_base = HDR1;
    hdtr.headers[0].iov_len  = strlen(hdtr.headers[0].iov_base);
    hdtr.headers[1].iov_base = HDR2;
    hdtr.headers[1].iov_len  = strlen(hdtr.headers[1].iov_base);
    hdtr.headers[2].iov_base = malloc(HDR3_LEN);
    assert(hdtr.headers[2].iov_base);
    memset(hdtr.headers[2].iov_base, HDR3_CHAR, HDR3_LEN);
    hdtr.headers[2].iov_len  = HDR3_LEN;

    hdtr.trailers = trailers;
    hdtr.numtrailers = 3;
    hdtr.trailers[0].iov_base = TRL1;
    hdtr.trailers[0].iov_len  = strlen(hdtr.trailers[0].iov_base);
    hdtr.trailers[1].iov_base = TRL2;
    hdtr.trailers[1].iov_len  = strlen(hdtr.trailers[1].iov_base);
    hdtr.trailers[2].iov_base = malloc(TRL3_LEN);
    memset(hdtr.trailers[2].iov_base, TRL3_CHAR, TRL3_LEN);
    assert(hdtr.trailers[2].iov_base);
    hdtr.trailers[2].iov_len  = TRL3_LEN;

    expected_len = 
        strlen(HDR1) + strlen(HDR2) + HDR3_LEN +
        strlen(TRL1) + strlen(TRL2) + TRL3_LEN +
        FILE_LENGTH;
    
    if (socket_mode == BLK) {
        current_file_offset = 0;
        len = FILE_LENGTH;
        rv = apr_socket_sendfile(sock, f, &hdtr, &current_file_offset, &len, 0);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_sendfile()", rv);
        }
        
        printf("apr_socket_sendfile() updated offset with %ld\n",
               (long int)current_file_offset);
        
        printf("apr_socket_sendfile() updated len with %ld\n",
               (long int)len);
        
        printf("bytes really sent: %" APR_SIZE_T_FMT "\n",
               expected_len);

        if (len != expected_len) {
            fprintf(stderr, "apr_socket_sendfile() didn't report the correct "
                    "number of bytes sent!\n");
            exit(1);
        }
    }
    else {
        /* non-blocking... wooooooo */
        apr_size_t total_bytes_sent;
        apr_pollfd_t pfd;

        pset = NULL;
        rv = apr_pollset_create(&pset, 1, p, 0);
        assert(!rv);
        pfd.p = p;
        pfd.desc_type = APR_POLL_SOCKET;
        pfd.reqevents = APR_POLLOUT;
        pfd.rtnevents = 0;
        pfd.desc.s = sock;
        pfd.client_data = NULL;

        rv = apr_pollset_add(pset, &pfd);        
        assert(!rv);

        total_bytes_sent = 0;
        current_file_offset = 0;
        len = FILE_LENGTH;
        do {
            apr_size_t tmplen;

            tmplen = len; /* bytes remaining to send from the file */
            printf("Calling apr_socket_sendfile()...\n");
            printf("Headers (%d):\n", hdtr.numheaders);
            for (i = 0; i < hdtr.numheaders; i++) {
                printf("\t%ld bytes (%c)\n",
                       (long)hdtr.headers[i].iov_len,
                       *(char *)hdtr.headers[i].iov_base);
            }
            printf("File: %ld bytes from offset %ld\n",
                   (long)tmplen, (long)current_file_offset);
            printf("Trailers (%d):\n", hdtr.numtrailers);
            for (i = 0; i < hdtr.numtrailers; i++) {
                printf("\t%ld bytes\n",
                       (long)hdtr.trailers[i].iov_len);
            }

            rv = apr_socket_sendfile(sock, f, &hdtr, &current_file_offset, &tmplen, 0);
            printf("apr_socket_sendfile()->%d, sent %ld bytes\n", rv, (long)tmplen);
            if (rv) {
                if (APR_STATUS_IS_EAGAIN(rv)) {
                    assert(tmplen == 0);
                    nsocks = 1;
                    tmprv = apr_pollset_poll(pset, -1, &nsocks, NULL);
                    assert(!tmprv);
                    assert(nsocks == 1);
                    /* continue; */
                }
            }

            total_bytes_sent += tmplen;

            /* Adjust hdtr to compensate for partially-written
             * data.
             */

            /* First, skip over any header data which might have
             * been written.
             */
            while (tmplen && hdtr.numheaders) {
                if (tmplen >= hdtr.headers[0].iov_len) {
                    tmplen -= hdtr.headers[0].iov_len;
                    --hdtr.numheaders;
                    ++hdtr.headers;
                }
                else {
                    hdtr.headers[0].iov_len -= tmplen;
                    hdtr.headers[0].iov_base = 
			(char*) hdtr.headers[0].iov_base + tmplen;
                    tmplen = 0;
                }
            }

            /* Now, skip over any file data which might have been
             * written.
             */

            if (tmplen <= len) {
                current_file_offset += tmplen;
                len -= tmplen;
                tmplen = 0;
            }
            else {
                tmplen -= len;
                len = 0;
                current_file_offset = 0;
            }

            /* Last, skip over any trailer data which might have
             * been written.
             */

            while (tmplen && hdtr.numtrailers) {
                if (tmplen >= hdtr.trailers[0].iov_len) {
                    tmplen -= hdtr.trailers[0].iov_len;
                    --hdtr.numtrailers;
                    ++hdtr.trailers;
                }
                else {
                    hdtr.trailers[0].iov_len -= tmplen;
                    hdtr.trailers[0].iov_base = 
			(char *)hdtr.trailers[0].iov_base + tmplen;
                    tmplen = 0;
                }
            }

        } while (total_bytes_sent < expected_len &&
                 (rv == APR_SUCCESS || 
                 (APR_STATUS_IS_EAGAIN(rv) && socket_mode != TIMEOUT)));
        if (total_bytes_sent != expected_len) {
            fprintf(stderr,
                    "client problem: sent %ld of %ld bytes\n",
                    (long)total_bytes_sent, (long)expected_len);
            exit(1);
        }

        if (rv) {
            fprintf(stderr,
                    "client problem: rv %d\n",
                    rv);
            exit(1);
        }
    }
    
    current_file_offset = 0;
    rv = apr_file_seek(f, APR_CUR, &current_file_offset);
    if (rv != APR_SUCCESS) {
        aprerr("apr_file_seek()", rv);
    }

    printf("After apr_socket_sendfile(), the kernel file pointer is "
           "at offset %ld.\n",
           (long int)current_file_offset);

    rv = apr_socket_shutdown(sock, APR_SHUTDOWN_WRITE);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_shutdown()", rv);
    }

    /* in case this is the non-blocking test, set socket timeout;
     * we're just waiting for EOF */

    rv = apr_socket_timeout_set(sock, apr_time_from_sec(3));
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_timeout_set()", rv);
    }
    
    bytes_read = 1;
    rv = apr_socket_recv(sock, buf, &bytes_read);
    if (rv != APR_EOF) {
        aprerr("apr_socket_recv() (expected APR_EOF)", rv);
    }
    if (bytes_read != 0) {
        fprintf(stderr, "We expected to get 0 bytes read with APR_EOF\n"
                "but instead we read %ld bytes.\n",
                (long int)bytes_read);
        exit(1);
    }

    printf("client: apr_socket_sendfile() worked as expected!\n");

    rv = apr_file_remove(TESTFILE, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_file_remove()", rv);
    }

    if (start_server) {
        apr_exit_why_e exitwhy;
        apr_size_t nbytes;
        char responsebuf[1024];
        int exitcode;

        rv = apr_file_pipe_timeout_set(server.out, apr_time_from_sec(2));
        if (rv != APR_SUCCESS) {
            aprerr("apr_file_pipe_timeout_set()", rv);
        }
        nbytes = sizeof(responsebuf);
        rv = apr_file_read(server.out, responsebuf, &nbytes);
        if (rv != APR_SUCCESS) {
            aprerr("apr_file_read() messages from server", rv);
        }
        printf("%.*s", (int)nbytes, responsebuf);
        rv = apr_proc_wait(&server, &exitcode, &exitwhy, APR_WAIT);
        if (rv != APR_CHILD_DONE) {
            aprerr("apr_proc_wait() (expected APR_CHILD_DONE)", rv);
        }
        if (exitcode != 0) {
            fprintf(stderr, "sendfile server returned %d\n", exitcode);
            exit(1);
        }
    }

    return 0;
}

static int server(apr_pool_t *p)
{
    apr_status_t rv;
    apr_socket_t *sock;
    char buf[120];
    int i;
    apr_socket_t *newsock = NULL;
    apr_size_t bytes_read;
    apr_sockaddr_t *localsa;
    int family;

    family = APR_INET;
    apr_setup(p, &sock, &family);

    rv = apr_socket_opt_set(sock, APR_SO_REUSEADDR, 1);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_opt_set()", rv);
    }

    rv = apr_sockaddr_info_get(&localsa, NULL, family, TESTSF_PORT, 0, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_sockaddr_info_get()", rv);
    }

    rv = apr_socket_bind(sock, localsa);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_bind()", rv);
    }

    rv = apr_socket_listen(sock, 5);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_listen()", rv);
    }

    printf("Waiting for a client to connect...\n");

    rv = apr_socket_accept(&newsock, sock, p);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_accept()", rv);
    }

    printf("Processing a client...\n");

    assert(sizeof buf > strlen(HDR1));
    bytes_read = strlen(HDR1);
    rv = apr_socket_recv(newsock, buf, &bytes_read);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_recv()", rv);
    }
    if (bytes_read != strlen(HDR1)) {
        fprintf(stderr, "wrong data read (1)\n");
        exit(1);
    }
    if (memcmp(buf, HDR1, strlen(HDR1))) {
        fprintf(stderr, "wrong data read (2)\n");
        fprintf(stderr, "received: `%.*s'\nexpected: `%s'\n",
                (int)bytes_read, buf, HDR1);
        exit(1);
    }
        
    assert(sizeof buf > strlen(HDR2));
    bytes_read = strlen(HDR2);
    rv = apr_socket_recv(newsock, buf, &bytes_read);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_recv()", rv);
    }
    if (bytes_read != strlen(HDR2)) {
        fprintf(stderr, "wrong data read (3)\n");
        exit(1);
    }
    if (memcmp(buf, HDR2, strlen(HDR2))) {
        fprintf(stderr, "wrong data read (4)\n");
        fprintf(stderr, "received: `%.*s'\nexpected: `%s'\n",
                (int)bytes_read, buf, HDR2);
        exit(1);
    }

    for (i = 0; i < HDR3_LEN; i++) {
        bytes_read = 1;
        rv = apr_socket_recv(newsock, buf, &bytes_read);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_recv()", rv);
        }
        if (bytes_read != 1) {
            fprintf(stderr, "apr_socket_recv()->%ld bytes instead of 1\n",
                    (long int)bytes_read);
            exit(1);
        }
        if (buf[0] != HDR3_CHAR) {
            fprintf(stderr,
                    "problem with data read (byte %d of hdr 3):\n",
                    i);
            fprintf(stderr, "read `%c' (0x%x) from client; expected "
                    "`%c'\n",
                    buf[0], buf[0], HDR3_CHAR);
            exit(1);
        }
    }
        
    for (i = 0; i < FILE_LENGTH; i++) {
        bytes_read = 1;
        rv = apr_socket_recv(newsock, buf, &bytes_read);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_recv()", rv);
        }
        if (bytes_read != 1) {
            fprintf(stderr, "apr_socket_recv()->%ld bytes instead of 1\n",
                    (long int)bytes_read);
            exit(1);
        }
        if (buf[0] != FILE_DATA_CHAR) {
            fprintf(stderr,
                    "problem with data read (byte %d of file):\n",
                    i);
            fprintf(stderr, "read `%c' (0x%x) from client; expected "
                    "`%c'\n",
                    buf[0], buf[0], FILE_DATA_CHAR);
            exit(1);
        }
    }
        
    assert(sizeof buf > strlen(TRL1));
    bytes_read = strlen(TRL1);
    rv = apr_socket_recv(newsock, buf, &bytes_read);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_recv()", rv);
    }
    if (bytes_read != strlen(TRL1)) {
        fprintf(stderr, "wrong data read (5)\n");
        exit(1);
    }
    if (memcmp(buf, TRL1, strlen(TRL1))) {
        fprintf(stderr, "wrong data read (6)\n");
        fprintf(stderr, "received: `%.*s'\nexpected: `%s'\n",
                (int)bytes_read, buf, TRL1);
        exit(1);
    }
        
    assert(sizeof buf > strlen(TRL2));
    bytes_read = strlen(TRL2);
    rv = apr_socket_recv(newsock, buf, &bytes_read);
    if (rv != APR_SUCCESS) {
        aprerr("apr_socket_recv()", rv);
    }
    if (bytes_read != strlen(TRL2)) {
        fprintf(stderr, "wrong data read (7)\n");
        exit(1);
    }
    if (memcmp(buf, TRL2, strlen(TRL2))) {
        fprintf(stderr, "wrong data read (8)\n");
        fprintf(stderr, "received: `%.*s'\nexpected: `%s'\n",
                (int)bytes_read, buf, TRL2);
        exit(1);
    }

    for (i = 0; i < TRL3_LEN; i++) {
        bytes_read = 1;
        rv = apr_socket_recv(newsock, buf, &bytes_read);
        if (rv != APR_SUCCESS) {
            aprerr("apr_socket_recv()", rv);
        }
        if (bytes_read != 1) {
            fprintf(stderr, "apr_socket_recv()->%ld bytes instead of 1\n",
                    (long int)bytes_read);
            exit(1);
        }
        if (buf[0] != TRL3_CHAR) {
            fprintf(stderr,
                    "problem with data read (byte %d of trl 3):\n",
                    i);
            fprintf(stderr, "read `%c' (0x%x) from client; expected "
                    "`%c'\n",
                    buf[0], buf[0], TRL3_CHAR);
            exit(1);
        }
    }
        
    bytes_read = 1;
    rv = apr_socket_recv(newsock, buf, &bytes_read);
    if (rv != APR_EOF) {
        aprerr("apr_socket_recv() (expected APR_EOF)", rv);
    }
    if (bytes_read != 0) {
        fprintf(stderr, "We expected to get 0 bytes read with APR_EOF\n"
                "but instead we read %ld bytes (%c).\n",
                (long int)bytes_read, buf[0]);
        exit(1);
    }

    printf("server: apr_socket_sendfile() worked as expected!\n");

    return 0;
}

int main(int argc, char *argv[])
{
    apr_pool_t *p;
    apr_status_t rv;

#ifdef SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif

    rv = apr_initialize();
    if (rv != APR_SUCCESS) {
        aprerr("apr_initialize()", rv);
    }

    atexit(apr_terminate);

    rv = apr_pool_create(&p, NULL);
    if (rv != APR_SUCCESS) {
        aprerr("apr_pool_create()", rv);
    }

    if (argc >= 2 && !strcmp(argv[1], "client")) {
        const char *host = NULL;
        int mode = BLK;
        int start_server = 0;
        int i;

        for (i = 2; i < argc; i++) {
            if (!strcmp(argv[i], "blocking")) {
                mode = BLK;
            }
            else if (!strcmp(argv[i], "timeout")) {
                mode = TIMEOUT;
            }
            else if (!strcmp(argv[i], "nonblocking")) {
                mode = NONBLK;
            }
            else if (!strcmp(argv[i], "startserver")) {
                start_server = 1;
            }
            else {
                host = argv[i];
            }	
        }
        return client(p, mode, host, start_server);
    }
    else if (argc == 2 && !strcmp(argv[1], "server")) {
        return server(p);
    }

    fprintf(stderr, 
            "Usage: %s client {blocking|nonblocking|timeout} [startserver] [server-host]\n"
            "       %s server\n",
            argv[0], argv[0]);
    return -1;
}

#endif /* !APR_HAS_SENDFILE */
