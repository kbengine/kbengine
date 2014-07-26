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

#include <stdio.h>
#include <stdlib.h>
#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "testutil.h"

static apr_pool_t *pool;
static char *testdata;
static int cleanup_called = 0;

static apr_status_t string_cleanup(void *data)
{
    cleanup_called = 1;
    return APR_SUCCESS;
}

static void set_userdata(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_pool_userdata_set(testdata, "TEST", string_cleanup, pool);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void get_userdata(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *retdata;

    rv = apr_pool_userdata_get(&retdata, "TEST", pool);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, testdata, retdata);
}

static void get_nonexistkey(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *retdata;

    rv = apr_pool_userdata_get(&retdata, "DOESNTEXIST", pool);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_EQUAL(tc, NULL, retdata);
}

static void post_pool_clear(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *retdata;

    rv = apr_pool_userdata_get(&retdata, "DOESNTEXIST", pool);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_EQUAL(tc, NULL, retdata);
}

abts_suite *testud(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    apr_pool_create(&pool, p);
    testdata = apr_pstrdup(pool, "This is a test\n");

    abts_run_test(suite, set_userdata, NULL);
    abts_run_test(suite, get_userdata, NULL);
    abts_run_test(suite, get_nonexistkey, NULL);

    apr_pool_clear(pool);

    abts_run_test(suite, post_pool_clear, NULL);

    return suite;
}

