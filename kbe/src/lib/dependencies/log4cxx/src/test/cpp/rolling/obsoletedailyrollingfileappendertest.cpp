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
#include <log4cxx/dailyrollingfileappender.h>
#include <log4cxx/helpers/stringhelper.h>


using namespace log4cxx;
using namespace log4cxx::rolling;
using namespace log4cxx::xml;
using namespace log4cxx::filter;
using namespace log4cxx::helpers;


/**
 * Tests the emulation of org.apache.log4j.DailyRollingFileAppender
 *
 * 
 *
 */
LOGUNIT_CLASS(ObsoleteDailyRollingFileAppenderTest)  {
  LOGUNIT_TEST_SUITE(ObsoleteDailyRollingFileAppenderTest);
          LOGUNIT_TEST(test1);
          LOGUNIT_TEST(test2);
  LOGUNIT_TEST_SUITE_END();


public:

  void tearDown() {
    LogManager::shutdown();
  }

  /**
   * Test basic rolling functionality.
   */
  void test1() {
    PropertyConfigurator::configure(File("input/rolling/obsoleteDRFA1.properties"));

    int preCount = getFileCount("output", LOG4CXX_STR("obsoleteDRFA-test1.log."));
    LoggerPtr logger(Logger::getLogger("org.apache.log4j.ObsoleteDailyRollingFileAppenderTest"));

    char msg[11];
    strcpy(msg, "Hello---??");

    for (int i = 0; i < 25; i++) {
      apr_sleep(100000);
      msg[8] = (char) ('0' + (i / 10));
      msg[9] = (char) ('0' + (i % 10));
      LOG4CXX_DEBUG(logger, msg);
    }

    int postCount = getFileCount("output", LOG4CXX_STR("obsoleteDRFA-test1.log."));
    LOGUNIT_ASSERT_EQUAL(true, postCount > preCount);
  }

  /**
   * Test basic rolling functionality.
   * @deprecated Class under test is deprecated.
   */
  void test2() {
    PatternLayoutPtr layout(new PatternLayout(LOG4CXX_STR("%m%n")));
    log4cxx::DailyRollingFileAppenderPtr rfa(new log4cxx::DailyRollingFileAppender());
    rfa->setName(LOG4CXX_STR("ROLLING"));
    rfa->setLayout(layout);
    rfa->setAppend(false);
    rfa->setFile(LOG4CXX_STR("output/obsoleteDRFA-test2.log"));
    rfa->setDatePattern(LOG4CXX_STR("'.'yyyy-MM-dd-HH_mm_ss"));
    Pool p;
    rfa->activateOptions(p);
    LoggerPtr root(Logger::getRootLogger());
    root->addAppender(rfa);
    LoggerPtr logger(Logger::getLogger("org.apache.log4j.ObsoleteDailyRollingAppenderTest"));

    int preCount = getFileCount("output", LOG4CXX_STR("obsoleteDRFA-test2.log."));

    char msg[11];
    strcpy(msg, "Hello---??");
    for (int i = 0; i < 25; i++) {
      apr_sleep(100000);
      msg[8] = (char) ('0' + i / 10);
      msg[9] = (char) ('0' + i % 10);
      LOG4CXX_DEBUG(logger, msg);
    }

    int postCount = getFileCount("output", LOG4CXX_STR("obsoleteDRFA-test2.log."));
    LOGUNIT_ASSERT_EQUAL(true, postCount > preCount);
  }

private:
  static int getFileCount(const char* dir, const LogString& initial) {
    Pool p;
    std::vector<LogString> files(File(dir).list(p));
    int count = 0;

    for (size_t i = 0; i < files.size(); i++) {
      if (StringHelper::startsWith(files[i], initial)) {
        count++;
      }
    }

    return count;
  }
};

LOGUNIT_TEST_SUITE_REGISTRATION(ObsoleteDailyRollingFileAppenderTest);

