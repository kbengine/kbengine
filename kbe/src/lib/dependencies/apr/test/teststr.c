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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if APR_HAVE_LIMITS_H
#include <limits.h>
#endif

#include "apr_general.h"
#include "apr_strings.h"
#include "apr_errno.h"

/* I haven't bothered to check for APR_ENOTIMPL here, AFAIK, all string
 * functions exist on all platforms.
 */

static void test_strtok(abts_case *tc, void *data)
{
    struct {
        char *input;
        char *sep;
    }
    cases[] = {
        {
            "",
            "Z"
        },
        {
            "      asdf jkl; 77889909            \r\n\1\2\3Z",
            " \r\n\3\2\1"
        },
        {
            NULL,  /* but who cares if apr_strtok() segfaults? */
            " \t"
        },
#if 0     /* don't do this... you deserve to segfault */
        {
            "a b c              ",
            NULL
        },
#endif
        {
            "   a       b        c   ",
            ""
        },
        {
            "a              b c         ",
            " "
        }
    };
    int curtc;

    for (curtc = 0; curtc < sizeof cases / sizeof cases[0]; curtc++) {
        char *retval1, *retval2;
        char *str1, *str2;
        char *state;

        str1 = apr_pstrdup(p, cases[curtc].input);
        str2 = apr_pstrdup(p, cases[curtc].input);

        do {
            retval1 = apr_strtok(str1, cases[curtc].sep, &state);
            retval2 = strtok(str2, cases[curtc].sep);

            if (!retval1) {
                ABTS_TRUE(tc, retval2 == NULL);
            }
            else {
                ABTS_TRUE(tc, retval2 != NULL);
                ABTS_STR_EQUAL(tc, retval2, retval1);
            }

            str1 = str2 = NULL; /* make sure we pass NULL on subsequent calls */
        } while (retval1);
    }
}

static void snprintf_noNULL(abts_case *tc, void *data)
{
    char buff[100];
    char *testing = apr_palloc(p, 10);

    testing[0] = 't';
    testing[1] = 'e';
    testing[2] = 's';
    testing[3] = 't';
    testing[4] = 'i';
    testing[5] = 'n';
    testing[6] = 'g';
    
    /* If this test fails, we are going to seg fault. */
    apr_snprintf(buff, sizeof(buff), "%.*s", 7, testing);
    ABTS_STR_NEQUAL(tc, buff, testing, 7);
}

static void snprintf_0NULL(abts_case *tc, void *data)
{
    int rv;

    rv = apr_snprintf(NULL, 0, "%sBAR", "FOO");
    ABTS_INT_EQUAL(tc, 6, rv);
}

static void snprintf_0nonNULL(abts_case *tc, void *data)
{
    int rv;
    char *buff = "testing";

    rv = apr_snprintf(buff, 0, "%sBAR", "FOO");
    ABTS_INT_EQUAL(tc, 6, rv);
    ABTS_ASSERT(tc, "buff unmangled", strcmp(buff, "FOOBAR") != 0);
}

static void snprintf_underflow(abts_case *tc, void *data)
{
    char buf[20];
    int rv;

    rv = apr_snprintf(buf, sizeof buf, "%.2f", (double)0.0001);
    ABTS_INT_EQUAL(tc, 4, rv);
    ABTS_STR_EQUAL(tc, "0.00", buf);
    
    rv = apr_snprintf(buf, sizeof buf, "%.2f", (double)0.001);
    ABTS_INT_EQUAL(tc, 4, rv);
    ABTS_STR_EQUAL(tc, "0.00", buf);
    
    rv = apr_snprintf(buf, sizeof buf, "%.2f", (double)0.01);
    ABTS_INT_EQUAL(tc, 4, rv);
    ABTS_STR_EQUAL(tc, "0.01", buf);
}

static void string_error(abts_case *tc, void *data)
{
     char buf[128], *rv;
     apr_status_t n;

     buf[0] = '\0';
     rv = apr_strerror(APR_ENOENT, buf, sizeof buf);
     ABTS_PTR_EQUAL(tc, buf, rv);
     ABTS_TRUE(tc, strlen(buf) > 0);

     rv = apr_strerror(APR_TIMEUP, buf, sizeof buf);
     ABTS_PTR_EQUAL(tc, buf, rv);
     ABTS_STR_EQUAL(tc, "The timeout specified has expired", buf);
     
     /* throw some randomish numbers at it to check for robustness */
     for (n = 1; n < 1000000; n *= 2) {
         apr_strerror(n, buf, sizeof buf);
     }
}

