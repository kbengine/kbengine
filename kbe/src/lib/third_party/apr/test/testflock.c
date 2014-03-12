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

#include "testflock.h"
#include "testutil.h"
#include "apr_pools.h"
#include "apr_thread_proc.h"
#include "apr_file_io.h"
#include "apr_file_info.h"
#include "apr_general.h"
#include "apr_strings.h"

static int launch_reader(abts_case *tc)
{
    apr_proc_t proc = {0};
    apr_procattr_t *procattr;
    const char *args[2];
    apr_status_t rv;
    apr_exit_why_e why;
    int exitcode;

    rv = apr_procattr_create(&procattr, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't create procattr", rv);

    rv = apr_procattr_io_set(procattr, APR_NO_PIPE, APR_NO_PIPE,
            APR_NO_PIPE);
    APR_ASSERT_SUCCESS(tc, "Couldn't set io in procattr", rv);

    rv = apr_procattr_cmdtype_set(procattr, APR_PROGRAM_ENV);
    APR_ASSERT_SUCCESS(tc, "Couldn't set copy environment", rv);

    rv = apr_procattr_error_check_set(procattr, 1);
    APR_ASSERT_SUCCESS(tc, "Couldn't set error check in procattr", rv);

    args[0] = "tryread" EXTENSION;
    args[1] = NULL;
    rv = apr_proc_create(&proc, TESTBINPATH "tryread" EXTENSION, args, NULL, procattr, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't launch program", rv);

    ABTS_ASSERT(tc, "wait for child process",
            apr_proc_wait(&proc, &exitcode, &why, APR_WAIT) == APR_CHILD_DONE);

    ABTS_ASSERT(tc, "child terminated normally", why == APR_PROC_EXIT);
    return exitcode;
}

static void test_withlock(abts_case *tc, void *data)
{
    apr_file_t *file;
    apr_status_t rv;
    int code;
    
    rv = apr_file_open(&file, TESTFILE, APR_FOPEN_WRITE|APR_FOPEN_CREATE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "Could not create file.", rv);
    ABTS_PTR_NOTNULL(tc, file);

    rv = apr_file_lock(file, APR_FLOCK_EXCLUSIVE);
    APR_ASSERT_SUCCESS(tc, "Could not lock the file.", rv);
    ABTS_PTR_NOTNULL(tc, file);

    code = launch_reader(tc);
    ABTS_INT_EQUAL(tc, FAILED_READ, code);

    (void) apr_file_close(file);
}

static void test_withoutlock(abts_case *tc, void *data)
{
    int code;
    
    code = launch_reader(tc);
    ABTS_INT_EQUAL(tc, SUCCESSFUL_READ, code);
}

static void remove_lockfile(abts_case *tc, void *data)
{
    APR_ASSERT_SUCCESS(tc, "Couldn't remove lock file.",
                       apr_file_remove(TESTFILE, p));
}
    
abts_suite *testflock(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_withlock, NULL);
    abts_run_test(suite, test_withoutlock, NULL);
    abts_run_test(suite, remove_lockfile, NULL);

    return suite;
}
