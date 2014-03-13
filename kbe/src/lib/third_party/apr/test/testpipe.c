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

#include <stdlib.h>

#include "testutil.h"
#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_thread_proc.h"
#include "apr_strings.h"

static apr_file_t *readp = NULL;
static apr_file_t *writep = NULL;

static void create_pipe(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_file_pipe_create(&readp, &writep, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, readp);
    ABTS_PTR_NOTNULL(tc, writep);
}   

static void close_pipe(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t nbytes = 256;
    char buf[256];

    rv = apr_file_close(readp);
    rv = apr_file_close(writep);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_read(readp, buf, &nbytes);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EBADF(rv));
}   

static void set_timeout(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_interval_time_t timeout;

    rv = apr_file_pipe_create_ex(&readp, &writep, APR_WRITE_BLOCK, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, readp);
    ABTS_PTR_NOTNULL(tc, writep);

    rv = apr_file_pipe_timeout_get(writep, &timeout);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "Timeout mismatch, expected -1", timeout == -1);

    rv = apr_file_pipe_timeout_set(readp, apr_time_from_sec(1));
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_pipe_timeout_get(readp, &timeout);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "Timeout mismatch, expected 1 second", 
		        timeout == apr_time_from_sec(1));
}

static void read_write(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *buf;
    apr_size_t nbytes;
    
    nbytes = strlen("this is a test");
    buf = (char *)apr_palloc(p, nbytes + 1);

    rv = apr_file_pipe_create_ex(&readp, &writep, APR_WRITE_BLOCK, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, readp);
    ABTS_PTR_NOTNULL(tc, writep);

    rv = apr_file_pipe_timeout_set(readp, apr_time_from_sec(1));
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    if (!rv) {
        rv = apr_file_read(readp, buf, &nbytes);
        ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));
        ABTS_SIZE_EQUAL(tc, 0, nbytes);
    }
}

static void read_write_notimeout(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *buf = "this is a test";
    char *input;
    apr_size_t nbytes;
    
    nbytes = strlen("this is a test");

    rv = apr_file_pipe_create(&readp, &writep, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, readp);
    ABTS_PTR_NOTNULL(tc, writep);

    rv = apr_file_write(writep, buf, &nbytes);
    ABTS_SIZE_EQUAL(tc, strlen("this is a test"), nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    nbytes = 256;
    input = apr_pcalloc(p, nbytes + 1);
    rv = apr_file_read(readp, input, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen("this is a test"), nbytes);
    ABTS_STR_EQUAL(tc, "this is a test", input);
}

static void test_pipe_writefull(abts_case *tc, void *data)
{
    int iterations = 1000;
    int i;
    int bytes_per_iteration = 8000;
    char *buf = (char *)malloc(bytes_per_iteration);
    char responsebuf[128];
    apr_size_t nbytes;
    int bytes_processed;
    apr_proc_t proc = {0};
    apr_procattr_t *procattr;
    const char *args[2];
    apr_status_t rv;
    apr_exit_why_e why;
    
    rv = apr_procattr_create(&procattr, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_procattr_io_set(procattr, APR_CHILD_BLOCK, APR_CHILD_BLOCK,
                             APR_CHILD_BLOCK);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_procattr_cmdtype_set(procattr, APR_PROGRAM_ENV);
    APR_ASSERT_SUCCESS(tc, "Couldn't set copy environment", rv);

    rv = apr_procattr_error_check_set(procattr, 1);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    args[0] = "readchild" EXTENSION;
    args[1] = NULL;
    rv = apr_proc_create(&proc, TESTBINPATH "readchild" EXTENSION, args, NULL, procattr, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_pipe_timeout_set(proc.in, apr_time_from_sec(10));
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_pipe_timeout_set(proc.out, apr_time_from_sec(10));
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    i = iterations;
    do {
        rv = apr_file_write_full(proc.in, buf, bytes_per_iteration, NULL);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    } while (--i);

    free(buf);

    rv = apr_file_close(proc.in);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    nbytes = sizeof(responsebuf);
    rv = apr_file_read(proc.out, responsebuf, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    bytes_processed = (int)apr_strtoi64(responsebuf, NULL, 10);
    ABTS_INT_EQUAL(tc, iterations * bytes_per_iteration, bytes_processed);

    ABTS_ASSERT(tc, "wait for child process",
             apr_proc_wait(&proc, NULL, &why, APR_WAIT) == APR_CHILD_DONE);
    
    ABTS_ASSERT(tc, "child terminated normally", why == APR_PROC_EXIT);
}

abts_suite *testpipe(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, create_pipe, NULL);
    abts_run_test(suite, close_pipe, NULL);
    abts_run_test(suite, set_timeout, NULL);
    abts_run_test(suite, close_pipe, NULL);
    abts_run_test(suite, read_write, NULL);
    abts_run_test(suite, close_pipe, NULL);
    abts_run_test(suite, read_write_notimeout, NULL);
    abts_run_test(suite, test_pipe_writefull, NULL);
    abts_run_test(suite, close_pipe, NULL);

    return suite;
}

