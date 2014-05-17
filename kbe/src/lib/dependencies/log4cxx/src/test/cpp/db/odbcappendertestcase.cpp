/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/db/odbcappender.h>
#include "../appenderskeletontestcase.h"
#include "../logunit.h"

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>

#ifdef LOG4CXX_HAVE_ODBC

using namespace log4cxx;
using namespace log4cxx::helpers;

/**
   Unit tests of log4cxx::SocketAppender
 */
class ODBCAppenderTestCase : public AppenderSkeletonTestCase
{
   LOGUNIT_TEST_SUITE(ODBCAppenderTestCase);
                //
                //    tests inherited from AppenderSkeletonTestCase
                //
                LOGUNIT_TEST(testDefaultThreshold);
                LOGUNIT_TEST(testSetOptionThreshold);

   LOGUNIT_TEST_SUITE_END();


public:

        AppenderSkeleton* createAppenderSkeleton() const {
         return new log4cxx::db::ODBCAppender();
        }
};

LOGUNIT_TEST_SUITE_REGISTRATION(ODBCAppenderTestCase);

#endif
