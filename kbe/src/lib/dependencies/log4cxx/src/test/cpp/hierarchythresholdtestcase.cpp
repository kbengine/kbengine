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
#include <log4cxx/propertyconfigurator.h>
#include "util/compare.h"
#include "xml/xlevel.h"
#include "logunit.h"
#include "testchar.h"
#include <log4cxx/spi/loggerrepository.h>

using namespace log4cxx;

/**
Test the configuration of the hierarchy-wide threshold.
*/
LOGUNIT_CLASS(HierarchyThresholdTestCase)
{
   LOGUNIT_TEST_SUITE(HierarchyThresholdTestCase);
      LOGUNIT_TEST(test1);
      LOGUNIT_TEST(test2);
      LOGUNIT_TEST(test3);
      LOGUNIT_TEST(test4);
      LOGUNIT_TEST(test5);
      LOGUNIT_TEST(test6);
      LOGUNIT_TEST(test7);
      LOGUNIT_TEST(test8);
   LOGUNIT_TEST_SUITE_END();


public:
   void setUp()
   {
   }

   void tearDown()
   {
      logger->getLoggerRepository()->resetConfiguration();
   }

   void test1()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold1.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.1")));
   }

   void test2()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold2.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.2")));
   }

   void test3()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold3.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.3")));
   }

   void test4()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold4.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.4")));
   }

   void test5()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold5.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.5")));
   }

   void test6()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold6.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.6")));
   }

   void test7()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold7.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.7")));
   }

   void test8()
   {
      PropertyConfigurator::configure(LOG4CXX_FILE("input/hierarchyThreshold8.properties"));
      common();
      LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/hierarchyThreshold.8")));
   }

   static void common()
   {
      logger->log(XLevel::getTrace(), LOG4CXX_TEST_STR("m0"));
      logger->debug(LOG4CXX_TEST_STR("m1"));
      logger->info(LOG4CXX_TEST_STR("m2"));
      logger->warn(LOG4CXX_TEST_STR("m3"));
      logger->error(LOG4CXX_TEST_STR("m4"));
      logger->fatal(LOG4CXX_TEST_STR("m5"));
   }

private:
   static File TEMP;
   static LoggerPtr logger;
};

File HierarchyThresholdTestCase::TEMP(LOG4CXX_FILE("output/temp"));

LoggerPtr HierarchyThresholdTestCase::logger =
   Logger::getLogger(LOG4CXX_TEST_STR("HierarchyThresholdTestCase"));

LOGUNIT_TEST_SUITE_REGISTRATION(HierarchyThresholdTestCase);
