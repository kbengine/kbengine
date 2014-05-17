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

#include "apr_file_io.h"
#include "apr_file_info.h"
#include "apr_strings.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_poll.h"
#include "apr_lib.h"
#include "testutil.h"

#define FILENAME "data/file_datafile.txt"
#define NEWFILENAME "data/new_datafile.txt"
#define NEWFILEDATA "This is new text in a new file."

static const struct view_fileinfo
{
    apr_int32_t bits;
    char *description;
} vfi[] = {
    {APR_FINFO_MTIME,  "MTIME"},
    {APR_FINFO_CTIME,  "CTIME"},
    {APR_FINFO_ATIME,  "ATIME"},
    {APR_FINFO_SIZE,   "SIZE"},
    {APR_FINFO_DEV,    "DEV"},
    {APR_FINFO_INODE,  "INODE"},
    {APR_FINFO_NLINK,  "NLINK"},
    {APR_FINFO_TYPE,   "TYPE"},
    {APR_FINFO_USER,   "USER"}, 
    {APR_FINFO_GROUP,  "GROUP"}, 
    {APR_FINFO_UPROT,  "UPROT"}, 
    {APR_FINFO_GPROT,  "GPROT"},
    {APR_FINFO_WPROT,  "WPROT"},
    {0,                NULL}
}; 

static void finfo_equal(abts_case *tc, apr_finfo_t *f1, apr_finfo_t *f2)
{
    /* Minimum supported flags across all platforms (APR_FINFO_MIN) */
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo must return APR_FINFO_TYPE",
             (f1->valid & f2->valid & APR_FINFO_TYPE));
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in filetype",
             f1->filetype == f2->filetype);
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo must return APR_FINFO_SIZE",
             (f1->valid & f2->valid & APR_FINFO_SIZE));
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in size",
             f1->size == f2->size);
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo must return APR_FINFO_ATIME",
             (f1->valid & f2->valid & APR_FINFO_ATIME));
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in atime",
             f1->atime == f2->atime);
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo must return APR_FINFO_MTIME",
             (f1->valid & f2->valid & APR_FINFO_MTIME));
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in mtime",
             f1->mtime == f2->mtime);
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo must return APR_FINFO_CTIME",
             (f1->valid & f2->valid & APR_FINFO_CTIME));
    ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in ctime",
             f1->ctime == f2->ctime);

    if (f1->valid & f2->valid & APR_FINFO_NAME)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in name",
                 !strcmp(f1->name, f2->name));
    if (f1->fname && f2->fname)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in fname",
                 !strcmp(f1->fname, f2->fname));

    /* Additional supported flags not supported on all platforms */
    if (f1->valid & f2->valid & APR_FINFO_USER)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in user",
                 !apr_uid_compare(f1->user, f2->user));
    if (f1->valid & f2->valid & APR_FINFO_GROUP)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in group",
                 !apr_gid_compare(f1->group, f2->group));
    if (f1->valid & f2->valid & APR_FINFO_INODE)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in inode",
                 f1->inode == f2->inode);
    if (f1->valid & f2->valid & APR_FINFO_DEV)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in device",
                 f1->device == f2->device);
    if (f1->valid & f2->valid & APR_FINFO_NLINK)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in nlink",
                 f1->nlink == f2->nlink);
    if (f1->valid & f2->valid & APR_FINFO_CSIZE)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in csize",
                 f1->csize == f2->csize);
    if (f1->valid & f2->valid & APR_FINFO_PROT)
        ABTS_ASSERT(tc, "apr_stat and apr_getfileinfo differ in protection",
                 f1->protection == f2->protection);
}

