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


#include <log4cxx/logger.h>
#include <log4cxx/hierarchy.h>
#include "logunit.h"
#include "insertwide.h"

using namespace log4cxx;

/**
 * Tests hierarchy.
 * 
 */
LOGUNIT_CLASS(HierarchyTest) {
  LOGUNIT_TEST_SUITE(HierarchyTest);
          LOGUNIT_TEST(testGetParent);
  LOGUNIT_TEST_SUITE_END();
public:

    /**
     * Tests getParent.
     */
  void testGetParent() {
      //
      //  Note: test inspired by LOGCXX-118.
      //
      LoggerPtr logger1(Logger::getLogger("HierarchyTest_testGetParent.logger1"));
      LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("root")), logger1->getParent()->getName());
      LoggerPtr logger2(Logger::getLogger("HierarchyTest_testGetParent.logger2"));
      LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("root")), logger1->getParent()->getName());
      LoggerPtr logger3(Logger::getLogger("HierarchyTest_testGetParent"));
      LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("HierarchyTest_testGetParent")), 
          logger1->getParent()->getName());
      LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("HierarchyTest_testGetParent")), 
          logger2->getParent()->getName());
  }

};

LOGUNIT_TEST_SUITE_REGISTRATION(HierarchyTest);

