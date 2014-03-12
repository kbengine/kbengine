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
#include "apr_file_info.h"
#include "apr_fnmatch.h"
#include "apr_tables.h"

/* XXX NUM_FILES must be equal to the nummber of expected files with a
 * .txt extension in the data directory at the time testfnmatch
 * happens to be run (!?!). */

#define NUM_FILES (5)

#define APR_FNM_BITS    15
#define APR_FNM_FAILBIT 256

#define FAILS_IF(X)     0, X
#define SUCCEEDS_IF(X)  X, 256
#define SUCCEEDS        0, 256
#define FAILS           256, 0

static struct pattern_s {
    const char *pattern;
    const char *string;
    int         require_flags;
    int         fail_flags;
} patterns[] = {

/*   Pattern,  String to Test,          Flags to Match  */
    {"", "test",                        FAILS},
    {"", "*",                           FAILS},
    {"test", "*",                       FAILS},
    {"test", "test",                    SUCCEEDS},

     /* Remember C '\\' is a single backslash in pattern */
    {"te\\st", "test",                  FAILS_IF(APR_FNM_NOESCAPE)},
    {"te\\\\st", "te\\st",              FAILS_IF(APR_FNM_NOESCAPE)},
    {"te\\*t", "te*t",                  FAILS_IF(APR_FNM_NOESCAPE)},
    {"te\\*t", "test",                  FAILS},
    {"te\\?t", "te?t",                  FAILS_IF(APR_FNM_NOESCAPE)},
    {"te\\?t", "test",                  FAILS},

    {"tesT", "test",                    SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"test", "Test",                    SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"tEst", "teSt",                    SUCCEEDS_IF(APR_FNM_CASE_BLIND)},

    {"?est", "test",                    SUCCEEDS},
    {"te?t", "test",                    SUCCEEDS},
    {"tes?", "test",                    SUCCEEDS},
    {"test?", "test",                   FAILS},

    {"*", "",                           SUCCEEDS},
    {"*", "test",                       SUCCEEDS},
    {"*test", "test",                   SUCCEEDS},
    {"*est", "test",                    SUCCEEDS},
    {"*st", "test",                     SUCCEEDS},
    {"t*t", "test",                     SUCCEEDS},
    {"te*t", "test",                    SUCCEEDS},
    {"te*st", "test",                   SUCCEEDS},
    {"te*", "test",                     SUCCEEDS},
    {"tes*", "test",                    SUCCEEDS},
    {"test*", "test",                   SUCCEEDS},

    {".[\\-\\t]", ".t",                 SUCCEEDS},
    {"test*?*[a-z]*", "testgoop",       SUCCEEDS},
    {"te[^x]t", "test",                 SUCCEEDS},
    {"te[^abc]t", "test",               SUCCEEDS},
    {"te[^x]t", "test",                 SUCCEEDS},
    {"te[!x]t", "test",                 SUCCEEDS},
    {"te[^x]t", "text",                 FAILS},
    {"te[^\\x]t", "text",               FAILS},
    {"te[^x\\", "text",                 FAILS},
    {"te[/]t", "text",                  FAILS},
    {"te[S]t", "test",                  SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"te[r-t]t", "test",                SUCCEEDS},
    {"te[r-t]t", "teSt",                SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"te[r-T]t", "test",                SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"te[R-T]t", "test",                SUCCEEDS_IF(APR_FNM_CASE_BLIND)},
    {"te[r-Tz]t", "tezt",               SUCCEEDS},
    {"te[R-T]t", "tent",                FAILS},
    {"tes[]t]", "test",                 SUCCEEDS},
    {"tes[t-]", "test",                 SUCCEEDS},
    {"tes[t-]]", "test]",               SUCCEEDS},
    {"tes[t-]]", "test",                FAILS},
    {"tes[u-]", "test",                 FAILS},
    {"tes[t-]", "tes[t-]",              FAILS},
    {"test[/-/]", "test[/-/]",          SUCCEEDS_IF(APR_FNM_PATHNAME)},
    {"test[\\/-/]", "test[/-/]",        APR_FNM_PATHNAME, APR_FNM_NOESCAPE},
    {"test[/-\\/]", "test[/-/]",        APR_FNM_PATHNAME, APR_FNM_NOESCAPE},
    {"test[/-/]", "test/",              FAILS_IF(APR_FNM_PATHNAME)},
    {"test[\\/-/]", "test/",            FAILS_IF(APR_FNM_PATHNAME)},
    {"test[/-\\/]", "test/",            FAILS_IF(APR_FNM_PATHNAME)},

    {"/", "",                           FAILS},
    {"", "/",                           FAILS},
    {"/test", "test",                   FAILS},
    {"test", "/test",                   FAILS},
    {"test/", "test",                   FAILS},
    {"test", "test/",                   FAILS},
    {"\\/test", "/test",                FAILS_IF(APR_FNM_NOESCAPE)},
    {"*test", "/test",                  FAILS_IF(APR_FNM_PATHNAME)},
    {"/*/test/", "/test",               FAILS},
    {"/*/test/", "/test/test/",         SUCCEEDS},
    {"test/this", "test/",              FAILS},
    {"test/", "test/this",              FAILS},
    {"test*/this", "test/this",         SUCCEEDS},
    {"test*/this", "test/that",         FAILS},
    {"test/*this", "test/this",         SUCCEEDS},

    {".*", ".this",                     SUCCEEDS},
    {"*", ".this",                      FAILS_IF(APR_FNM_PERIOD)},
    {"?this", ".this",                  FAILS_IF(APR_FNM_PERIOD)},
    {"[.]this", ".this",                FAILS_IF(APR_FNM_PERIOD)},

    {"test/this", "test/this",          SUCCEEDS},
    {"test?this", "test/this",          FAILS_IF(APR_FNM_PATHNAME)},
    {"test*this", "test/this",          FAILS_IF(APR_FNM_PATHNAME)},
    {"test[/]this", "test/this",        FAILS_IF(APR_FNM_PATHNAME)},

    {"test/.*", "test/.this",           SUCCEEDS},
    {"test/*", "test/.this",            FAILS_IF(APR_FNM_PERIOD | APR_FNM_PATHNAME)},
    {"test/?this", "test/.this",        FAILS_IF(APR_FNM_PERIOD | APR_FNM_PATHNAME)},
    {"test/[.]this", "test/.this",      FAILS_IF(APR_FNM_PERIOD | APR_FNM_PATHNAME)},

    {NULL, NULL, 0}
};



