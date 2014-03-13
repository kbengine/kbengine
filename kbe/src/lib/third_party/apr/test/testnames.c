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
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_lib.h"
#include "apr_strings.h"

#if defined(WIN32)
#include <direct.h>
#endif

#if defined(WIN32) || defined(OS2)
#define ABS_ROOT "C:/"
#elif defined(NETWARE)
#define ABS_ROOT "SYS:/"
#else
#define ABS_ROOT "/"
#endif

static void merge_aboveroot(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;
    char errmsg[256];

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo", ABS_ROOT"bar", APR_FILEPATH_NOTABOVEROOT,
                            p);
    apr_strerror(rv, errmsg, sizeof(errmsg));
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EABOVEROOT(rv));
    ABTS_PTR_EQUAL(tc, NULL, dstpath);
    ABTS_STR_EQUAL(tc, "The given path was above the root path", errmsg);
}

static void merge_belowroot(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo", ABS_ROOT"foo/bar", 
                            APR_FILEPATH_NOTABOVEROOT, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT"foo/bar", dstpath);
}

static void merge_noflag(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo", ABS_ROOT"foo/bar", 0, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT"foo/bar", dstpath);
}

static void merge_dotdot(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo/bar", "../baz", 0, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT"foo/baz", dstpath);

    rv = apr_filepath_merge(&dstpath, "", "../test", 0, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "../test", dstpath);

    /* Very dangerous assumptions here about what the cwd is.  However, let's assume
     * that the testall is invoked from within apr/test/ so the following test should
     * return ../test unless a previously fixed bug remains or the developer changes
     * the case of the test directory:
     */
    rv = apr_filepath_merge(&dstpath, "", "../test", APR_FILEPATH_TRUENAME, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "../test", dstpath);
}

static void merge_dotdot_dotdot_dotdot(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, "", 
                            "../../..", APR_FILEPATH_TRUENAME, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "../../..", dstpath);

    rv = apr_filepath_merge(&dstpath, "", 
                            "../../../", APR_FILEPATH_TRUENAME, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "../../../", dstpath);
}

static void merge_secure(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo/bar", "../bar/baz", 0, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT"foo/bar/baz", dstpath);
}

static void merge_notrel(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo/bar", "../baz",
                            APR_FILEPATH_NOTRELATIVE, p);
    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT"foo/baz", dstpath);
}

static void merge_notrelfail(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;
    char errmsg[256];

    rv = apr_filepath_merge(&dstpath, "foo/bar", "../baz", 
                            APR_FILEPATH_NOTRELATIVE, p);
    apr_strerror(rv, errmsg, sizeof(errmsg));

    ABTS_PTR_EQUAL(tc, NULL, dstpath);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ERELATIVE(rv));
    ABTS_STR_EQUAL(tc, "The given path is relative", errmsg);
}

static void merge_notabsfail(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;
    char errmsg[256];

    rv = apr_filepath_merge(&dstpath, ABS_ROOT"foo/bar", "../baz", 
                            APR_FILEPATH_NOTABSOLUTE, p);
    apr_strerror(rv, errmsg, sizeof(errmsg));

    ABTS_PTR_EQUAL(tc, NULL, dstpath);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EABSOLUTE(rv));
    ABTS_STR_EQUAL(tc, "The given path is absolute", errmsg);
}

static void merge_notabs(abts_case *tc, void *data)
{
    apr_status_t rv;
    char *dstpath = NULL;

    rv = apr_filepath_merge(&dstpath, "foo/bar", "../baz", 
                            APR_FILEPATH_NOTABSOLUTE, p);

    ABTS_PTR_NOTNULL(tc, dstpath);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "foo/baz", dstpath);
}

#if defined (WIN32)
static void merge_lowercasedrive(abts_case *tc, void *data)
{
  char current_dir[1024];
  char current_dir_on_C[1024];
  char *dir_on_c;
  char *testdir;
  apr_status_t rv;

  /* Change the current directory on C: from something like "C:\dir"
     to something like "c:\dir" to replicate the failing case. */
  ABTS_PTR_NOTNULL(tc, _getcwd(current_dir, sizeof(current_dir)));

   /* 3 stands for drive C: */
  ABTS_PTR_NOTNULL(tc, _getdcwd(3, current_dir_on_C,
	                            sizeof(current_dir_on_C)));

  /* Use the same path, but now with a lower case driveletter */
  dir_on_c = apr_pstrdup(p, current_dir_on_C);
  dir_on_c[0] = (char)tolower(dir_on_c[0]);

  chdir(dir_on_c);

  /* Now merge a drive relative path with an upper case drive letter. */
  rv = apr_filepath_merge(&testdir, NULL, "C:hi",
                          APR_FILEPATH_NOTRELATIVE, p);

  /* Change back to original directory for next tests */
  chdir("C:\\"); /* Switch to upper case */
  chdir(current_dir_on_C); /* Switch cwd on C: */
  chdir(current_dir); /* Switch back to original cwd */

  ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
}
#endif

