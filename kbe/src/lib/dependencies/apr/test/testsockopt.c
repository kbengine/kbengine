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

#include "apr_network_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "testutil.h"

static apr_socket_t *sock = NULL;

static void create_socket(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_socket_create(&sock, APR_INET, SOCK_STREAM, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, sock);
}

static void set_keepalive(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_int32_t ck;

    rv = apr_socket_opt_set(sock, APR_SO_KEEPALIVE, 1);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_opt_get(sock, APR_SO_KEEPALIVE, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, ck);
}

static void set_debug(abts_case *tc, void *data)
{
    apr_status_t rv1, rv2;
    apr_int32_t ck;
    
    /* On some platforms APR_SO_DEBUG can only be set as root; just test
     * for get/set consistency of this option. */
    rv1 = apr_socket_opt_set(sock, APR_SO_DEBUG, 1);
    rv2 = apr_socket_opt_get(sock, APR_SO_DEBUG, &ck);
    APR_ASSERT_SUCCESS(tc, "get SO_DEBUG option", rv2);
    if (rv1 == APR_SUCCESS) {
        ABTS_INT_EQUAL(tc, 1, ck);
    } else {
        ABTS_INT_EQUAL(tc, 0, ck);
    }
}

static void remove_keepalive(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_int32_t ck;

    rv = apr_socket_opt_get(sock, APR_SO_KEEPALIVE, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, ck);

    rv = apr_socket_opt_set(sock, APR_SO_KEEPALIVE, 0);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_opt_get(sock, APR_SO_KEEPALIVE, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 0, ck);
}

static void corkable(abts_case *tc, void *data)
{
#if !APR_HAVE_CORKABLE_TCP
    ABTS_NOT_IMPL(tc, "TCP isn't corkable");
#else
    apr_status_t rv;
    apr_int32_t ck;

    rv = apr_socket_opt_set(sock, APR_TCP_NODELAY, 1);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_opt_get(sock, APR_TCP_NODELAY, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, ck);

    rv = apr_socket_opt_set(sock, APR_TCP_NOPUSH, 1);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_socket_opt_get(sock, APR_TCP_NOPUSH, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, ck);

    rv = apr_socket_opt_get(sock, APR_TCP_NODELAY, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    /* TCP_NODELAY is now in an unknown state; it may be zero if
     * TCP_NOPUSH and TCP_NODELAY are mutually exclusive on this
     * platform, e.g. Linux < 2.6. */

    rv = apr_socket_opt_set(sock, APR_TCP_NOPUSH, 0);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    rv = apr_socket_opt_get(sock, APR_TCP_NODELAY, &ck);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 1, ck);
#endif
}

static void close_socket(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_socket_close(sock);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

abts_suite *testsockopt(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, create_socket, NULL);
    abts_run_test(suite, set_keepalive, NULL);
    abts_run_test(suite, set_debug, NULL);
    abts_run_test(suite, remove_keepalive, NULL);
    abts_run_test(suite, corkable, NULL);
    abts_run_test(suite, close_socket, NULL);

    return suite;
}

