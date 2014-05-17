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

#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_strings.h"
#include "testutil.h"

static void less0(abts_case *tc, void *data)
{
    int rv = apr_strnatcmp("a", "b");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv < 0);
}

static void str_equal(abts_case *tc, void *data)
{
    int rv = apr_strnatcmp("a", "a");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv == 0);
}

static void more0(abts_case *tc, void *data)
{
    int rv = apr_strnatcmp("b", "a");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv > 0);
}

static void less_ignore_case(abts_case *tc, void *data)
{
    int rv = apr_strnatcasecmp("a", "B");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv < 0);
}

static void str_equal_ignore_case(abts_case *tc, void *data)
{
    int rv = apr_strnatcasecmp("a", "A");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv == 0);
}

static void more_ignore_case(abts_case *tc, void *data)
{
    int rv = apr_strnatcasecmp("b", "A");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv > 0);
}

static void natcmp(abts_case *tc, void *data)
{
    int rv = apr_strnatcasecmp("a2", "a10");
    ABTS_ASSERT(tc, "didn't compare simple strings properly", rv < 0);
}

abts_suite *teststrnatcmp(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, less0, NULL);
    abts_run_test(suite, str_equal, NULL);
    abts_run_test(suite, more0, NULL);
    abts_run_test(suite, less_ignore_case, NULL);
    abts_run_test(suite, str_equal_ignore_case, NULL);
    abts_run_test(suite, more_ignore_case, NULL);
    abts_run_test(suite, natcmp, NULL);

    return suite;
}

