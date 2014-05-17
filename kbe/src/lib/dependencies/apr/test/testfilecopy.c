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
#include "apr_file_info.h"
#include "apr_errno.h"
#include "apr_pools.h"

static void copy_helper(abts_case *tc, const char *from, const char * to,
                        apr_fileperms_t perms, int append, apr_pool_t *p)
{
    apr_status_t rv;
    apr_status_t dest_rv;
    apr_finfo_t copy;
    apr_finfo_t orig;
    apr_finfo_t dest;
    
    dest_rv = apr_stat(&dest, to, APR_FINFO_SIZE, p);
    
    if (!append) {
        rv = apr_file_copy(from, to, perms, p);
    }
    else {
        rv = apr_file_append(from, to, perms, p);
    }
    APR_ASSERT_SUCCESS(tc, "Error copying file", rv);

    rv = apr_stat(&orig, from, APR_FINFO_SIZE, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't stat original file", rv);

    rv = apr_stat(&copy, to, APR_FINFO_SIZE, p);
    APR_ASSERT_SUCCESS(tc, "Couldn't stat copy file", rv);

    if (!append) {
        ABTS_ASSERT(tc, "File size differs", orig.size == copy.size);
    }
    else {
        ABTS_ASSERT(tc, "File size differs", 
			            ((dest_rv == APR_SUCCESS) 
			              ? dest.size : 0) + orig.size == copy.size);
    }
}

static void copy_short_file(abts_case *tc, void *data)
{
    apr_status_t rv;

    /* make absolutely sure that the dest file doesn't exist. */
    apr_file_remove("data/file_copy.txt", p);
    
    copy_helper(tc, "data/file_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 0, p);
    rv = apr_file_remove("data/file_copy.txt", p);
    APR_ASSERT_SUCCESS(tc, "Couldn't remove copy file", rv);
}

static void copy_over_existing(abts_case *tc, void *data)
{
    apr_status_t rv;
    
    /* make absolutely sure that the dest file doesn't exist. */
    apr_file_remove("data/file_copy.txt", p);
    
    /* This is a cheat.  I don't want to create a new file, so I just copy
     * one file, then I copy another.  If the second copy succeeds, then
     * this works.
     */
    copy_helper(tc, "data/file_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 0, p);
    
    copy_helper(tc, "data/mmap_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 0, p);
  
    rv = apr_file_remove("data/file_copy.txt", p);
    APR_ASSERT_SUCCESS(tc, "Couldn't remove copy file", rv);
}

static void append_nonexist(abts_case *tc, void *data)
{
    apr_status_t rv;

    /* make absolutely sure that the dest file doesn't exist. */
    apr_file_remove("data/file_copy.txt", p);

    copy_helper(tc, "data/file_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 0, p);
    rv = apr_file_remove("data/file_copy.txt", p);
    APR_ASSERT_SUCCESS(tc, "Couldn't remove copy file", rv);
}

static void append_exist(abts_case *tc, void *data)
{
    apr_status_t rv;
    
    /* make absolutely sure that the dest file doesn't exist. */
    apr_file_remove("data/file_copy.txt", p);
    
    /* This is a cheat.  I don't want to create a new file, so I just copy
     * one file, then I copy another.  If the second copy succeeds, then
     * this works.
     */
    copy_helper(tc, "data/file_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 0, p);
    
    copy_helper(tc, "data/mmap_datafile.txt", "data/file_copy.txt", 
                APR_FILE_SOURCE_PERMS, 1, p);
  
    rv = apr_file_remove("data/file_copy.txt", p);
    APR_ASSERT_SUCCESS(tc, "Couldn't remove copy file", rv);
}

abts_suite *testfilecopy(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, copy_short_file, NULL);
    abts_run_test(suite, copy_over_existing, NULL);

    abts_run_test(suite, append_nonexist, NULL);
    abts_run_test(suite, append_exist, NULL);

    return suite;
}

