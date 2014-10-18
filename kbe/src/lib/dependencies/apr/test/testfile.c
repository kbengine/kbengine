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
#include "apr_network_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_poll.h"
#include "apr_lib.h"
#include "testutil.h"

#define DIRNAME "data"
#define FILENAME DIRNAME "/file_datafile.txt"
#define TESTSTR  "This is the file data file."

#define TESTREAD_BLKSIZE 1024
#define APR_BUFFERSIZE   4096 /* This should match APR's buffer size. */



static void test_open_noreadwrite(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *thefile = NULL;

    rv = apr_file_open(&thefile, FILENAME,
                       APR_FOPEN_CREATE | APR_FOPEN_EXCL,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_TRUE(tc, rv != APR_SUCCESS);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EACCES(rv));
    ABTS_PTR_EQUAL(tc, NULL, thefile); 
}

static void test_open_excl(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *thefile = NULL;

    rv = apr_file_open(&thefile, FILENAME,
                       APR_FOPEN_CREATE | APR_FOPEN_EXCL | APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_TRUE(tc, rv != APR_SUCCESS);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EEXIST(rv));
    ABTS_PTR_EQUAL(tc, NULL, thefile); 
}

static void test_open_read(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, filetest);
    apr_file_close(filetest);
}

static void link_existing(abts_case *tc, void *data)
{
    apr_status_t rv;
    
    rv = apr_file_link("data/file_datafile.txt", "data/file_datafile2.txt");
    apr_file_remove("data/file_datafile2.txt", p);
    ABTS_ASSERT(tc, "Couldn't create hardlink to file", rv == APR_SUCCESS);
}

static void link_nonexisting(abts_case *tc, void *data)
{
    apr_status_t rv;
    
    rv = apr_file_link("data/does_not_exist.txt", "data/fake.txt");
    ABTS_ASSERT(tc, "", rv != APR_SUCCESS);
}

static void test_read(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t nbytes = 256;
    char *str = apr_pcalloc(p, nbytes + 1);
    apr_file_t *filetest = NULL;
    
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);

    APR_ASSERT_SUCCESS(tc, "Opening test file " FILENAME, rv);
    rv = apr_file_read(filetest, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(TESTSTR), nbytes);
    ABTS_STR_EQUAL(tc, TESTSTR, str);

    apr_file_close(filetest);
}

static void test_readzero(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t nbytes = 0;
    char *str = NULL;
    apr_file_t *filetest;
    
    rv = apr_file_open(&filetest, FILENAME, APR_FOPEN_READ, APR_OS_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "Opening test file " FILENAME, rv);

    rv = apr_file_read(filetest, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, 0, nbytes);

    apr_file_close(filetest);
}

static void test_filename(abts_case *tc, void *data)
{
    const char *str;
    apr_status_t rv;
    apr_file_t *filetest = NULL;
    
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    APR_ASSERT_SUCCESS(tc, "Opening test file " FILENAME, rv);

    rv = apr_file_name_get(&str, filetest);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, FILENAME, str);

    apr_file_close(filetest);
}
    
static void test_fileclose(abts_case *tc, void *data)
{
    char str;
    apr_status_t rv;
    apr_size_t one = 1;
    apr_file_t *filetest = NULL;
    
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    APR_ASSERT_SUCCESS(tc, "Opening test file " FILENAME, rv);

    rv = apr_file_close(filetest);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    /* We just closed the file, so this should fail */
    rv = apr_file_read(filetest, &str, &one);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EBADF(rv));
}

static void test_file_remove(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    rv = apr_file_remove(FILENAME, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_open(&filetest, FILENAME, APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ENOENT(rv));
}

static void test_open_write(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    filetest = NULL;
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ENOENT(rv));
    ABTS_PTR_EQUAL(tc, NULL, filetest);
}

static void test_open_writecreate(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    filetest = NULL;
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE | APR_FOPEN_CREATE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_close(filetest);
}

