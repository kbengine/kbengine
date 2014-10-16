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
#include "apr_mmap.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_file_io.h"
#include "apr_strings.h"

/* hmmm, what is a truly portable define for the max path
 * length on a platform?
 */
#define PATH_LEN 255

#if !APR_HAS_MMAP
static void not_implemented(abts_case *tc, void *data)
{
    ABTS_NOT_IMPL(tc, "MMAP functions");
}

#else

static char test_string[256]; /* read from the datafile */
static apr_mmap_t *themmap = NULL;
static apr_file_t *thefile = NULL;
static char *file1;
static apr_finfo_t thisfinfo;
static apr_size_t thisfsize;

static void create_filename(abts_case *tc, void *data)
{
    char *oldfileptr;

    apr_filepath_get(&file1, 0, p);
#ifndef NETWARE
#ifdef WIN32
    ABTS_TRUE(tc, file1[1] == ':');
#else
    ABTS_TRUE(tc, file1[0] == '/');
#endif
#endif
    ABTS_TRUE(tc, file1[strlen(file1) - 1] != '/');

    oldfileptr = file1;
    file1 = apr_pstrcat(p, file1,"/data/mmap_datafile.txt" ,NULL);
    ABTS_TRUE(tc, oldfileptr != file1);
}

static void test_file_close(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_file_close(thefile);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}
   
static void read_expected_contents(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t nbytes = sizeof(test_string) - 1;

    rv = apr_file_read(thefile, test_string, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    test_string[nbytes] = '\0';
    thisfsize = strlen(test_string);
}

static void test_file_open(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_file_open(&thefile, file1, APR_FOPEN_READ, APR_UREAD | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, thefile);
}
   
static void test_get_filesize(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_file_info_get(&thisfinfo, APR_FINFO_NORM, thefile);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "File size mismatch", thisfsize == thisfinfo.size);
}

static void test_mmap_create(abts_case *tc, void *data)
{
    apr_status_t rv;

    rv = apr_mmap_create(&themmap, thefile, 0, (apr_size_t) thisfinfo.size, 
		                 APR_MMAP_READ, p);
    ABTS_PTR_NOTNULL(tc, themmap);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void test_mmap_contents(abts_case *tc, void *data)
{
    
    ABTS_PTR_NOTNULL(tc, themmap);
    ABTS_PTR_NOTNULL(tc, themmap->mm);
    ABTS_SIZE_EQUAL(tc, thisfsize, themmap->size);

    /* Must use nEquals since the string is not guaranteed to be NULL terminated */
    ABTS_STR_NEQUAL(tc, themmap->mm, test_string, thisfsize);
}

static void test_mmap_delete(abts_case *tc, void *data)
{
    apr_status_t rv;

    ABTS_PTR_NOTNULL(tc, themmap);
    rv = apr_mmap_delete(themmap);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void test_mmap_offset(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *addr;

    ABTS_PTR_NOTNULL(tc, themmap);
    rv = apr_mmap_offset(&addr, themmap, 5);

    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    /* Must use nEquals since the string is not guaranteed to be NULL terminated */
    ABTS_STR_NEQUAL(tc, addr, test_string + 5, thisfsize-5);
}
#endif

abts_suite *testmmap(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

#if APR_HAS_MMAP    
    abts_run_test(suite, create_filename, NULL);
    abts_run_test(suite, test_file_open, NULL);
    abts_run_test(suite, read_expected_contents, NULL);
    abts_run_test(suite, test_get_filesize, NULL);
    abts_run_test(suite, test_mmap_create, NULL);
    abts_run_test(suite, test_mmap_contents, NULL);
    abts_run_test(suite, test_mmap_offset, NULL);
    abts_run_test(suite, test_mmap_delete, NULL);
    abts_run_test(suite, test_file_close, NULL);
#else
    abts_run_test(suite, not_implemented, NULL);
#endif

    return suite;
}