static void test_fnmatch(abts_case *tc, void *data)
{
    struct pattern_s *test = patterns;
    char buf[80];
    int i = APR_FNM_BITS + 1;
    int res;

    for (test = patterns; test->pattern; ++test)
    {
        for (i = 0; i <= APR_FNM_BITS; ++i)
        {
            res = apr_fnmatch(test->pattern, test->string, i);
            if (((i & test->require_flags) != test->require_flags)
                || ((i & test->fail_flags) == test->fail_flags)) {
                if (res != APR_FNM_NOMATCH)
                    break;
            }
            else {
                if (res != 0)
                    break;
            }
        }
        if (i <= APR_FNM_BITS)
            break;
    }

    if (i <= APR_FNM_BITS) {
        sprintf(buf, "apr_fnmatch(\"%s\", \"%s\", %d) returns %d\n",
                test->pattern, test->string, i, res);
        abts_fail(tc, buf, __LINE__);
    }
}

static void test_fnmatch_test(abts_case *tc, void *data)
{
    static const struct test {
        const char *pattern;
        int result;
    } ft_tests[] = {
        { "a*b", 1 },
        { "a?", 1 },
        { "a\\b?", 1 },
        { "a[b-c]", 1 },
        { "a", 0 },
        { "a\\", 0 },
        { NULL, 0 }
    };
    const struct test *t;

    for (t = ft_tests; t->pattern != NULL; t++) {
        int res = apr_fnmatch_test(t->pattern);
        
        if (res != t->result) {
            char buf[128];

            sprintf(buf, "apr_fnmatch_test(\"%s\") = %d, expected %d\n",
                    t->pattern, res, t->result);
            abts_fail(tc, buf, __LINE__);
        }
    }
}

static void test_glob(abts_case *tc, void *data)
{
    int i;
    char **list;
    apr_array_header_t *result;
    
    APR_ASSERT_SUCCESS(tc, "glob match against data/*.txt",
                       apr_match_glob("data\\*.txt", &result, p));

    ABTS_INT_EQUAL(tc, NUM_FILES, result->nelts);

    list = (char **)result->elts;
    for (i = 0; i < result->nelts; i++) {
        char *dot = strrchr(list[i], '.');
        ABTS_STR_EQUAL(tc, ".txt", dot);
    }
}

static void test_glob_currdir(abts_case *tc, void *data)
{
    int i;
    char **list;
    apr_array_header_t *result;
    apr_filepath_set("data", p);
    
    APR_ASSERT_SUCCESS(tc, "glob match against *.txt with data as current",
                       apr_match_glob("*.txt", &result, p));


    ABTS_INT_EQUAL(tc, NUM_FILES, result->nelts);

    list = (char **)result->elts;
    for (i = 0; i < result->nelts; i++) {
        char *dot = strrchr(list[i], '.');
        ABTS_STR_EQUAL(tc, ".txt", dot);
    }
    apr_filepath_set("..", p);
}

abts_suite *testfnmatch(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_fnmatch, NULL);
    abts_run_test(suite, test_fnmatch_test, NULL);
    abts_run_test(suite, test_glob, NULL);
    abts_run_test(suite, test_glob_currdir, NULL);

    return suite;
}

