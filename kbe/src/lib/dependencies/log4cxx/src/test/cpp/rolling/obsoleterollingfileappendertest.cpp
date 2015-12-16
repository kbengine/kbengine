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
#include "../util/compare.h"
#include "../insertwide.h"
#include "../logunit.h"
#include <apr_time.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/rolling/fixedwindowrollingpolicy.h>
#include <log4cxx/rolling/filterbasedtriggeringpolicy.h>
#include <log4cxx/filter/levelrangefilter.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/helpers/stringhelper.h>

 using namespace log4cxx;
 using namespace log4cxx::xml;
 using namespace log4cxx::filter;
 using namespace log4cxx::helpers;

/**
 * Tests the emulation of org.apache.log4j.RollingFileAppender
 *
 * 
 *
 */
 LOGUNIT_CLASS(ObsoleteRollingFileAppenderTest)  {
   LOGUNIT_TEST_SUITE(ObsoleteRollingFileAppenderTest);
           LOGUNIT_TEST(test1);
           LOGUNIT_TEST(test2);
           LOGUNIT_TEST(testIsOptionHandler);
           LOGUNIT_TEST(testClassForName);
   LOGUNIT_TEST_SUITE_END();


 public:

  void tearDown() {
    LogManager::shutdown();
  }

  /**
   * Test basic rolling functionality.
   */
  void test1() {
    PropertyConfigurator::configure(File("input/rolling/obsoleteRFA1.properties"));

    char msg[] = { 'H', 'e', 'l', 'l', 'o', '-', '-', '-', '?', 0};
    LoggerPtr logger(Logger::getLogger("org.apache.logj4.ObsoleteRollingFileAppenderTest"));

    // Write exactly 10 bytes with each log
    for (int i = 0; i < 25; i++) {
      apr_sleep(100000);

      if (i < 10) {
        msg[8] = (char) ('0' + i);
        LOG4CXX_DEBUG(logger, msg);
      } else if (i < 100) {
        msg[7] = (char) ('0' + i / 10);
        msg[8] = (char) ('0' + i % 10);
        LOG4CXX_DEBUG(logger, msg);
      }
    }

    Pool p;
    LOGUNIT_ASSERT_EQUAL(true, File("output/obsoleteRFA-test1.log").exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File("output/obsoleteRFA-test1.log.1").exists(p));
  }

  /**
   * Test basic rolling functionality.
   * @deprecated Class under test is deprecated.
   */
  void test2()  {
    PatternLayoutPtr layout(new PatternLayout(LOG4CXX_STR("%m\n")));
    log4cxx::RollingFileAppenderPtr rfa(
      new log4cxx::RollingFileAppender());
    rfa->setName(LOG4CXX_STR("ROLLING"));
    rfa->setLayout(layout);
    rfa->setOption(LOG4CXX_STR("append"), LOG4CXX_STR("false"));
    rfa->setMaximumFileSize(100);
    rfa->setFile(LOG4CXX_STR("output/obsoleteRFA-test2.log"));
    Pool p;
    rfa->activateOptions(p);
    LoggerPtr root(Logger::getRootLogger());
    root->addAppender(rfa);

    char msg[] = { 'H', 'e', 'l', 'l', 'o', '-', '-', '-', '?', 0};
    LoggerPtr logger(Logger::getLogger("org.apache.logj4.ObsoleteRollingFileAppenderTest"));

    // Write exactly 10 bytes with each log
    for (int i = 0; i < 25; i++) {
      apr_sleep(100000);

      if (i < 10) {
        msg[8] = (char) ('0' + i);
        LOG4CXX_DEBUG(logger, msg);
      } else if (i < 100) {
        msg[7] = (char) ('0' + i / 10);
        msg[8] = (char) ('0' + i % 10);
        LOG4CXX_DEBUG(logger, msg);
      }
    }

    LOGUNIT_ASSERT_EQUAL(true, File("output/obsoleteRFA-test2.log").exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File("output/obsoleteRFA-test2.log.1").exists(p));
  }

  /**
   *  Tests if class is declared to support the OptionHandler interface.
   *  See LOGCXX-136.
   */
  void testIsOptionHandler() {
      RollingFileAppenderPtr rfa(new RollingFileAppender());
      LOGUNIT_ASSERT_EQUAL(true, rfa->instanceof(log4cxx::spi::OptionHandler::getStaticClass()));
  }

  void testClassForName() {
      LogString className(LOG4CXX_STR("org.apache.log4j.RollingFileAppender"));
      const Class& myclass = Class::forName(className);
      LOGUNIT_ASSERT_EQUAL(className, LogString(myclass.getName()));
  }
};

LOGUNIT_TEST_SUITE_REGISTRATION(ObsoleteRollingFileAppenderTest);