#define SIZE 180000
static void string_long(abts_case *tc, void *data)
{
    char s[SIZE + 1];

    memset(s, 'A', SIZE);
    s[SIZE] = '\0';

    apr_psprintf(p, "%s", s);
}

/* ### FIXME: apr.h/apr_strings.h should provide these! */
#define MY_LLONG_MAX (APR_INT64_C(9223372036854775807))
#define MY_LLONG_MIN (-MY_LLONG_MAX - APR_INT64_C(1))

static void string_strtoi64(abts_case *tc, void *data)
{
    static const struct {
        int errnum, base;
        const char *in, *end;
        apr_int64_t result;
    } ts[] = {
        
        /* base 10 tests */
        { 0, 10, "123545", NULL, APR_INT64_C(123545) },
        { 0, 10, "   123545", NULL, APR_INT64_C(123545) },
        { 0, 10, "   +123545", NULL, APR_INT64_C(123545) },
        { 0, 10, "-123545", NULL, APR_INT64_C(-123545) },
        { 0, 10, "   00000123545", NULL, APR_INT64_C(123545) },
        { 0, 10, "123545ZZZ", "ZZZ", APR_INT64_C(123545) },
        { 0, 10, "   123545   ", "   ", APR_INT64_C(123545) },

        /* base 16 tests */
        { 0, 16, "1E299", NULL, APR_INT64_C(123545) },
        { 0, 16, "1e299", NULL, APR_INT64_C(123545) },
        { 0, 16, "0x1e299", NULL, APR_INT64_C(123545) },
        { 0, 16, "0X1E299", NULL, APR_INT64_C(123545) },
        { 0, 16, "+1e299", NULL, APR_INT64_C(123545) },
        { 0, 16, "-1e299", NULL, APR_INT64_C(-123545) },
        { 0, 16, "   -1e299", NULL, APR_INT64_C(-123545) },

        /* automatic base detection tests */
        { 0, 0, "123545", NULL, APR_INT64_C(123545) },
        { 0, 0, "0x1e299", NULL, APR_INT64_C(123545) },
        { 0, 0, "  0x1e299", NULL, APR_INT64_C(123545) },
        { 0, 0, "+0x1e299", NULL, APR_INT64_C(123545) },
        { 0, 0, "-0x1e299", NULL, APR_INT64_C(-123545) },

        /* large number tests */
        { 0, 10, "8589934605", NULL, APR_INT64_C(8589934605) },
        { 0, 10, "-8589934605", NULL, APR_INT64_C(-8589934605) },
        { 0, 16, "0x20000000D", NULL, APR_INT64_C(8589934605) },
        { 0, 16, "-0x20000000D", NULL, APR_INT64_C(-8589934605) },
        { 0, 16, "   0x20000000D", NULL, APR_INT64_C(8589934605) },
        { 0, 16, "   0x20000000D", NULL, APR_INT64_C(8589934605) },

        /* error cases */
        { ERANGE, 10, "999999999999999999999999999999999", "", MY_LLONG_MAX },
        { ERANGE, 10, "-999999999999999999999999999999999", "", MY_LLONG_MIN },

#if 0
        /* C99 doesn't require EINVAL for an invalid range. */
        { EINVAL, 99, "", (void *)-1 /* don't care */, 0 },
#endif

        /* some strtoll implementations give EINVAL when no conversion
         * is performed. */
        { -1 /* don't care */, 10, "zzz", "zzz", APR_INT64_C(0) },
        { -1 /* don't care */, 10, "", NULL, APR_INT64_C(0) }

    };
    int n;

    for (n = 0; n < sizeof(ts)/sizeof(ts[0]); n++) {
        char *end = "end ptr not changed";
        apr_int64_t result;
        int errnum;
        
        errno = 0;
        result = apr_strtoi64(ts[n].in, &end, ts[n].base);
        errnum = errno;

        ABTS_ASSERT(tc,
                 apr_psprintf(p, "for '%s': result was %" APR_INT64_T_FMT 
                              " not %" APR_INT64_T_FMT, ts[n].in,
                              result, ts[n].result),
                 result == ts[n].result);
        
        if (ts[n].errnum != -1) {
            ABTS_ASSERT(tc,
                     apr_psprintf(p, "for '%s': errno was %d not %d", ts[n].in,
                                  errnum, ts[n].errnum),
                     ts[n].errnum == errnum);
        }

        if (ts[n].end == NULL) {
            /* end must point to NUL terminator of .in */
            ABTS_PTR_EQUAL(tc, ts[n].in + strlen(ts[n].in), end);
        } else if (ts[n].end != (void *)-1) {
            ABTS_ASSERT(tc,
                     apr_psprintf(p, "for '%s', end was '%s' not '%s'",
                                  ts[n].in, end, ts[n].end),
                     strcmp(ts[n].end, end) == 0);
        }
    }
}

