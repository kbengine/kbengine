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



#include "../logunit.h"

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/file.h>

#include "../util/compare.h"
#include "xlevel.h"
#include "../testchar.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::xml;


LOGUNIT_CLASS(CustomLevelTestCase)
{
   LOGUNIT_TEST_SUITE(CustomLevelTestCase);
      LOGUNIT_TEST(test1);
      LOGUNIT_TEST(test2);
      LOGUNIT_TEST(test3);
      LOGUNIT_TEST(test4);
   LOGUNIT_TEST_SUITE_END();

   LoggerPtr root;
   LoggerPtr logger;
    static const File TEMP;

public:
   void setUp()
   {
      root = Logger::getRootLogger();
      logger = Logger::getLogger(LOG4CXX_TEST_STR("xml.CustomLevelTestCase"));
   }

   void tearDown()
   {
      root->getLoggerRepository()->resetConfiguration();

      LoggerPtr logger1 = Logger::getLogger(LOG4CXX_TEST_STR("LOG4J"));
      logger1->setAdditivity(false);
      logger1->addAppender(
         new ConsoleAppender(new PatternLayout(LOG4CXX_STR("log4j: %-22c{2} - %m%n"))));
   }

   void test1()
   {
      DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/customLevel1.xml"));
      common();
        const File witness("witness/customLevel.1");
      LOGUNIT_ASSERT(Compare::compare(TEMP, witness));
   }

   void test2()
   {
      DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/customLevel2.xml"));
      common();
        const File witness("witness/customLevel.2");
      LOGUNIT_ASSERT(Compare::compare(TEMP, witness));
   }

   void test3()
   {
      DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/customLevel3.xml"));
      common();
        const File witness("witness/customLevel.3");
      LOGUNIT_ASSERT(Compare::compare(TEMP, witness));
   }

   void test4()
   {
      DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/customLevel4.xml"));
      common();
        const File witness("witness/customLevel.4");
      LOGUNIT_ASSERT(Compare::compare(TEMP, witness));
   }

   void common()
   {
      int i = 0;
        std::ostringstream os;
        os << "Message " << ++i;
      LOG4CXX_DEBUG(logger, os.str());
        os.str("");
        os << "Message " <<  ++i;
      LOG4CXX_INFO(logger, os.str());
        os.str("");
        os << "Message " <<  ++i;
      LOG4CXX_WARN(logger, os.str());
        os.str("");
        os << "Message " <<  ++i;
      LOG4CXX_ERROR(logger, os.str());
        os.str("");
        os << "Message " <<  ++i;
      LOG4CXX_LOG(logger, XLevel::getTrace(), os.str());
   }
};

LOGUNIT_TEST_SUITE_REGISTRATION(CustomLevelTestCase);

const File CustomLevelTestCase::TEMP("output/temp");