static void test_write(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t bytes = strlen(TESTSTR);
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE | APR_FOPEN_CREATE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_write(filetest, TESTSTR, &bytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_close(filetest);
}

static void test_open_readwrite(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    filetest = NULL;
    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ | APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_NOTNULL(tc, filetest);

    apr_file_close(filetest);
}

static void test_seek(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_off_t offset = 5;
    apr_size_t nbytes = 256;
    char *str = apr_pcalloc(p, nbytes + 1);
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_READ,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    APR_ASSERT_SUCCESS(tc, "Open test file " FILENAME, rv);

    rv = apr_file_read(filetest, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(TESTSTR), nbytes);
    ABTS_STR_EQUAL(tc, TESTSTR, str);

    memset(str, 0, nbytes + 1);

    rv = apr_file_seek(filetest, SEEK_SET, &offset);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    rv = apr_file_read(filetest, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(TESTSTR) - 5, nbytes);
    ABTS_STR_EQUAL(tc, TESTSTR + 5, str);

    apr_file_close(filetest);

    /* Test for regression of sign error bug with SEEK_END and
       buffered files. */
    rv = apr_file_open(&filetest, FILENAME,
                       APR_FOPEN_READ | APR_FOPEN_BUFFERED,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    APR_ASSERT_SUCCESS(tc, "Open test file " FILENAME, rv);

    offset = -5;
    rv = apr_file_seek(filetest, SEEK_END, &offset);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(TESTSTR) - 5, nbytes);

    memset(str, 0, nbytes + 1);
    nbytes = 256;
    rv = apr_file_read(filetest, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, 5, nbytes);
    ABTS_STR_EQUAL(tc, TESTSTR + strlen(TESTSTR) - 5, str);

    apr_file_close(filetest);
}                

static void test_userdata_set(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_data_set(filetest, "This is a test",
                           "test", apr_pool_cleanup_null);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    apr_file_close(filetest);
}

static void test_userdata_get(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *udata;
    char *teststr;
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_data_set(filetest, "This is a test",
                           "test", apr_pool_cleanup_null);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_data_get(&udata, "test", filetest);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    teststr = udata;
    ABTS_STR_EQUAL(tc, "This is a test", teststr);

    apr_file_close(filetest);
}

static void test_userdata_getnokey(abts_case *tc, void *data)
{
    apr_status_t rv;
    void *teststr;
    apr_file_t *filetest = NULL;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_data_get(&teststr, "nokey", filetest);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_PTR_EQUAL(tc, NULL, teststr);
    apr_file_close(filetest);
}

static void test_buffer_set_get(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_size_t bufsize;
    apr_file_t *filetest = NULL;
    char   * buffer;

    rv = apr_file_open(&filetest, FILENAME, 
                       APR_FOPEN_WRITE | APR_FOPEN_BUFFERED,
                       APR_UREAD | APR_UWRITE | APR_GREAD, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    bufsize = apr_file_buffer_size_get(filetest);
    ABTS_SIZE_EQUAL(tc, APR_BUFFERSIZE, bufsize);
 
    buffer = apr_pcalloc(p, 10240);
    rv = apr_file_buffer_set(filetest, buffer, 10240);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    bufsize = apr_file_buffer_size_get(filetest);
    ABTS_SIZE_EQUAL(tc, 10240, bufsize);
    
    rv = apr_file_buffer_set(filetest, buffer, 12);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    bufsize = apr_file_buffer_size_get(filetest);
    ABTS_SIZE_EQUAL(tc, 12, bufsize);
    
    apr_file_close(filetest);
}
static void test_getc(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char ch;

    rv = apr_file_open(&f, FILENAME, APR_FOPEN_READ, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_getc(&ch, f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, (int)TESTSTR[0], (int)ch);
    apr_file_close(f);
}

static void test_ungetc(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char ch;

    rv = apr_file_open(&f, FILENAME, APR_FOPEN_READ, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_getc(&ch, f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, (int)TESTSTR[0], (int)ch);

    apr_file_ungetc('X', f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    apr_file_getc(&ch, f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_INT_EQUAL(tc, 'X', (int)ch);

    apr_file_close(f);
}

static void test_gets(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char *str = apr_palloc(p, 256);

    rv = apr_file_open(&f, FILENAME, APR_FOPEN_READ, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_gets(str, 256, f);
    /* Only one line in the test file, so APR will encounter EOF on the first
     * call to gets, but we should get APR_SUCCESS on this call and
     * APR_EOF on the next.
     */
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, TESTSTR, str);
    rv = apr_file_gets(str, 256, f);
    ABTS_INT_EQUAL(tc, APR_EOF, rv);
    ABTS_STR_EQUAL(tc, "", str);
    apr_file_close(f);
}

static void test_gets_buffered(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char *str = apr_palloc(p, 256);

    /* This will deadlock gets before the r524355 fix. */
    rv = apr_file_open(&f, FILENAME, APR_FOPEN_READ|APR_FOPEN_BUFFERED|APR_FOPEN_XTHREAD, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_gets(str, 256, f);
    /* Only one line in the test file, so APR will encounter EOF on the first
     * call to gets, but we should get APR_SUCCESS on this call and
     * APR_EOF on the next.
     */
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, TESTSTR, str);
    rv = apr_file_gets(str, 256, f);
    ABTS_INT_EQUAL(tc, APR_EOF, rv);
    ABTS_STR_EQUAL(tc, "", str);
    apr_file_close(f);
}

static void test_bigread(abts_case *tc, void *data)
{
    apr_file_t *f = NULL;
    apr_status_t rv;
    char buf[APR_BUFFERSIZE * 2];
    apr_size_t nbytes;

    /* Create a test file with known content.
     */
    rv = apr_file_open(&f, "data/created_file", 
                       APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE,
                       APR_UREAD | APR_UWRITE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    nbytes = APR_BUFFERSIZE;
    memset(buf, 0xFE, nbytes);

    rv = apr_file_write(f, buf, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, APR_BUFFERSIZE, nbytes);

    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    f = NULL;
    rv = apr_file_open(&f, "data/created_file", APR_FOPEN_READ, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    nbytes = sizeof buf;
    rv = apr_file_read(f, buf, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, APR_BUFFERSIZE, nbytes);

    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_remove("data/created_file", p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

/* This is a horrible name for this function.  We are testing APR, not how
 * Apache uses APR.  And, this function tests _way_ too much stuff.
 */
static void test_mod_neg(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *f;
    const char *s;
    int i;
    apr_size_t nbytes;
    char buf[8192];
    apr_off_t cur;
    const char *fname = "data/modneg.dat";

    rv = apr_file_open(&f, fname, 
                       APR_FOPEN_CREATE | APR_FOPEN_WRITE, APR_UREAD | APR_UWRITE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    s = "body56789\n";
    nbytes = strlen(s);
    rv = apr_file_write(f, s, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(s), nbytes);
    
    for (i = 0; i < 7980; i++) {
        s = "0";
        nbytes = strlen(s);
        rv = apr_file_write(f, s, &nbytes);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
        ABTS_SIZE_EQUAL(tc, strlen(s), nbytes);
    }
    
    s = "end456789\n";
    nbytes = strlen(s);
    rv = apr_file_write(f, s, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(s), nbytes);

    for (i = 0; i < 10000; i++) {
        s = "1";
        nbytes = strlen(s);
        rv = apr_file_write(f, s, &nbytes);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
        ABTS_SIZE_EQUAL(tc, strlen(s), nbytes);
    }
    
    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_open(&f, fname, APR_FOPEN_READ, 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_gets(buf, 11, f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "body56789\n", buf);

    cur = 0;
    rv = apr_file_seek(f, APR_CUR, &cur);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "File Pointer Mismatch, expected 10", cur == 10);

    nbytes = sizeof(buf);
    rv = apr_file_read(f, buf, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, nbytes, sizeof(buf));

    cur = -((apr_off_t)nbytes - 7980);
    rv = apr_file_seek(f, APR_CUR, &cur);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "File Pointer Mismatch, expected 7990", cur == 7990);

    rv = apr_file_gets(buf, 11, f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "end456789\n", buf);

    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_remove(fname, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

/* Test that the contents of file FNAME are equal to data EXPECT of
 * length EXPECTLEN. */
static void file_contents_equal(abts_case *tc,
                                const char *fname,
                                const void *expect,
                                apr_size_t expectlen)
{
    void *actual = apr_palloc(p, expectlen);
    apr_file_t *f;

    APR_ASSERT_SUCCESS(tc, "open file",
                       apr_file_open(&f, fname, APR_FOPEN_READ|APR_FOPEN_BUFFERED,
                                     0, p));
    APR_ASSERT_SUCCESS(tc, "read from file",
                       apr_file_read_full(f, actual, expectlen, NULL));
    
    ABTS_ASSERT(tc, "matched expected file contents",
                memcmp(expect, actual, expectlen) == 0);

    APR_ASSERT_SUCCESS(tc, "close file", apr_file_close(f));
}

#define LINE1 "this is a line of text\n"
#define LINE2 "this is a second line of text\n"

static void test_puts(abts_case *tc, void *data)
{
    apr_file_t *f;
    const char *fname = "data/testputs.txt";

    APR_ASSERT_SUCCESS(tc, "open file for writing",
                       apr_file_open(&f, fname, 
                                     APR_FOPEN_WRITE|APR_FOPEN_CREATE|APR_FOPEN_TRUNCATE,
                                     APR_OS_DEFAULT, p));
    
    APR_ASSERT_SUCCESS(tc, "write line to file", 
                       apr_file_puts(LINE1, f));
    APR_ASSERT_SUCCESS(tc, "write second line to file", 
                       apr_file_puts(LINE2, f));
    
    APR_ASSERT_SUCCESS(tc, "close for writing",
                       apr_file_close(f));

    file_contents_equal(tc, fname, LINE1 LINE2, strlen(LINE1 LINE2));
}

static void test_writev(abts_case *tc, void *data)
{
    apr_file_t *f;
    apr_size_t nbytes;
    struct iovec vec[5];
    const char *fname = "data/testwritev.txt";

    APR_ASSERT_SUCCESS(tc, "open file for writing",
                       apr_file_open(&f, fname, 
                                     APR_FOPEN_WRITE|APR_FOPEN_CREATE|APR_FOPEN_TRUNCATE,
                                     APR_OS_DEFAULT, p));
    
    vec[0].iov_base = LINE1;
    vec[0].iov_len = strlen(LINE1);

    APR_ASSERT_SUCCESS(tc, "writev of size 1 to file",
                       apr_file_writev(f, vec, 1, &nbytes));

    file_contents_equal(tc, fname, LINE1, strlen(LINE1));
    
    vec[0].iov_base = LINE1;
    vec[0].iov_len = strlen(LINE1);
    vec[1].iov_base = LINE2;
    vec[1].iov_len = strlen(LINE2);
    vec[2].iov_base = LINE1;
    vec[2].iov_len = strlen(LINE1);
    vec[3].iov_base = LINE1;
    vec[3].iov_len = strlen(LINE1);
    vec[4].iov_base = LINE2;
    vec[4].iov_len = strlen(LINE2);

    APR_ASSERT_SUCCESS(tc, "writev of size 5 to file",
                       apr_file_writev(f, vec, 5, &nbytes));

    APR_ASSERT_SUCCESS(tc, "close for writing",
                       apr_file_close(f));

    file_contents_equal(tc, fname, LINE1 LINE1 LINE2 LINE1 LINE1 LINE2, 
                        strlen(LINE1)*4 + strlen(LINE2)*2);

}

static void test_writev_full(abts_case *tc, void *data)
{
    apr_file_t *f;
    apr_size_t nbytes;
    struct iovec vec[5];
    const char *fname = "data/testwritev_full.txt";

    APR_ASSERT_SUCCESS(tc, "open file for writing",
                       apr_file_open(&f, fname, 
                                     APR_FOPEN_WRITE|APR_FOPEN_CREATE|APR_FOPEN_TRUNCATE,
                                     APR_OS_DEFAULT, p));
    
    vec[0].iov_base = LINE1;
    vec[0].iov_len = strlen(LINE1);
    vec[1].iov_base = LINE2;
    vec[1].iov_len = strlen(LINE2);
    vec[2].iov_base = LINE1;
    vec[2].iov_len = strlen(LINE1);
    vec[3].iov_base = LINE1;
    vec[3].iov_len = strlen(LINE1);
    vec[4].iov_base = LINE2;
    vec[4].iov_len = strlen(LINE2);

    APR_ASSERT_SUCCESS(tc, "writev_full of size 5 to file",
                       apr_file_writev_full(f, vec, 5, &nbytes));

    ABTS_SIZE_EQUAL(tc, strlen(LINE1)*3 + strlen(LINE2)*2, nbytes);

    APR_ASSERT_SUCCESS(tc, "close for writing",
                       apr_file_close(f));

    file_contents_equal(tc, fname, LINE1 LINE2 LINE1 LINE1 LINE2, 
                        strlen(LINE1)*3 + strlen(LINE2)*2);

}

static void test_writev_buffered(abts_case *tc, void *data)
{
    apr_file_t *f;
    apr_size_t nbytes;
    struct iovec vec[2];
    const char *fname = "data/testwritev_buffered.dat";

    APR_ASSERT_SUCCESS(tc, "open file for writing",
                       apr_file_open(&f, fname,
                                     APR_FOPEN_WRITE | APR_FOPEN_CREATE | APR_FOPEN_TRUNCATE |
                                     APR_FOPEN_BUFFERED, APR_OS_DEFAULT, p));

    nbytes = strlen(TESTSTR);
    APR_ASSERT_SUCCESS(tc, "buffered write",
                       apr_file_write(f, TESTSTR, &nbytes));

    vec[0].iov_base = LINE1;
    vec[0].iov_len = strlen(LINE1);
    vec[1].iov_base = LINE2;
    vec[1].iov_len = strlen(LINE2);

    APR_ASSERT_SUCCESS(tc, "writev of size 2 to file",
                       apr_file_writev(f, vec, 2, &nbytes));

    APR_ASSERT_SUCCESS(tc, "close for writing",
                       apr_file_close(f));

    file_contents_equal(tc, fname, TESTSTR LINE1 LINE2,
                        strlen(TESTSTR) + strlen(LINE1) + strlen(LINE2));
}

static void test_writev_buffered_seek(abts_case *tc, void *data)
{
    apr_file_t *f;
    apr_status_t rv;
    apr_off_t off = 0;
    struct iovec vec[3];
    apr_size_t nbytes = strlen(TESTSTR);
    char *str = apr_pcalloc(p, nbytes+1);
    const char *fname = "data/testwritev_buffered.dat";

    APR_ASSERT_SUCCESS(tc, "open file for writing",
                       apr_file_open(&f, fname,
                                     APR_FOPEN_WRITE | APR_FOPEN_READ | APR_FOPEN_BUFFERED,
                                     APR_OS_DEFAULT, p));

    rv = apr_file_read(f, str, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, TESTSTR, str);
    APR_ASSERT_SUCCESS(tc, "buffered seek", apr_file_seek(f, APR_SET, &off));

    vec[0].iov_base = LINE1;
    vec[0].iov_len = strlen(LINE1);
    vec[1].iov_base = LINE2;
    vec[1].iov_len = strlen(LINE2);
    vec[2].iov_base = TESTSTR;
    vec[2].iov_len = strlen(TESTSTR);

    APR_ASSERT_SUCCESS(tc, "writev of size 2 to file",
                       apr_file_writev(f, vec, 3, &nbytes));

    APR_ASSERT_SUCCESS(tc, "close for writing",
                       apr_file_close(f));

    file_contents_equal(tc, fname, LINE1 LINE2 TESTSTR,
                        strlen(LINE1) + strlen(LINE2) + strlen(TESTSTR));

    APR_ASSERT_SUCCESS(tc, "remove file", apr_file_remove(fname, p));
}

static void test_truncate(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_file_t *f;
    const char *fname = "data/testtruncate.dat";
    const char *s;
    apr_size_t nbytes;
    apr_finfo_t finfo;

    apr_file_remove(fname, p);

    rv = apr_file_open(&f, fname,
                       APR_FOPEN_CREATE | APR_FOPEN_WRITE, APR_UREAD | APR_UWRITE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    
    s = "some data";
    nbytes = strlen(s);
    rv = apr_file_write(f, s, &nbytes);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_SIZE_EQUAL(tc, strlen(s), nbytes);

    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_open(&f, fname,
                       APR_FOPEN_TRUNCATE | APR_FOPEN_WRITE, APR_UREAD | APR_UWRITE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_file_close(f);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);

    rv = apr_stat(&finfo, fname, APR_FINFO_SIZE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_ASSERT(tc, "File size mismatch, expected 0 (empty)", finfo.size == 0);

    rv = apr_file_remove(fname, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}

static void test_bigfprintf(abts_case *tc, void *data)
{
    apr_file_t *f;
    const char *fname = "data/testbigfprintf.dat";
    char *to_write;
    int i;

    apr_file_remove(fname, p);

    APR_ASSERT_SUCCESS(tc, "open test file",
                       apr_file_open(&f, fname,
                                     APR_FOPEN_CREATE|APR_FOPEN_WRITE,
                                     APR_UREAD|APR_UWRITE, p));
    

    to_write = malloc(HUGE_STRING_LEN + 3);

    for (i = 0; i < HUGE_STRING_LEN + 1; ++i)
        to_write[i] = 'A' + i%26;

    strcpy(to_write + HUGE_STRING_LEN, "42");

    i = apr_file_printf(f, "%s", to_write);
    ABTS_INT_EQUAL(tc, HUGE_STRING_LEN + 2, i);

    apr_file_close(f);

    file_contents_equal(tc, fname, to_write, HUGE_STRING_LEN + 2);

    free(to_write);
}

static void test_fail_write_flush(abts_case *tc, void *data)
{
    apr_file_t *f;
    const char *fname = "data/testflush.dat";
    apr_status_t rv;
    char buf[APR_BUFFERSIZE];
    int n;

    apr_file_remove(fname, p);

    APR_ASSERT_SUCCESS(tc, "open test file",
                       apr_file_open(&f, fname,
                                     APR_FOPEN_CREATE|APR_FOPEN_READ|APR_FOPEN_BUFFERED,
                                     APR_UREAD|APR_UWRITE, p));

    memset(buf, 'A', sizeof buf);

    /* Try three writes.  One of these should fail when it exceeds the
     * internal buffer and actually tries to write to the file, which
     * was opened read-only and hence should be unwritable. */
    for (n = 0, rv = APR_SUCCESS; n < 4 && rv == APR_SUCCESS; n++) {
        apr_size_t bytes = sizeof buf;
        rv = apr_file_write(f, buf, &bytes);
    }

    ABTS_ASSERT(tc, "failed to write to read-only buffered fd",
                rv != APR_SUCCESS);

    apr_file_close(f);
}

static void test_fail_read_flush(abts_case *tc, void *data)
{
    apr_file_t *f;
    const char *fname = "data/testflush.dat";
    apr_status_t rv;
    char buf[2];

    apr_file_remove(fname, p);

    APR_ASSERT_SUCCESS(tc, "open test file",
                       apr_file_open(&f, fname,
                                     APR_FOPEN_CREATE|APR_FOPEN_READ|APR_FOPEN_BUFFERED,
                                     APR_UREAD|APR_UWRITE, p));

    /* this write should be buffered. */
    APR_ASSERT_SUCCESS(tc, "buffered write should succeed",
                       apr_file_puts("hello", f));

    /* Now, trying a read should fail since the write must be flushed,
     * and should fail with something other than EOF since the file is
     * opened read-only. */
    rv = apr_file_read_full(f, buf, 2, NULL);

    ABTS_ASSERT(tc, "read should flush buffered write and fail",
                rv != APR_SUCCESS && rv != APR_EOF);

    /* Likewise for gets */
    rv = apr_file_gets(buf, 2, f);

    ABTS_ASSERT(tc, "gets should flush buffered write and fail",
                rv != APR_SUCCESS && rv != APR_EOF);

    /* Likewise for seek. */
    {
        apr_off_t offset = 0;

        rv = apr_file_seek(f, APR_SET, &offset);
    }

    ABTS_ASSERT(tc, "seek should flush buffered write and fail",
                rv != APR_SUCCESS && rv != APR_EOF);

    apr_file_close(f);
}

static void test_xthread(abts_case *tc, void *data)
{
    apr_file_t *f;
    const char *fname = "data/testxthread.dat";
    apr_status_t rv;
    apr_int32_t flags = APR_FOPEN_CREATE|APR_FOPEN_READ|APR_FOPEN_WRITE|APR_FOPEN_APPEND|APR_FOPEN_XTHREAD;
    char buf[128] = { 0 };

    /* Test for bug 38438, opening file with append + xthread and seeking to 
       the end of the file resulted in writes going to the beginning not the
       end. */

    apr_file_remove(fname, p);

    APR_ASSERT_SUCCESS(tc, "open test file",
                       apr_file_open(&f, fname, flags,
                                     APR_UREAD|APR_UWRITE, p));

    APR_ASSERT_SUCCESS(tc, "write should succeed",
                       apr_file_puts("hello", f));

    apr_file_close(f);
    
    APR_ASSERT_SUCCESS(tc, "open test file",
                       apr_file_open(&f, fname, flags,
                                     APR_UREAD|APR_UWRITE, p));

    /* Seek to the end. */
    {
        apr_off_t offset = 0;

        rv = apr_file_seek(f, APR_END, &offset);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    }

    APR_ASSERT_SUCCESS(tc, "more writes should succeed",
                       apr_file_puts("world", f));

    /* Back to the beginning. */
    {
        apr_off_t offset = 0;
        
        rv = apr_file_seek(f, APR_SET, &offset);
        ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    }
    
    apr_file_read_full(f, buf, sizeof(buf), NULL);

    ABTS_STR_EQUAL(tc, "helloworld", buf);

    apr_file_close(f);
}

abts_suite *testfile(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_open_noreadwrite, NULL);
    abts_run_test(suite, test_open_excl, NULL);
    abts_run_test(suite, test_open_read, NULL);
    abts_run_test(suite, test_open_readwrite, NULL);
    abts_run_test(suite, link_existing, NULL);
    abts_run_test(suite, link_nonexisting, NULL);
    abts_run_test(suite, test_read, NULL); 
    abts_run_test(suite, test_readzero, NULL); 
    abts_run_test(suite, test_seek, NULL);
    abts_run_test(suite, test_filename, NULL);
    abts_run_test(suite, test_fileclose, NULL);
    abts_run_test(suite, test_file_remove, NULL);
    abts_run_test(suite, test_open_write, NULL);
    abts_run_test(suite, test_open_writecreate, NULL);
    abts_run_test(suite, test_write, NULL);
    abts_run_test(suite, test_userdata_set, NULL);
    abts_run_test(suite, test_userdata_get, NULL);
    abts_run_test(suite, test_userdata_getnokey, NULL);
    abts_run_test(suite, test_getc, NULL);
    abts_run_test(suite, test_ungetc, NULL);
    abts_run_test(suite, test_gets, NULL);
    abts_run_test(suite, test_gets_buffered, NULL);
    abts_run_test(suite, test_puts, NULL);
    abts_run_test(suite, test_writev, NULL);
    abts_run_test(suite, test_writev_full, NULL);
    abts_run_test(suite, test_writev_buffered, NULL);
    abts_run_test(suite, test_writev_buffered_seek, NULL);
    abts_run_test(suite, test_bigread, NULL);
    abts_run_test(suite, test_mod_neg, NULL);
    abts_run_test(suite, test_truncate, NULL);
    abts_run_test(suite, test_bigfprintf, NULL);
    abts_run_test(suite, test_fail_write_flush, NULL);
    abts_run_test(suite, test_fail_read_flush, NULL);
    abts_run_test(suite, test_buffer_set_get, NULL);
    abts_run_test(suite, test_xthread, NULL);

    return suite;
}

