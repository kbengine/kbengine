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
#include "apr_file_io.h"
#include "apr_strings.h"

static void test_temp_dir(abts_case *tc, void *data)
{
    const char *tempdir = NULL;
    apr_status_t rv;

    rv = apr_temp_dir_get(&tempdir, p);
    APR_ASSERT_SUCCESS(tc, "Error finding Temporary Directory", rv);
    ABTS_PTR_NOTNULL(tc, tempdir);
}

static void test_mktemp(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    const char *tempdir = NULL;
    char *filetemplate;
    apr_status_t rv;

    rv = apr_temp_dir_get(&tempdir, p);
    APR_ASSERT_SUCCESS(tc, "Error finding Temporary Directory", rv);
    
    filetemplate = apr_pstrcat(p, tempdir, "/tempfileXXXXXX", NULL);
    rv = apr_file_mktemp(&f, filetemplate, 0, p);
    APR_ASSERT_SUCCESS(tc, "Error opening Temporary file", rv);
}

abts_suite *testtemp(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_temp_dir, NULL);
    abts_run_test(suite, test_mktemp, NULL);

    return suite;
}

