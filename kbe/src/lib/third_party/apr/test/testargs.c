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

#include "apr_errno.h"
#include "apr_general.h"
#include "apr_getopt.h"
#include "apr_strings.h"
#include "testutil.h"

static void format_arg(char *str, char option, const char *arg)
{
    if (arg) {
        apr_snprintf(str, 8196, "%soption: %c with %s\n", str, option, arg);
    }
    else {
        apr_snprintf(str, 8196, "%soption: %c\n", str, option);
    }
}

static void unknown_arg(void *str, const char *err, ...)
{
    va_list va;

    va_start(va, err);
    apr_vsnprintf(str, 8196, err, va);
    va_end(va);
}

static void no_options_found(abts_case *tc, void *data)
{
    int largc = 5;
    const char * const largv[] = {"testprog", "-a", "-b", "-c", "-d"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
   
    while (apr_getopt(opt, "abcd", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            default:
                format_arg(str, ch, optarg);
        }
    }
    ABTS_STR_EQUAL(tc, "option: a\n"
                          "option: b\n"
                          "option: c\n"
                          "option: d\n", str);
}

static void no_options(abts_case *tc, void *data)
{
    int largc = 5;
    const char * const largv[] = {"testprog", "-a", "-b", "-c", "-d"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    opt->errfn = unknown_arg;
    opt->errarg = str;
   
    while (apr_getopt(opt, "efgh", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
            case 'b':
            case 'c':
            case 'd':
                format_arg(str, ch, optarg);
                break;
            default:
                break;
        }
    }
    ABTS_STR_EQUAL(tc, "testprog: illegal option -- a\n", str);
}

static void required_option(abts_case *tc, void *data)
{
    int largc = 3;
    const char * const largv[] = {"testprog", "-a", "foo"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    opt->errfn = unknown_arg;
    opt->errarg = str;
   
    while (apr_getopt(opt, "a:", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
                format_arg(str, ch, optarg);
                break;
            default:
                break;
        }
    }
    ABTS_STR_EQUAL(tc, "option: a with foo\n", str);
}

static void required_option_notgiven(abts_case *tc, void *data)
{
    int largc = 2;
    const char * const largv[] = {"testprog", "-a"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    opt->errfn = unknown_arg;
    opt->errarg = str;
   
    while (apr_getopt(opt, "a:", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
                format_arg(str, ch, optarg);
                break;
            default:
                break;
        }
    }
    ABTS_STR_EQUAL(tc, "testprog: option requires an argument -- a\n", str);
}

static void optional_option(abts_case *tc, void *data)
{
    int largc = 3;
    const char * const largv[] = {"testprog", "-a", "foo"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    opt->errfn = unknown_arg;
    opt->errarg = str;
   
    while (apr_getopt(opt, "a::", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
                format_arg(str, ch, optarg);
                break;
            default:
                break;
        }
    }
    ABTS_STR_EQUAL(tc, "option: a with foo\n", str);
}

static void optional_option_notgiven(abts_case *tc, void *data)
{
    int largc = 2;
    const char * const largv[] = {"testprog", "-a"};
    apr_getopt_t *opt;
    apr_status_t rv;
    char ch;
    const char *optarg;
    char str[8196];

    str[0] = '\0';
    rv = apr_getopt_init(&opt, p, largc, largv);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    opt->errfn = unknown_arg;
    opt->errarg = str;
   
    while (apr_getopt(opt, "a::", &ch, &optarg) == APR_SUCCESS) {
        switch (ch) {
            case 'a':
                format_arg(str, ch, optarg);
                break;
            default:
                break;
        }
    }
#if 0
/*  Our version of getopt doesn't allow for optional arguments.  */
    ABTS_STR_EQUAL(tc, "option: a\n", str);
#endif
    ABTS_STR_EQUAL(tc, "testprog: option requires an argument -- a\n", str);
}

abts_suite *testgetopt(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, no_options, NULL);
    abts_run_test(suite, no_options_found, NULL);
    abts_run_test(suite, required_option, NULL);
    abts_run_test(suite, required_option_notgiven, NULL);
    abts_run_test(suite, optional_option, NULL);
    abts_run_test(suite, optional_option_notgiven, NULL);

    return suite;
}
