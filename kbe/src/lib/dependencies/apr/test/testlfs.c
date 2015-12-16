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
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_poll.h"
#include "apr_strings.h"
#include "apr_lib.h"
#include "apr_mmap.h"
#include "testutil.h"

/* TODO: in 1.3.0 this becomes APR_HAS_SPARSE_FILES, HOWEVER we will
 * still need to test csize before proceeding, because having sparse
 * file support in the OS/APR does not mean this volume supports it!
 */
#if APR_HAS_LARGE_FILES

/* Tests which create an 8GB sparse file and then check it can be used
 * as normal. */

static apr_off_t oneMB = APR_INT64_C(2) << 19;
static apr_off_t eightGB = APR_INT64_C(2) << 32;

static int madefile = 0;

#define PRECOND if (!madefile) { ABTS_NOT_IMPL(tc, "Large file tests not enabled"); return; }

#define TESTDIR "lfstests"
#define TESTFILE "large.bin"
#define TESTFN "lfstests/large.bin"

static void test_open(abts_case *tc, void *data)
{
    apr_file_t *f;
    apr_finfo_t testsize;
    apr_status_t rv;

    rv = apr_dir_make(TESTDIR, APR_OS_DEFAULT, p);
    if (rv && !APR_STATUS_IS_EEXIST(rv)) {
        APR_ASSERT_SUCCESS(tc, "make test directory", rv);
    }

    /* First attempt a 1MB sparse file so we don't tax the poor test box */
    rv = apr_file_open(&f, TESTFN, APR_FOPEN_CREATE | APR_FOPEN_WRITE
                                 | APR_FOPEN_TRUNCATE | APR_FOPEN_SPARSE,
                       APR_OS_DEFAULT, p);

    APR_ASSERT_SUCCESS(tc, "open file", rv);

    APR_ASSERT_SUCCESS(tc, "Truncate to 1MB", rv = apr_file_trunc(f, oneMB+1));

    if (rv == APR_SUCCESS) {
        rv = apr_file_info_get(&testsize, APR_FINFO_CSIZE, f);
    }

    /* give up if we can't determine the allocation size of the file,
     * or if it's not an obviously small allocation but the allocation
     * unit doesn't appear insanely large - on most platforms, it's just
     * zero physical bytes at this point.
     */
    if (rv != APR_SUCCESS || (testsize.csize > oneMB
                              && testsize.csize < oneMB * 2)) {
        ABTS_NOT_IMPL(tc, "Creation of large file (apparently not sparse)");

        madefile = 0;
    } 
    else {
        /* Proceed with our 8GB sparse file now */
        rv = apr_file_trunc(f, eightGB);

        /* 8GB may pass rlimits or filesystem limits */

        if (APR_STATUS_IS_EINVAL(rv)
#ifdef EFBIG
            || rv == EFBIG
#endif
            ) {
            ABTS_NOT_IMPL(tc, "Creation of large file (rlimit, quota or fs)");
        } 
        else {
            APR_ASSERT_SUCCESS(tc, "truncate file to 8gb", rv);
        }
        madefile = rv == APR_SUCCESS;
    }

    APR_ASSERT_SUCCESS(tc, "close large file", apr_file_close(f));

    if (!madefile) {
        APR_ASSERT_SUCCESS(tc, "remove large file", apr_file_remove(TESTFN, p));
    }
}

static void test_reopen(abts_case *tc, void *data)
{
    apr_file_t *fh;
    apr_finfo_t finfo;
    apr_status_t rv;

    PRECOND;
    
    rv = apr_file_open(&fh, TESTFN, APR_FOPEN_SPARSE | APR_FOPEN_READ,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "re-open 8GB file", rv);

    APR_ASSERT_SUCCESS(tc, "file_info_get failed",
                       apr_file_info_get(&finfo, APR_FINFO_NORM, fh));
    
    ABTS_ASSERT(tc, "file_info_get gave incorrect size",
             finfo.size == eightGB);

    APR_ASSERT_SUCCESS(tc, "re-close large file", apr_file_close(fh));
}

