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

#include "testutil.h"
#include "testsock.h"
#include "apr_thread_proc.h"
#include "apr_network_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_poll.h"
#define APR_WANT_BYTEFUNC
#include "apr_want.h"

static void launch_child(abts_case *tc, apr_proc_t *proc, const char *arg1, apr_pool_t *p)
{
    apr_procattr_t *procattr;
    const char *args[3];
    apr_status_t rv;

    rv = apr_procattr_create(&procattr, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't create procattr", rv);

    rv = apr_procattr_io_set(procattr, APR_NO_PIPE, APR_NO_PIPE,
            APR_NO_PIPE);
    APR_ASSERT_SUCCESS(tc, "Couldn't set io in procattr", rv);

    rv = apr_procattr_error_check_set(procattr, 1);
    APR_ASSERT_SUCCESS(tc, "Couldn't set error check in procattr", rv);

    rv = apr_procattr_cmdtype_set(procattr, APR_PROGRAM_ENV);
    APR_ASSERT_SUCCESS(tc, "Couldn't set copy environment", rv);

    args[0] = "sockchild" EXTENSION;
    args[1] = arg1;
    args[2] = NULL;
    rv = apr_proc_create(proc, TESTBINPATH "sockchild" EXTENSION, args, NULL,
                         procattr, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't launch program", rv);
}

static int wait_child(abts_case *tc, apr_proc_t *proc) 
{
    int exitcode;
    apr_exit_why_e why;

    ABTS_ASSERT(tc, "Error waiting for child process",
            apr_proc_wait(proc, &exitcode, &why, APR_WAIT) == APR_CHILD_DONE);

    ABTS_ASSERT(tc, "child terminated normally", why == APR_PROC_EXIT);
    return exitcode;
}

static void test_addr_info(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_sockaddr_t *sa;
    int rc;

    rv = apr_sockaddr_info_get(&sa, NULL, APR_UNSPEC, 80, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);

    rc = apr_sockaddr_is_wildcard(sa);
    ABTS_INT_NEQUAL(tc, 0, rc);

    rv = apr_sockaddr_info_get(&sa, "127.0.0.1", APR_UNSPEC, 80, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);
    ABTS_STR_EQUAL(tc, "127.0.0.1", sa->hostname);

    rc = apr_sockaddr_is_wildcard(sa);
    ABTS_INT_EQUAL(tc, 0, rc);

    rv = apr_sockaddr_info_get(&sa, "127.0.0.1", APR_UNSPEC, 0, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);
    ABTS_STR_EQUAL(tc, "127.0.0.1", sa->hostname);
    ABTS_INT_EQUAL(tc, 0, sa->port);
    ABTS_INT_EQUAL(tc, 0, ntohs(sa->sa.sin.sin_port));
}

static void test_serv_by_name(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_sockaddr_t *sa;

    rv = apr_sockaddr_info_get(&sa, NULL, APR_UNSPEC, 0, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);

    rv = apr_getservbyname(sa, "ftp");
    APR_ASSERT_SUCCESS(tc, "Problem getting ftp service", rv);
    ABTS_INT_EQUAL(tc, 21, sa->port);

    rv = apr_getservbyname(sa, "complete_and_utter_rubbish");
    APR_ASSERT_SUCCESS(tc, "Problem getting non-existent service", !rv);

    rv = apr_getservbyname(sa, "telnet");
    APR_ASSERT_SUCCESS(tc, "Problem getting telnet service", rv);
    ABTS_INT_EQUAL(tc, 23, sa->port);
}

static apr_socket_t *setup_socket(abts_case *tc)
{
    apr_status_t rv;
    apr_sockaddr_t *sa;
    apr_socket_t *sock;

    rv = apr_sockaddr_info_get(&sa, "127.0.0.1", APR_INET, 8021, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);

    rv = apr_socket_create(&sock, sa->family, SOCK_STREAM, APR_PROTO_TCP, p);
    APR_ASSERT_SUCCESS(tc, "Problem creating socket", rv);

    rv = apr_socket_opt_set(sock, APR_SO_REUSEADDR, 1);
    APR_ASSERT_SUCCESS(tc, "Could not set REUSEADDR on socket", rv);
    
    rv = apr_socket_bind(sock, sa);
    APR_ASSERT_SUCCESS(tc, "Problem binding to port", rv);
    if (rv) return NULL;
                
    rv = apr_socket_listen(sock, 5);
    APR_ASSERT_SUCCESS(tc, "Problem listening on socket", rv);

    return sock;
}

static void test_create_bind_listen(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock = setup_socket(tc);
    
    if (!sock) return;
    
    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

static void test_send(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock;
    apr_socket_t *sock2;
    apr_proc_t proc;
    int protocol;
    apr_size_t length;

    sock = setup_socket(tc);
    if (!sock) return;

    launch_child(tc, &proc, "read", p);
    
    rv = apr_socket_accept(&sock2, sock, p);
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    apr_socket_protocol_get(sock2, &protocol);
    ABTS_INT_EQUAL(tc, APR_PROTO_TCP, protocol);
    
    length = strlen(DATASTR);
    apr_socket_send(sock2, DATASTR, &length);

    /* Make sure that the client received the data we sent */
    ABTS_SIZE_EQUAL(tc, strlen(DATASTR), wait_child(tc, &proc));

    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);
    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

static void test_recv(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock;
    apr_socket_t *sock2;
    apr_proc_t proc;
    int protocol;
    apr_size_t length = STRLEN;
    char datastr[STRLEN];
    
    sock = setup_socket(tc);
    if (!sock) return;

    launch_child(tc, &proc, "write", p);
    
    rv = apr_socket_accept(&sock2, sock, p);
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    apr_socket_protocol_get(sock2, &protocol);
    ABTS_INT_EQUAL(tc, APR_PROTO_TCP, protocol);
    
    memset(datastr, 0, STRLEN);
    apr_socket_recv(sock2, datastr, &length);

    /* Make sure that the server received the data we sent */
    ABTS_STR_EQUAL(tc, DATASTR, datastr);
    ABTS_SIZE_EQUAL(tc, strlen(datastr), wait_child(tc, &proc));

    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);
    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

static void test_atreadeof(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock;
    apr_socket_t *sock2;
    apr_proc_t proc;
    apr_size_t length = STRLEN;
    char datastr[STRLEN];
    int atreadeof = -1;

    sock = setup_socket(tc);
    if (!sock) return;

    launch_child(tc, &proc, "write", p);

    rv = apr_socket_accept(&sock2, sock, p);
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    /* Check that the remote socket is still open */
    rv = apr_socket_atreadeof(sock2, &atreadeof);
    APR_ASSERT_SUCCESS(tc, "Determine whether at EOF, #1", rv);
    ABTS_INT_EQUAL(tc, 0, atreadeof);

    memset(datastr, 0, STRLEN);
    apr_socket_recv(sock2, datastr, &length);

    /* Make sure that the server received the data we sent */
    ABTS_STR_EQUAL(tc, DATASTR, datastr);
    ABTS_SIZE_EQUAL(tc, strlen(datastr), wait_child(tc, &proc));

    /* The child is dead, so should be the remote socket */
    rv = apr_socket_atreadeof(sock2, &atreadeof);
    APR_ASSERT_SUCCESS(tc, "Determine whether at EOF, #2", rv);
    ABTS_INT_EQUAL(tc, 1, atreadeof);

    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);

    launch_child(tc, &proc, "close", p);

    rv = apr_socket_accept(&sock2, sock, p);
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    /* The child closed the socket as soon as it could... */
    rv = apr_socket_atreadeof(sock2, &atreadeof);
    APR_ASSERT_SUCCESS(tc, "Determine whether at EOF, #3", rv);
    if (!atreadeof) { /* ... but perhaps not yet; wait a moment */
        apr_sleep(apr_time_from_msec(5));
        rv = apr_socket_atreadeof(sock2, &atreadeof);
        APR_ASSERT_SUCCESS(tc, "Determine whether at EOF, #4", rv);
    }
    ABTS_INT_EQUAL(tc, 1, atreadeof);
    wait_child(tc, &proc);

    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);

    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

static void test_timeout(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock;
    apr_socket_t *sock2;
    apr_proc_t proc;
    int protocol;
    int exit;
    
    sock = setup_socket(tc);
    if (!sock) return;

    launch_child(tc, &proc, "read", p);
    
    rv = apr_socket_accept(&sock2, sock, p);
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    apr_socket_protocol_get(sock2, &protocol);
    ABTS_INT_EQUAL(tc, APR_PROTO_TCP, protocol);
    
    exit = wait_child(tc, &proc);    
    ABTS_INT_EQUAL(tc, SOCKET_TIMEOUT, exit);

    /* We didn't write any data, so make sure the child program returns
     * an error.
     */
    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);
    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

static void test_print_addr(abts_case *tc, void *data)
{
    apr_sockaddr_t *sa;
    apr_status_t rv;
    char *s;

    rv = apr_sockaddr_info_get(&sa, "0.0.0.0", APR_INET, 80, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);

    s = apr_psprintf(p, "foo %pI bar", sa);

    ABTS_STR_EQUAL(tc, "foo 0.0.0.0:80 bar", s);

#if APR_HAVE_IPV6
    rv = apr_sockaddr_info_get(&sa, "::ffff:0.0.0.0", APR_INET6, 80, 0, p);
    APR_ASSERT_SUCCESS(tc, "Problem generating sockaddr", rv);
    if (rv == APR_SUCCESS)
        ABTS_TRUE(tc, sa != NULL);
    if (rv == APR_SUCCESS && sa) {
        /* sa should now be a v4-mapped IPv6 address. */
        char buf[128];
        int rc;

        rc = apr_sockaddr_is_wildcard(sa);
        ABTS_INT_NEQUAL(tc, 0, rc);

        memset(buf, 'z', sizeof buf);
        
        APR_ASSERT_SUCCESS(tc, "could not get IP address",
                           apr_sockaddr_ip_getbuf(buf, 22, sa));
        
        ABTS_STR_EQUAL(tc, "0.0.0.0", buf);
    }
#endif
}

static void test_get_addr(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *ld, *sd, *cd;
    apr_sockaddr_t *sa, *ca;
    apr_pool_t *subp;
    char *a, *b;

    APR_ASSERT_SUCCESS(tc, "create subpool", apr_pool_create(&subp, p));

    ld = setup_socket(tc);
    if (!ld) return;

    APR_ASSERT_SUCCESS(tc,
                       "get local address of bound socket",
                       apr_socket_addr_get(&sa, APR_LOCAL, ld));

    rv = apr_socket_create(&cd, sa->family, SOCK_STREAM,
                           APR_PROTO_TCP, subp);
    APR_ASSERT_SUCCESS(tc, "create client socket", rv);

    APR_ASSERT_SUCCESS(tc, "enable non-block mode",
                       apr_socket_opt_set(cd, APR_SO_NONBLOCK, 1));

    /* It is valid for a connect() on a socket with NONBLOCK set to
     * succeed (if the connection can be established synchronously),
     * but if it does, this test cannot proceed.  */
    rv = apr_socket_connect(cd, sa);
    if (rv == APR_SUCCESS) {
        apr_socket_close(ld);
        apr_socket_close(cd);
        ABTS_NOT_IMPL(tc, "Cannot test if connect completes "
                      "synchronously");
        return;
    }

    if (!APR_STATUS_IS_EINPROGRESS(rv)) {
        apr_socket_close(ld);
        apr_socket_close(cd);
        APR_ASSERT_SUCCESS(tc, "connect to listener", rv);
        return;
    }

    APR_ASSERT_SUCCESS(tc, "accept connection",
                       apr_socket_accept(&sd, ld, subp));
    
    {
        /* wait for writability */
        apr_pollfd_t pfd;
        int n;

        pfd.p = p;
        pfd.desc_type = APR_POLL_SOCKET;
        pfd.reqevents = APR_POLLOUT|APR_POLLHUP;
        pfd.desc.s = cd;
        pfd.client_data = NULL;

        APR_ASSERT_SUCCESS(tc, "poll for connect completion",
                           apr_poll(&pfd, 1, &n, 5 * APR_USEC_PER_SEC));

    }

    APR_ASSERT_SUCCESS(tc, "get local address of server socket",
                       apr_socket_addr_get(&sa, APR_LOCAL, sd));
    APR_ASSERT_SUCCESS(tc, "get remote address of client socket",
                       apr_socket_addr_get(&ca, APR_REMOTE, cd));

    /* Test that the pool of the returned sockaddr objects exactly
     * match the socket. */
    ABTS_PTR_EQUAL(tc, subp, sa->pool);
    ABTS_PTR_EQUAL(tc, subp, ca->pool);

    /* Check equivalence. */
    a = apr_psprintf(p, "%pI fam=%d", sa, sa->family);
    b = apr_psprintf(p, "%pI fam=%d", ca, ca->family);
    ABTS_STR_EQUAL(tc, a, b);

    /* Check pool of returned sockaddr, as above. */
    APR_ASSERT_SUCCESS(tc, "get local address of client socket",
                       apr_socket_addr_get(&sa, APR_LOCAL, cd));
    APR_ASSERT_SUCCESS(tc, "get remote address of server socket",
                       apr_socket_addr_get(&ca, APR_REMOTE, sd));

    /* Check equivalence. */
    a = apr_psprintf(p, "%pI fam=%d", sa, sa->family);
    b = apr_psprintf(p, "%pI fam=%d", ca, ca->family);
    ABTS_STR_EQUAL(tc, a, b);

    ABTS_PTR_EQUAL(tc, subp, sa->pool);
    ABTS_PTR_EQUAL(tc, subp, ca->pool);
                       
    apr_socket_close(cd);
    apr_socket_close(sd);
    apr_socket_close(ld);

    apr_pool_destroy(subp);
}

/* Make sure that setting a connected socket non-blocking works
 * when the listening socket was non-blocking.
 * If APR thinks that non-blocking is inherited but it really
 * isn't, this testcase will fail.
 */
static void test_nonblock_inheritance(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_socket_t *sock;
    apr_socket_t *sock2;
    apr_proc_t proc;
    char buffer[10];
    apr_size_t length;
    int tries;

    sock = setup_socket(tc);
    if (!sock) return;

    rv = apr_socket_opt_set(sock, APR_SO_NONBLOCK, 1);
    APR_ASSERT_SUCCESS(tc, "Could not make listening socket nonblocking", rv);

    launch_child(tc, &proc, "write_after_delay", p);

    tries = 10;
    while (tries--) {
        rv = apr_socket_accept(&sock2, sock, p);
        if (!APR_STATUS_IS_EAGAIN(rv)) {
            break;
        }
        apr_sleep(apr_time_from_msec(50));
    }
    APR_ASSERT_SUCCESS(tc, "Problem with receiving connection", rv);

    rv = apr_socket_opt_set(sock2, APR_SO_NONBLOCK, 1);
    APR_ASSERT_SUCCESS(tc, "Could not make connected socket nonblocking", rv);

    length = sizeof buffer;
    rv = apr_socket_recv(sock2, buffer, &length);
    ABTS_ASSERT(tc, "should have gotten EAGAIN", APR_STATUS_IS_EAGAIN(rv));

    wait_child(tc, &proc);

    rv = apr_socket_close(sock2);
    APR_ASSERT_SUCCESS(tc, "Problem closing connected socket", rv);
    rv = apr_socket_close(sock);
    APR_ASSERT_SUCCESS(tc, "Problem closing socket", rv);
}

abts_suite *testsock(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_addr_info, NULL);
    abts_run_test(suite, test_serv_by_name, NULL);
    abts_run_test(suite, test_create_bind_listen, NULL);
    abts_run_test(suite, test_send, NULL);
    abts_run_test(suite, test_recv, NULL);
    abts_run_test(suite, test_atreadeof, NULL);
    abts_run_test(suite, test_timeout, NULL);
    abts_run_test(suite, test_print_addr, NULL);
    abts_run_test(suite, test_get_addr, NULL);
    abts_run_test(suite, test_nonblock_inheritance, NULL);

    return suite;
}

