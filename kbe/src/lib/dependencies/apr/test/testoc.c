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
#include "apr_thread_proc.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"

#if APR_HAS_OTHER_CHILD

static char reasonstr[256];

static void ocmaint(int reason, void *data, int status)
{
    switch (reason) {
    case APR_OC_REASON_DEATH:
        apr_cpystrn(reasonstr, "APR_OC_REASON_DEATH", 
                    strlen("APR_OC_REASON_DEATH") + 1);
        break;
    case APR_OC_REASON_LOST:
        apr_cpystrn(reasonstr, "APR_OC_REASON_LOST", 
                    strlen("APR_OC_REASON_LOST") + 1);
        break;
    case APR_OC_REASON_UNWRITABLE:
        apr_cpystrn(reasonstr, "APR_OC_REASON_UNWRITEABLE", 
                    strlen("APR_OC_REASON_UNWRITEABLE") + 1);
        break;
    case APR_OC_REASON_RESTART:
        apr_cpystrn(reasonstr, "APR_OC_REASON_RESTART", 
                    strlen("APR_OC_REASON_RESTART") + 1);
        break;
    }
}

#ifndef SIGKILL
#define SIGKILL 1
#endif

/* It would be great if we could stress this stuff more, and make the test
 * more granular.
 */
static void test_child_kill(abts_case *tc, void *data)
{
    apr_file_t *std = NULL;
    apr_proc_t newproc;
    apr_procattr_t *procattr = NULL;
    const char *args[3];
    apr_status_t rv;

    args[0] = apr_pstrdup(p, "occhild" EXTENSION);
    args[1] = apr_pstrdup(p, "-X");
    args[2] = NULL;

    rv = apr_procattr_create(&procattr, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_procattr_io_set(procattr, APR_FULL_BLOCK, APR_NO_PIPE, 
                             APR_NO_PIPE);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_procattr_cmdtype_set(procattr, APR_PROGRAM_ENV);
    APR_ASSERT_SUCCESS(tc, "Couldn't set copy environment", rv);

    rv = apr_proc_create(&newproc, TESTBINPATH "occhild" EXTENSION, args, NULL, procattr, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, newproc.in);
    ABTS_PTR_EQUAL(tc, NULL, newproc.out);
    ABTS_PTR_EQUAL(tc, NULL, newproc.err);

    std = newproc.in;

    apr_proc_other_child_register(&newproc, ocmaint, NULL, std, p);

    apr_sleep(apr_time_from_sec(1));
    rv = apr_proc_kill(&newproc, SIGKILL);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    /* allow time for things to settle... */
    apr_sleep(apr_time_from_sec(3));
    
    apr_proc_other_child_refresh_all(APR_OC_REASON_RUNNING);
    ABTS_STR_EQUAL(tc, "APR_OC_REASON_DEATH", reasonstr);
}    
#else

static void oc_not_impl(abts_case *tc, void *data)
{
    ABTS_NOT_IMPL(tc, "Other child logic not implemented on this platform");
}
#endif

abts_suite *testoc(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

#if !APR_HAS_OTHER_CHILD
    abts_run_test(suite, oc_not_impl, NULL);
#else

    abts_run_test(suite, test_child_kill, NULL); 

#endif
    return suite;
}

