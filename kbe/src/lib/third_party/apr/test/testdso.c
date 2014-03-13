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


#include "apr.h"
#include "testutil.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_errno.h"
#include "apr_dso.h"
#include "apr_strings.h"
#include "apr_file_info.h"
#if APR_HAVE_UNISTD_H
#include <unistd.h>
#endif

#if APR_HAS_DSO

#ifdef NETWARE
# define MOD_NAME "mod_test.nlm"
#elif defined(BEOS) || defined(__MVS__)
# define MOD_NAME "mod_test.so"
#elif defined(WIN32)
# define MOD_NAME TESTBINPATH "mod_test.dll"
#elif defined(DARWIN)
# define MOD_NAME ".libs/mod_test.so" 
# define LIB_NAME ".libs/libmod_test.dylib" 
#elif (defined(__hpux__) || defined(__hpux)) && !defined(__ia64)
# define MOD_NAME ".libs/mod_test.sl"
# define LIB_NAME ".libs/libmod_test.sl"
#elif defined(_AIX) || defined(__bsdi__)
# define MOD_NAME ".libs/libmod_test.so"
# define LIB_NAME ".libs/libmod_test.so"
#else /* Every other Unix */
# define MOD_NAME ".libs/mod_test.so"
# define LIB_NAME ".libs/libmod_test.so"
#endif

static char *modname;

static void test_load_module(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_status_t status;
    char errstr[256];

    status = apr_dso_load(&h, modname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    apr_dso_unload(h);
}

static void test_dso_sym(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_dso_handle_sym_t func1 = NULL;
    apr_status_t status;
    void (*function)(char str[256]);
    char teststr[256];
    char errstr[256];

    status = apr_dso_load(&h, modname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_sym(&func1, h, "print_hello");
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, func1);

    if (!tc->failed) {
        function = (void (*)(char *))func1;
        (*function)(teststr);
        ABTS_STR_EQUAL(tc, "Hello - I'm a DSO!\n", teststr);
    }

    apr_dso_unload(h);
}

static void test_dso_sym_return_value(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_dso_handle_sym_t func1 = NULL;
    apr_status_t status;
    int (*function)(int);
    char errstr[256];

    status = apr_dso_load(&h, modname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_sym(&func1, h, "count_reps");
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, func1);

    if (!tc->failed) {
        function = (int (*)(int))func1;
        status = (*function)(5);
        ABTS_INT_EQUAL(tc, 5, status);
    }

    apr_dso_unload(h);
}

static void test_unload_module(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_status_t status;
    char errstr[256];
    apr_dso_handle_sym_t func1 = NULL;

    status = apr_dso_load(&h, modname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_unload(h);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);

    status = apr_dso_sym(&func1, h, "print_hello");
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ESYMNOTFOUND(status));
}


#ifdef LIB_NAME
static char *libname;

static void test_load_library(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_status_t status;
    char errstr[256];

    status = apr_dso_load(&h, libname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    apr_dso_unload(h);
}

static void test_dso_sym_library(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_dso_handle_sym_t func1 = NULL;
    apr_status_t status;
    void (*function)(char str[256]);
    char teststr[256];
    char errstr[256];

    status = apr_dso_load(&h, libname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_sym(&func1, h, "print_hello");
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, func1);

    if (!tc->failed) {
        function = (void (*)(char *))func1;
        (*function)(teststr);
        ABTS_STR_EQUAL(tc, "Hello - I'm a DSO!\n", teststr);
    }

    apr_dso_unload(h);
}

static void test_dso_sym_return_value_library(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_dso_handle_sym_t func1 = NULL;
    apr_status_t status;
    int (*function)(int);
    char errstr[256];

    status = apr_dso_load(&h, libname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_sym(&func1, h, "count_reps");
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, func1);

    if (!tc->failed) {
        function = (int (*)(int))func1;
        status = (*function)(5);
        ABTS_INT_EQUAL(tc, 5, status);
    }

    apr_dso_unload(h);
}

static void test_unload_library(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_status_t status;
    char errstr[256];
    apr_dso_handle_sym_t func1 = NULL;

    status = apr_dso_load(&h, libname, p);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);
    ABTS_PTR_NOTNULL(tc, h);

    status = apr_dso_unload(h);
    ABTS_ASSERT(tc, apr_dso_error(h, errstr, 256), APR_SUCCESS == status);

    status = apr_dso_sym(&func1, h, "print_hello");
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ESYMNOTFOUND(status));
}

#endif /* def(LIB_NAME) */

static void test_load_notthere(abts_case *tc, void *data)
{
    apr_dso_handle_t *h = NULL;
    apr_status_t status;

    status = apr_dso_load(&h, "No_File.so", p);

    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EDSOOPEN(status));
    ABTS_PTR_NOTNULL(tc, h);
}    

#endif /* APR_HAS_DSO */

abts_suite *testdso(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

#if APR_HAS_DSO
    apr_filepath_merge(&modname, NULL, MOD_NAME, 0, p);
    
    abts_run_test(suite, test_load_module, NULL);
    abts_run_test(suite, test_dso_sym, NULL);
    abts_run_test(suite, test_dso_sym_return_value, NULL);
    abts_run_test(suite, test_unload_module, NULL);

#ifdef LIB_NAME
    apr_filepath_merge(&libname, NULL, LIB_NAME, 0, p);
    
    abts_run_test(suite, test_load_library, NULL);
    abts_run_test(suite, test_dso_sym_library, NULL);
    abts_run_test(suite, test_dso_sym_return_value_library, NULL);
    abts_run_test(suite, test_unload_library, NULL);
#endif

    abts_run_test(suite, test_load_notthere, NULL);
#endif /* APR_HAS_DSO */

    return suite;
}