static void root_absolute(abts_case *tc, void *data)
{
    apr_status_t rv;
    const char *root = NULL;
    const char *path = ABS_ROOT"foo/bar";

    rv = apr_filepath_root(&root, &path, 0, p);

    ABTS_PTR_NOTNULL(tc, root);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, ABS_ROOT, root);
}

static void root_relative(abts_case *tc, void *data)
{
    apr_status_t rv;
    const char *root = NULL;
    const char *path = "foo/bar";
    char errmsg[256];

    rv = apr_filepath_root(&root, &path, 0, p);
    apr_strerror(rv, errmsg, sizeof(errmsg));

    ABTS_PTR_EQUAL(tc, NULL, root);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_ERELATIVE(rv));
    ABTS_STR_EQUAL(tc, "The given path is relative", errmsg);
}

static void root_from_slash(abts_case *tc, void *data)
{
    apr_status_t rv;
    const char *root = NULL;
    const char *path = "//";

    rv = apr_filepath_root(&root, &path, APR_FILEPATH_TRUENAME, p);

#if defined(WIN32) || defined(OS2)
    ABTS_INT_EQUAL(tc, APR_EINCOMPLETE, rv);
    ABTS_STR_EQUAL(tc, "//", root);
#else
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "/", root);
#endif
    ABTS_STR_EQUAL(tc, "", path);
}

static void root_from_cwd_and_back(abts_case *tc, void *data)
{
    apr_status_t rv;
    const char *root = NULL;
    const char *path = "//";
    char *origpath;
    char *testpath;
#if defined(WIN32) || defined(OS2) || defined(NETWARE)
    int hadfailed;
#endif

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_filepath_get(&origpath, 0, p));
    path = origpath;
    rv = apr_filepath_root(&root, &path, APR_FILEPATH_TRUENAME, p);

#if defined(WIN32) || defined(OS2)
    hadfailed = tc->failed;
    /* It appears some mingw/cygwin and more modern builds can return
     * a lowercase drive designation, but we canonicalize to uppercase
     */
    ABTS_INT_EQUAL(tc, toupper(origpath[0]), root[0]);
    ABTS_INT_EQUAL(tc, ':', root[1]);
    ABTS_INT_EQUAL(tc, '/', root[2]);
    ABTS_INT_EQUAL(tc, 0, root[3]);
    ABTS_STR_EQUAL(tc, origpath + 3, path);
#elif defined(NETWARE)
    ABTS_INT_EQUAL(tc, origpath[0], root[0]);
    {
    char *pt = strchr(root, ':');
    ABTS_PTR_NOTNULL(tc, pt);
    ABTS_INT_EQUAL(tc, ':', pt[0]);
    ABTS_INT_EQUAL(tc, '/', pt[1]);
    ABTS_INT_EQUAL(tc, 0, pt[2]);
    pt = strchr(origpath, ':');
    ABTS_PTR_NOTNULL(tc, pt);
    ABTS_STR_EQUAL(tc, (pt+2), path);
    }
#else
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
    ABTS_STR_EQUAL(tc, "/", root);
    ABTS_STR_EQUAL(tc, origpath + 1, path);
#endif

    rv = apr_filepath_merge(&testpath, root, path, 
                            APR_FILEPATH_TRUENAME
                          | APR_FILEPATH_NOTABOVEROOT
                          | APR_FILEPATH_NOTRELATIVE, p);
    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv);
#if defined(WIN32) || defined(OS2) || defined(NETWARE)
    hadfailed = tc->failed;
#endif
    /* The API doesn't promise equality!!! 
     * apr_filepath_get never promised a canonical filepath.
     * We'll emit noise under verbose so the user is aware,
     * but translate this back to success.
     */
    ABTS_STR_EQUAL(tc, origpath, testpath);
#if defined(WIN32) || defined(OS2) || defined(NETWARE)
    if (!hadfailed) tc->failed = 0;
#endif
}


abts_suite *testnames(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, merge_aboveroot, NULL);
    abts_run_test(suite, merge_belowroot, NULL);
    abts_run_test(suite, merge_noflag, NULL);
    abts_run_test(suite, merge_dotdot, NULL);
    abts_run_test(suite, merge_secure, NULL);
    abts_run_test(suite, merge_notrel, NULL);
    abts_run_test(suite, merge_notrelfail, NULL);
    abts_run_test(suite, merge_notabs, NULL);
    abts_run_test(suite, merge_notabsfail, NULL);
    abts_run_test(suite, merge_dotdot_dotdot_dotdot, NULL);
#if defined(WIN32)
    abts_run_test(suite, merge_lowercasedrive, NULL);
#endif

    abts_run_test(suite, root_absolute, NULL);
    abts_run_test(suite, root_relative, NULL);
    abts_run_test(suite, root_from_slash, NULL);
    abts_run_test(suite, root_from_cwd_and_back, NULL);

    return suite;
}

