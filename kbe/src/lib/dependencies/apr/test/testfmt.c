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
#include "apr.h"
#include "apr_portable.h"
#include "apr_strings.h"

static void ssize_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_ssize_t var = 0;

    sprintf(buf, "%" APR_SSIZE_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_SSIZE_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
}

static void size_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_size_t var = 0;

    sprintf(buf, "%" APR_SIZE_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_SIZE_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
}

static void time_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_time_t var = 1;

    sprintf(buf, "%" APR_TIME_T_FMT, var);
    ABTS_STR_EQUAL(tc, "1", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_TIME_T_FMT, var);
    ABTS_STR_EQUAL(tc, "1", buf);
}

static void off_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_off_t var = 0;

    sprintf(buf, "%" APR_OFF_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_OFF_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
}

static void pid_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    pid_t var = 0;

    sprintf(buf, "%" APR_PID_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_PID_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
}

static void int64_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_int64_t var = 0;

    sprintf(buf, "%" APR_INT64_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_INT64_T_FMT, var);
    ABTS_STR_EQUAL(tc, "0", buf);
}

static void uint64_t_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_uint64_t var = APR_UINT64_C(14000000);

    sprintf(buf, "%" APR_UINT64_T_FMT, var);
    ABTS_STR_EQUAL(tc, "14000000", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_UINT64_T_FMT, var);
    ABTS_STR_EQUAL(tc, "14000000", buf);
}

static void uint64_t_hex_fmt(abts_case *tc, void *data)
{
    char buf[100];
    apr_uint64_t var = APR_UINT64_C(14000000);

    sprintf(buf, "%" APR_UINT64_T_HEX_FMT, var);
    ABTS_STR_EQUAL(tc, "d59f80", buf);
    apr_snprintf(buf, sizeof(buf), "%" APR_UINT64_T_HEX_FMT, var);
    ABTS_STR_EQUAL(tc, "d59f80", buf);
}

static void more_int64_fmts(abts_case *tc, void *data)
{
    char buf[100];
    apr_int64_t i = APR_INT64_C(-42);
    apr_int64_t ibig = APR_INT64_C(-314159265358979323);
    apr_uint64_t ui = APR_UINT64_C(42);
    apr_uint64_t big = APR_UINT64_C(10267677267010969076);

    apr_snprintf(buf, sizeof buf, "%" APR_INT64_T_FMT, i);
    ABTS_STR_EQUAL(tc, "-42", buf);

    apr_snprintf(buf, sizeof buf, "%" APR_UINT64_T_FMT, ui);
    ABTS_STR_EQUAL(tc, "42", buf);

    apr_snprintf(buf, sizeof buf, "%" APR_UINT64_T_FMT, big);
    ABTS_STR_EQUAL(tc, "10267677267010969076", buf);

    apr_snprintf(buf, sizeof buf, "%" APR_INT64_T_FMT, ibig);
    ABTS_STR_EQUAL(tc, "-314159265358979323", buf);
}

static void error_fmt(abts_case *tc, void *data)
{
    char ebuf[150], sbuf[150], *s;
    apr_status_t rv;

    rv = APR_SUCCESS;
    apr_strerror(rv, ebuf, sizeof ebuf);
    apr_snprintf(sbuf, sizeof sbuf, "%pm", &rv);
    ABTS_STR_EQUAL(tc, sbuf, ebuf);

    rv = APR_ENOTIMPL;
    s = apr_pstrcat(p, "foo-",
                    apr_strerror(rv, ebuf, sizeof ebuf),
                    "-bar", NULL);
    apr_snprintf(sbuf, sizeof sbuf, "foo-%pm-bar", &rv);
    ABTS_STR_EQUAL(tc, sbuf, s);
}

abts_suite *testfmt(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, ssize_t_fmt, NULL);
    abts_run_test(suite, size_t_fmt, NULL);
    abts_run_test(suite, time_t_fmt, NULL);
    abts_run_test(suite, off_t_fmt, NULL);
    abts_run_test(suite, pid_t_fmt, NULL);
    abts_run_test(suite, int64_t_fmt, NULL);
    abts_run_test(suite, uint64_t_fmt, NULL);
    abts_run_test(suite, uint64_t_hex_fmt, NULL);
    abts_run_test(suite, more_int64_fmts, NULL);
    abts_run_test(suite, error_fmt, NULL);

    return suite;
}

