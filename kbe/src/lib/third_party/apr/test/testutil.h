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

#include "apr_pools.h"
#include "apr_general.h"
#include "abts.h"

#ifndef APR_TEST_UTIL
#define APR_TEST_UTIL

/* XXX: FIXME - these all should become much more utilitarian 
 * and part of apr, itself
 */

#ifdef WIN32
#ifdef BINPATH
#define TESTBINPATH APR_STRINGIFY(BINPATH) "/"
#else
#define TESTBINPATH ""
#endif
#else
#define TESTBINPATH "./"
#endif

#ifdef WIN32
#define EXTENSION ".exe"
#elif NETWARE
#define EXTENSION ".nlm"
#else
#define EXTENSION
#endif

#define STRING_MAX 8096

/* Some simple functions to make the test apps easier to write and
 * a bit more consistent...
 */

extern apr_pool_t *p;

/* Assert that RV is an APR_SUCCESS value; else fail giving strerror
 * for RV and CONTEXT message. */
void apr_assert_success(abts_case* tc, const char *context, 
                        apr_status_t rv, int lineno);
#define APR_ASSERT_SUCCESS(tc, ctxt, rv) \
             apr_assert_success(tc, ctxt, rv, __LINE__)

void initialize(void);

abts_suite *testatomic(abts_suite *suite);
abts_suite *testdir(abts_suite *suite);
abts_suite *testdso(abts_suite *suite);
abts_suite *testdup(abts_suite *suite);
abts_suite *testescape(abts_suite *suite);
abts_suite *testenv(abts_suite *suite);
abts_suite *testfile(abts_suite *suite);
abts_suite *testfilecopy(abts_suite *suite);
abts_suite *testfileinfo(abts_suite *suite);
abts_suite *testflock(abts_suite *suite);
abts_suite *testfmt(abts_suite *suite);
abts_suite *testfnmatch(abts_suite *suite);
abts_suite *testgetopt(abts_suite *suite);
abts_suite *testglobalmutex(abts_suite *suite);
abts_suite *testhash(abts_suite *suite);
abts_suite *testipsub(abts_suite *suite);
abts_suite *testlock(abts_suite *suite);
abts_suite *testcond(abts_suite *suite);
abts_suite *testlfs(abts_suite *suite);
abts_suite *testmmap(abts_suite *suite);
abts_suite *testnames(abts_suite *suite);
abts_suite *testoc(abts_suite *suite);
abts_suite *testpath(abts_suite *suite);
abts_suite *testpipe(abts_suite *suite);
abts_suite *testpoll(abts_suite *suite);
abts_suite *testpool(abts_suite *suite);
abts_suite *testproc(abts_suite *suite);
abts_suite *testprocmutex(abts_suite *suite);
abts_suite *testrand(abts_suite *suite);
abts_suite *testsleep(abts_suite *suite);
abts_suite *testshm(abts_suite *suite);
abts_suite *testsock(abts_suite *suite);
abts_suite *testsockets(abts_suite *suite);
abts_suite *testsockopt(abts_suite *suite);
abts_suite *teststr(abts_suite *suite);
abts_suite *teststrnatcmp(abts_suite *suite);
abts_suite *testtable(abts_suite *suite);
abts_suite *testtemp(abts_suite *suite);
abts_suite *testthread(abts_suite *suite);
abts_suite *testtime(abts_suite *suite);
abts_suite *testud(abts_suite *suite);
abts_suite *testuser(abts_suite *suite);
abts_suite *testvsn(abts_suite *suite);

#endif /* APR_TEST_INCLUDES */