static void test_stat(abts_case *tc, void *data)
{
    apr_finfo_t finfo;

    PRECOND;

    APR_ASSERT_SUCCESS(tc, "stat large file", 
                       apr_stat(&finfo, TESTFN, APR_FINFO_NORM, p));
    
    ABTS_ASSERT(tc, "stat gave incorrect size", finfo.size == eightGB);
}

static void test_readdir(abts_case *tc, void *data)
{
    apr_dir_t *dh;
    apr_status_t rv;

    PRECOND;

    APR_ASSERT_SUCCESS(tc, "open test directory", 
                       apr_dir_open(&dh, TESTDIR, p));

    do {
        apr_finfo_t finfo;
        
        rv = apr_dir_read(&finfo, APR_FINFO_MIN, dh);
        
        if (rv == APR_SUCCESS && strcmp(finfo.name, TESTFILE) == 0) {
            ABTS_ASSERT(tc, "apr_dir_read gave incorrect size for large file", 
                     finfo.size == eightGB);
        }

    } while (rv == APR_SUCCESS);
        
    if (!APR_STATUS_IS_ENOENT(rv)) {
        APR_ASSERT_SUCCESS(tc, "apr_dir_read failed", rv);
    }
    
    APR_ASSERT_SUCCESS(tc, "close test directory",
                       apr_dir_close(dh));
}

#define TESTSTR "Hello, world."

