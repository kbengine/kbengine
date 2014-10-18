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

#include "apr_env.h"
#include "apr_errno.h"
#include "testutil.h"

#define TEST_ENVVAR_NAME "apr_test_envvar"
#define TEST_ENVVAR2_NAME "apr_test_envvar2"
#define TEST_ENVVAR_VALUE "Just a value that we'll check"

static int have_env_set;
static int have_env_get;
static int have_env_del;

static void test_setenv(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_env_set(TEST_ENVVAR_NAME, TEST_ENVVAR_VALUE, p);
    have_env_set = (rv != APR_ENOTIMPL);
    if (!have_env_set) {
        ABTS_NOT_IMPL(tc, "apr_env_set");
    } else {
        APR_ASSERT_SUCCESS(tc, "set environment variable", rv);
    }
}

static void test_getenv(abts_case *tc, void *data)
{
    char *value;
    apr_status_t rv;

    if (!have_env_set) {
        ABTS_NOT_IMPL(tc, "apr_env_set (skip test for apr_env_get)");
        return;
    }

    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    have_env_get = (rv != APR_ENOTIMPL);
    if (!have_env_get) {
        ABTS_NOT_IMPL(tc, "apr_env_get");
        return;
    }
    APR_ASSERT_SUCCESS(tc, "get environment variable", rv);
    ABTS_STR_EQUAL(tc, TEST_ENVVAR_VALUE, value);
}

static void test_delenv(abts_case *tc, void *data)
{
    char *value;
    apr_status_t rv;

    if (!have_env_set) {
        ABTS_NOT_IMPL(tc, "apr_env_set (skip test for apr_env_delete)");
        return;
    }

    rv = apr_env_delete(TEST_ENVVAR_NAME, p);
    have_env_del = (rv != APR_ENOTIMPL);
    if (!have_env_del) {
        ABTS_NOT_IMPL(tc, "apr_env_delete");
        return;
    }
    APR_ASSERT_SUCCESS(tc, "delete environment variable", rv);

    if (!have_env_get) {
        ABTS_NOT_IMPL(tc, "apr_env_get (skip sanity check for apr_env_delete)");
        return;
    }
    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    ABTS_INT_EQUAL(tc, APR_ENOENT, rv);
}

/** http://issues.apache.org/bugzilla/show_bug.cgi?id=40764 */
static void test_emptyenv(abts_case *tc, void *data)
{
    char *value;
    apr_status_t rv;

    if (!(have_env_set && have_env_get)) {
        ABTS_NOT_IMPL(tc, "apr_env_set (skip test_emptyenv)");
        return;
    }
    /** Set empty string and test that rv != ENOENT) */
    rv = apr_env_set(TEST_ENVVAR_NAME, "", p);
    APR_ASSERT_SUCCESS(tc, "set environment variable", rv);
    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    APR_ASSERT_SUCCESS(tc, "get environment variable", rv);
    ABTS_STR_EQUAL(tc, "", value);

    if (!have_env_del) {
        ABTS_NOT_IMPL(tc, "apr_env (skip recycle test_emptyenv)");
        return;
    }
    /** Delete and retest */
    rv = apr_env_delete(TEST_ENVVAR_NAME, p);
    APR_ASSERT_SUCCESS(tc, "delete environment variable", rv);
    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    ABTS_INT_EQUAL(tc, APR_ENOENT, rv);

    /** Set second variable + test*/
    rv = apr_env_set(TEST_ENVVAR2_NAME, TEST_ENVVAR_VALUE, p);
    APR_ASSERT_SUCCESS(tc, "set second environment variable", rv);
    rv = apr_env_get(&value, TEST_ENVVAR2_NAME, p);
    APR_ASSERT_SUCCESS(tc, "get second environment variable", rv);
    ABTS_STR_EQUAL(tc, TEST_ENVVAR_VALUE, value);

    /** Finally, test ENOENT (first variable) followed by second != ENOENT) */
    rv = apr_env_get(&value, TEST_ENVVAR_NAME, p);
    ABTS_INT_EQUAL(tc, APR_ENOENT, rv);
    rv = apr_env_get(&value, TEST_ENVVAR2_NAME, p);
    APR_ASSERT_SUCCESS(tc, "verify second environment variable", rv);
    ABTS_STR_EQUAL(tc, TEST_ENVVAR_VALUE, value);

    /** Cleanup */
    apr_env_delete(TEST_ENVVAR2_NAME, p);
}

abts_suite *testenv(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_setenv, NULL);
    abts_run_test(suite, test_getenv, NULL);
    abts_run_test(suite, test_delenv, NULL);
    abts_run_test(suite, test_emptyenv, NULL);

    return suite;
}

