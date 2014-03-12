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


#include "apr_general.h"
#include "apr_pools.h"
#include "apr_errno.h"
#include "apr_file_io.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "testutil.h"

#define ALLOC_BYTES 1024

static apr_pool_t *pmain = NULL;
static apr_pool_t *pchild = NULL;

static void alloc_bytes(abts_case *tc, void *data)
{
    int i;
    char *alloc;
    
    alloc = apr_palloc(pmain, ALLOC_BYTES);
    ABTS_PTR_NOTNULL(tc, alloc);

    for (i=0;i<ALLOC_BYTES;i++) {
        char *ptr = alloc + i;
        *ptr = 0xa;
    }
    /* This is just added to get the positive.  If this test fails, the
     * suite will seg fault.
     */
    ABTS_TRUE(tc, 1);
}

static void calloc_bytes(abts_case *tc, void *data)
{
    int i;
    char *alloc;
    
    alloc = apr_pcalloc(pmain, ALLOC_BYTES);
    ABTS_PTR_NOTNULL(tc, alloc);

    for (i=0;i<ALLOC_BYTES;i++) {
        char *ptr = alloc + i;
        ABTS_TRUE(tc, *ptr == '\0');
    }
}

static void parent_pool(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_pool_create(&pmain, NULL);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, pmain);
}

static void child_pool(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_pool_create(&pchild, pmain);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, pchild);
}

static void test_ancestor(abts_case *tc, void *data)
{
    ABTS_INT_EQUAL(tc, 1, apr_pool_is_ancestor(pmain, pchild));
}

static void test_notancestor(abts_case *tc, void *data)
{
    ABTS_INT_EQUAL(tc, 0, apr_pool_is_ancestor(pchild, pmain));
}

static apr_status_t success_cleanup(void *data)
{
    return APR_SUCCESS;
}

static char *checker_data = "Hello, world.";

static apr_status_t checker_cleanup(void *data)
{
    return data == checker_data ? APR_SUCCESS : APR_EGENERAL;
}

static void test_cleanups(abts_case *tc, void *data)
{
    apr_status_t rv;
    int n;

    /* do this several times to test the cleanup freelist handling. */
    for (n = 0; n < 5; n++) {
        apr_pool_cleanup_register(pchild, NULL, success_cleanup,
                                  success_cleanup);
        apr_pool_cleanup_register(pchild, checker_data, checker_cleanup,
                                  success_cleanup);
        apr_pool_cleanup_register(pchild, NULL, checker_cleanup, 
                                  success_cleanup);

        rv = apr_pool_cleanup_run(p, NULL, success_cleanup);
        ABTS_ASSERT(tc, "nullop cleanup run OK", rv == APR_SUCCESS);
        rv = apr_pool_cleanup_run(p, checker_data, checker_cleanup);
        ABTS_ASSERT(tc, "cleanup passed correct data", rv == APR_SUCCESS);
        rv = apr_pool_cleanup_run(p, NULL, checker_cleanup);
        ABTS_ASSERT(tc, "cleanup passed correct data", rv == APR_EGENERAL);

        if (n == 2) {
            /* clear the pool to check that works */
            apr_pool_clear(pchild);
        }

        if (n % 2 == 0) {
            /* throw another random cleanup into the mix */
            apr_pool_cleanup_register(pchild, NULL,
                                      apr_pool_cleanup_null,
                                      apr_pool_cleanup_null);
        }
    }
}

abts_suite *testpool(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, parent_pool, NULL);
    abts_run_test(suite, child_pool, NULL);
    abts_run_test(suite, test_ancestor, NULL);
    abts_run_test(suite, test_notancestor, NULL);
    abts_run_test(suite, alloc_bytes, NULL);
    abts_run_test(suite, calloc_bytes, NULL);
    abts_run_test(suite, test_cleanups, NULL);

    return suite;
}

