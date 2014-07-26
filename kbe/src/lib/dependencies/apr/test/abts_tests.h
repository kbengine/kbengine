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

#ifndef APR_TEST_INCLUDES
#define APR_TEST_INCLUDES

#include "abts.h"
#include "testutil.h"

const struct testlist {
    abts_suite *(*func)(abts_suite *suite);
} alltests[] = {
    {testatomic},
    {testdir},
    {testdso},
    {testdup},
    {testenv},
    {testescape},
    {testfile},
    {testfilecopy},
    {testfileinfo},
    {testflock},
    {testfmt},
    {testfnmatch},
    {testgetopt},
#if 0 /* not ready yet due to API issues */
    {testglobalmutex},
#endif
    {testhash},
    {testipsub},
    {testlock},
    {testcond},
    {testlfs},
    {testmmap},
    {testnames},
    {testoc},
    {testpath},
    {testpipe},
    {testpoll},
    {testpool},
    {testproc},
    {testprocmutex},
    {testrand},
    {testsleep},
    {testshm},
    {testsock},
    {testsockets},
    {testsockopt},
    {teststr},
    {teststrnatcmp},
    {testtable},
    {testtemp},
    {testthread},
    {testtime},
    {testud},
    {testuser},
    {testvsn}
};

#endif /* APR_TEST_INCLUDES */