static void test_info_get(abts_case *tc, void *data)
{
    apr_file_t *thefile;
    apr_finfo_t finfo;
    apr_status_t rv;

    rv = apr_file_open(&thefile, FILENAME, APR_FOPEN_READ, APR_OS_DEFAULT, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_info_get(&finfo, APR_FINFO_NORM, thefile);
    if (APR_STATUS_IS_INCOMPLETE(rv)) {
        char *str;
	int i;
        str = apr_pstrdup(p, "APR_INCOMPLETE:  Missing ");
        for (i = 0; vfi[i].bits; ++i) {
            if (vfi[i].bits & ~finfo.valid) {
                str = apr_pstrcat(p, str, vfi[i].description, " ", NULL);
            }
        }
        ABTS_FAIL(tc, str);
    }
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    apr_file_close(thefile);
}

static void test_stat(abts_case *tc, void *data)
{
    apr_finfo_t finfo;
    apr_status_t rv;

    rv = apr_stat(&finfo, FILENAME, APR_FINFO_NORM, p);
    if (APR_STATUS_IS_INCOMPLETE(rv)) {
        char *str;
	int i;
        str = apr_pstrdup(p, "APR_INCOMPLETE:  Missing ");
        for (i = 0; vfi[i].bits; ++i) {
            if (vfi[i].bits & ~finfo.valid) {
                str = apr_pstrcat(p, str, vfi[i].description, " ", NULL);
            }
        }
        ABTS_FAIL(tc, str);
    }
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void test_stat_eq_finfo(abts_case *tc, void *data)
{
    apr_file_t *thefile;
    apr_finfo_t finfo;
    apr_finfo_t stat_finfo;
    apr_status_t rv;

    rv = apr_file_open(&thefile, FILENAME, APR_FOPEN_READ, APR_OS_DEFAULT, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    rv = apr_file_info_get(&finfo, APR_FINFO_NORM, thefile);

    /* Opening the file may have toggled the atime member (time last
     * accessed), so fetch our apr_stat() after getting the fileinfo 
     * of the open file...
     */
    rv = apr_stat(&stat_finfo, FILENAME, APR_FINFO_NORM, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_close(thefile);

    finfo_equal(tc, &stat_finfo, &finfo);
}

static void test_buffered_write_size(abts_case *tc, void *data)
{
    const apr_size_t data_len = strlen(NEWFILEDATA);
    apr_file_t *thefile;
    apr_finfo_t finfo;
    apr_status_t rv;
    apr_size_t bytes;

    rv = apr_file_open(&thefile, NEWFILENAME,
                       APR_FOPEN_READ | APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE
                       | APR_FOPEN_BUFFERED | APR_FOPEN_DELONCLOSE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open file", rv);

    /* A funny thing happened to me the other day: I wrote something
     * into a buffered file, then asked for its size using
     * apr_file_info_get; and guess what? The size was 0! That's not a
     * nice way to behave.
     */
    bytes = data_len;
    rv = apr_file_write(thefile, NEWFILEDATA, &bytes);
    APR_ASSERT_SUCCESS(tc, "write file contents", rv);
    ABTS_TRUE(tc, data_len == bytes);

    rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, thefile);
    APR_ASSERT_SUCCESS(tc, "get file size", rv);
    ABTS_TRUE(tc, bytes == (apr_size_t) finfo.size);
    apr_file_close(thefile);
}

static void test_mtime_set(abts_case *tc, void *data)
{
    apr_file_t *thefile;
    apr_finfo_t finfo;
    apr_time_t epoch = 0;
    apr_status_t rv;

    /* This test sort of depends on the system clock being at least
     * marginally ccorrect; We'll be setting the modification time to
     * the epoch.
     */
    rv = apr_file_open(&thefile, NEWFILENAME,
                       APR_FOPEN_READ | APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE
                       | APR_FOPEN_BUFFERED | APR_FOPEN_DELONCLOSE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open file", rv);

    /* Check that the current mtime is not the epoch */
    rv = apr_stat(&finfo, NEWFILENAME, APR_FINFO_MTIME, p);
    if (APR_STATUS_IS_INCOMPLETE(rv)) {
        char *str;
	int i;
        str = apr_pstrdup(p, "APR_INCOMPLETE:  Missing ");
        for (i = 0; vfi[i].bits; ++i) {
            if (vfi[i].bits & ~finfo.valid) {
                str = apr_pstrcat(p, str, vfi[i].description, " ", NULL);
            }
        }
        ABTS_FAIL(tc, str);
    }
    APR_ASSERT_SUCCESS(tc, "get initial mtime", rv);
    ABTS_TRUE(tc, finfo.mtime != epoch);

    /* Reset the mtime to the epoch and verify the result.
     * Note: we blindly assume that if the first apr_stat succeeded,
     * the second one will, too.
     */
    rv = apr_file_mtime_set(NEWFILENAME, epoch, p);
    APR_ASSERT_SUCCESS(tc, "set mtime", rv);

    rv = apr_stat(&finfo, NEWFILENAME, APR_FINFO_MTIME, p);
    APR_ASSERT_SUCCESS(tc, "get modified mtime", rv);
    ABTS_TRUE(tc, finfo.mtime == epoch);

    apr_file_close(thefile);
}

abts_suite *testfileinfo(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_info_get, NULL);
    abts_run_test(suite, test_stat, NULL);
    abts_run_test(suite, test_stat_eq_finfo, NULL);
    abts_run_test(suite, test_buffered_write_size, NULL);
    abts_run_test(suite, test_mtime_set, NULL);

    return suite;
}

