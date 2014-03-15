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

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>


#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>
#include "../insertwide.h"
#include "../logunit.h"

using namespace log4cxx;

LOGUNIT_CLASS(TestCase4)
{
   LOGUNIT_TEST_SUITE(TestCase4);
      LOGUNIT_TEST(combinedTest);
   LOGUNIT_TEST_SUITE_END();

public:
   void setUp()
   {
   }

   void tearDown()
   {
      LogManager::shutdown();
   }

   void combinedTest()
   {
      LoggerPtr root = Logger::getRootLogger();
     LOG4CXX_DEBUG(root, "Hello, world");
      bool rootIsConfigured = !root->getAllAppenders().empty();
      LOGUNIT_ASSERT(rootIsConfigured);

      AppenderList list = root->getAllAppenders();
      LOGUNIT_ASSERT_EQUAL((size_t) 1, list.size());
      AppenderPtr appender = list.front();
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("D1"), appender->getName());
   }

};

LOGUNIT_TEST_SUITE_REGISTRATION_DISABLED(TestCase4)