static void string_strtoff(abts_case *tc, void *data)
{
    apr_off_t off;

    ABTS_ASSERT(tc, "strtoff fails on out-of-range integer",
                apr_strtoff(&off, "999999999999999999999999999999",
                            NULL, 10) != APR_SUCCESS);

    ABTS_ASSERT(tc, "strtoff failed for 1234",
                apr_strtoff(&off, "1234", NULL, 10) == APR_SUCCESS);

    ABTS_ASSERT(tc, "strtoff failed to parse 1234", off == 1234);
}

/* random-ish checks for strfsize buffer overflows */
static void overflow_strfsize(abts_case *tc, void *data)
{
    apr_off_t off;
    char buf[7];

    buf[5] = '$';
    buf[6] = '@';

    for (off = -9999; off < 20000; off++) {
        apr_strfsize(off, buf);
    }
    for (; off < 9999999; off += 9) {
        apr_strfsize(off, buf);
    }
    for (; off < 999999999; off += 999) {
        apr_strfsize(off, buf);
    }
    for (off = 1; off < LONG_MAX && off > 0; off *= 2) {
        apr_strfsize(off, buf);
        apr_strfsize(off + 1, buf);
        apr_strfsize(off - 1, buf);
    }

    ABTS_ASSERT(tc, "strfsize overflowed", buf[5] == '$');
    ABTS_ASSERT(tc, "strfsize overflowed", buf[6] == '@');
}

static void string_strfsize(abts_case *tc, void *data)
{
    static const struct {
        apr_off_t size;
        const char *buf;
    } ts[] = {
        { -1,   "  - " },
        { 0,    "  0 " },
        { 666,  "666 " },
        { 1024, "1.0K" },
        { 1536, "1.5K" },
        { 2048, "2.0K" },
        { 1293874, "1.2M" },
        { 9999999, "9.5M" },
        { 103809024, " 99M" },
        { 1047527424, "1.0G" } /* "999M" would be more correct */
    };
    apr_size_t n;

    for (n = 0; n < sizeof(ts)/sizeof(ts[0]); n++) {
        char buf[6], *ret;
        
        buf[5] = '%';

        ret = apr_strfsize(ts[n].size, buf);
        ABTS_ASSERT(tc, "strfsize returned wrong buffer", ret == buf);
        ABTS_ASSERT(tc, "strfsize overflowed", buf[5] == '%');

        ABTS_STR_EQUAL(tc, ts[n].buf, ret);
    }
}

static void string_cpystrn(abts_case *tc, void *data)
{
    char buf[6], *ret;
    
    buf[5] = 'Z';

    ret = apr_cpystrn(buf, "123456", 5);

    ABTS_STR_EQUAL(tc, "1234", buf);
    ABTS_PTR_EQUAL(tc, buf + 4, ret);
    ABTS_TRUE(tc, *ret == '\0');
    ABTS_TRUE(tc, ret[1] == 'Z');
}

static void snprintf_overflow(abts_case *tc, void *data)
{
    char buf[4];
    int rv;
    
    buf[2] = '4';
    buf[3] = '2';

    rv = apr_snprintf(buf, 2, "%s", "a");
    ABTS_INT_EQUAL(tc, 1, rv);

    rv = apr_snprintf(buf, 2, "%s", "abcd");
    ABTS_INT_EQUAL(tc, 1, rv);

    ABTS_STR_EQUAL(tc, "a", buf);

    /* Check the buffer really hasn't been overflowed. */
    ABTS_TRUE(tc, buf[2] == '4' && buf[3] == '2');
}

abts_suite *teststr(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, snprintf_0NULL, NULL);
    abts_run_test(suite, snprintf_0nonNULL, NULL);
    abts_run_test(suite, snprintf_noNULL, NULL);
    abts_run_test(suite, snprintf_underflow, NULL);
    abts_run_test(suite, test_strtok, NULL);
    abts_run_test(suite, string_error, NULL);
    abts_run_test(suite, string_long, NULL);
    abts_run_test(suite, string_strtoi64, NULL);
    abts_run_test(suite, string_strtoff, NULL);
    abts_run_test(suite, overflow_strfsize, NULL);
    abts_run_test(suite, string_strfsize, NULL);
    abts_run_test(suite, string_cpystrn, NULL);
    abts_run_test(suite, snprintf_overflow, NULL);

    return suite;
}