static void test_append(abts_case *tc, void *data)
{
    apr_file_t *fh;
    apr_finfo_t finfo;
    apr_status_t rv;
    
    PRECOND;

    rv = apr_file_open(&fh, TESTFN, APR_FOPEN_SPARSE | APR_FOPEN_WRITE 
                                  | APR_FOPEN_APPEND, 
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open 8GB file for append", rv);

    APR_ASSERT_SUCCESS(tc, "append to 8GB file",
                       apr_file_write_full(fh, TESTSTR, strlen(TESTSTR), NULL));

    APR_ASSERT_SUCCESS(tc, "file_info_get failed",
                       apr_file_info_get(&finfo, APR_FINFO_NORM, fh));
    
    ABTS_ASSERT(tc, "file_info_get gave incorrect size",
             finfo.size == eightGB + strlen(TESTSTR));

    APR_ASSERT_SUCCESS(tc, "close 8GB file", apr_file_close(fh));
}

static void test_seek(abts_case *tc, void *data)
{
    apr_file_t *fh;
    apr_off_t pos;
    apr_status_t rv;

    PRECOND;
    
    rv = apr_file_open(&fh, TESTFN, APR_FOPEN_SPARSE | APR_FOPEN_WRITE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open 8GB file for writing", rv);

    pos = 0;
    APR_ASSERT_SUCCESS(tc, "relative seek to end", 
                       apr_file_seek(fh, APR_END, &pos));
    ABTS_ASSERT(tc, "seek to END gave 8GB", pos == eightGB);
    
    pos = eightGB;
    APR_ASSERT_SUCCESS(tc, "seek to 8GB", apr_file_seek(fh, APR_SET, &pos));
    ABTS_ASSERT(tc, "seek gave 8GB offset", pos == eightGB);

    pos = 0;
    APR_ASSERT_SUCCESS(tc, "relative seek to 0", apr_file_seek(fh, APR_CUR, &pos));
    ABTS_ASSERT(tc, "relative seek gave 8GB offset", pos == eightGB);

    apr_file_close(fh);
}

static void test_write(abts_case *tc, void *data)
{
    apr_file_t *fh;
    apr_off_t pos = eightGB - 4;
    apr_status_t rv;

    PRECOND;

    rv = apr_file_open(&fh, TESTFN, APR_FOPEN_SPARSE | APR_FOPEN_WRITE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "re-open 8GB file", rv);

    APR_ASSERT_SUCCESS(tc, "seek to 8GB - 4", 
                       apr_file_seek(fh, APR_SET, &pos));
    ABTS_ASSERT(tc, "seek gave 8GB-4 offset", pos == eightGB - 4);

    APR_ASSERT_SUCCESS(tc, "write magic string to 8GB-4",
                       apr_file_write_full(fh, "FISH", 4, NULL));

    APR_ASSERT_SUCCESS(tc, "close 8GB file", apr_file_close(fh));
}


#if APR_HAS_MMAP
static void test_mmap(abts_case *tc, void *data)
{
    apr_mmap_t *map;
    apr_file_t *fh;
    apr_size_t len = 65536; /* hopefully a multiple of the page size */
    apr_off_t off = eightGB - len; 
    apr_status_t rv;
    void *ptr;

    PRECOND;

    rv = apr_file_open(&fh, TESTFN, APR_FOPEN_SPARSE | APR_FOPEN_READ,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open 8gb file for mmap", rv);
    
    APR_ASSERT_SUCCESS(tc, "mmap 8GB file",
                       apr_mmap_create(&map, fh, off, len, APR_MMAP_READ, p));

    APR_ASSERT_SUCCESS(tc, "close file", apr_file_close(fh));

    ABTS_ASSERT(tc, "mapped a 64K block", map->size == len);
    
    APR_ASSERT_SUCCESS(tc, "get pointer into mmaped region",
                       apr_mmap_offset(&ptr, map, len - 4));
    ABTS_ASSERT(tc, "pointer was not NULL", ptr != NULL);

    ABTS_ASSERT(tc, "found the magic string", memcmp(ptr, "FISH", 4) == 0);

    APR_ASSERT_SUCCESS(tc, "delete mmap handle", apr_mmap_delete(map));
}
#endif /* APR_HAS_MMAP */

static void test_format(abts_case *tc, void *data)
{
    apr_off_t off;

    PRECOND;

    off = apr_atoi64(apr_off_t_toa(p, eightGB));

    ABTS_ASSERT(tc, "apr_atoi64 parsed apr_off_t_toa result incorrectly",
             off == eightGB);
}

#define TESTBUFFN TESTDIR "/buffer.bin"

static void test_buffered(abts_case *tc, void *data)
{
    apr_off_t off;
    apr_file_t *f;
    apr_status_t rv;

    PRECOND;

    rv = apr_file_open(&f, TESTBUFFN, APR_FOPEN_CREATE | APR_FOPEN_WRITE
                                    | APR_FOPEN_TRUNCATE | APR_FOPEN_BUFFERED
                                    | APR_FOPEN_SPARSE,
                       APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "open buffered file", rv);

    APR_ASSERT_SUCCESS(tc, "truncate to 8GB",
                       apr_file_trunc(f, eightGB));

    off = eightGB;
    APR_ASSERT_SUCCESS(tc, "seek to 8GB",
                       apr_file_seek(f, APR_SET, &off));
    ABTS_ASSERT(tc, "returned seek position still 8GB",
                off == eightGB);

    off = 0;
    APR_ASSERT_SUCCESS(tc, "relative seek",
                       apr_file_seek(f, APR_CUR, &off));
    ABTS_ASSERT(tc, "relative seek still at 8GB",
                off == eightGB);

    off = 0;
    APR_ASSERT_SUCCESS(tc, "end-relative seek",
                       apr_file_seek(f, APR_END, &off));
    ABTS_ASSERT(tc, "end-relative seek still at 8GB",
                off == eightGB);

    off = -eightGB;
    APR_ASSERT_SUCCESS(tc, "relative seek to beginning",
                       apr_file_seek(f, APR_CUR, &off));
    ABTS_ASSERT(tc, "seek to beginning got zero",
                off == 0);

    APR_ASSERT_SUCCESS(tc, "close buffered file",
                       apr_file_close(f));
}

#else /* !APR_HAS_LARGE_FILES */

static void test_nolfs(abts_case *tc, void *data)
{
    ABTS_NOT_IMPL(tc, "Large Files not supported");
}

#endif

abts_suite *testlfs(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

#if APR_HAS_LARGE_FILES
    abts_run_test(suite, test_open, NULL);
    abts_run_test(suite, test_reopen, NULL);
    abts_run_test(suite, test_stat, NULL);
    abts_run_test(suite, test_readdir, NULL);
    abts_run_test(suite, test_seek, NULL);
    abts_run_test(suite, test_append, NULL);
    abts_run_test(suite, test_write, NULL);
#if APR_HAS_MMAP
    abts_run_test(suite, test_mmap, NULL);
#endif
    abts_run_test(suite, test_format, NULL);
    abts_run_test(suite, test_buffered, NULL);
#else
    abts_run_test(suite, test_nolfs, NULL);
#endif

    return suite;
}

