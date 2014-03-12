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

#include <log4cxx/rolling/rollingfileappender.h>
#include <log4cxx/logger.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/rolling/timebasedrollingpolicy.h>
#include <log4cxx/helpers/simpledateformat.h>
#include <iostream>
#include <log4cxx/helpers/stringhelper.h>
#include "../util/compare.h"
#include "../logunit.h"
#include <apr_strings.h>


#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif
#include <apr_time.h>




using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::rolling;

/**
 * A rather exhaustive set of tests. Tests include leaving the ActiveFileName
 * argument blank, or setting it, with and without compression, and tests
 * with or without stopping/restarting the RollingFileAppender.
 *
 * The regression tests log a few times using a RollingFileAppender. Then,
 * they predict the names of the files which sould be generated and compare
 * them with witness files.
 *
 * <pre>
         Compression    ActiveFileName  Stop/Restart
 Test1      NO              BLANK          NO
 Test2      NO              BLANK          YES
 Test3      YES             BLANK          NO
 Test4      NO                SET          YES
 Test5      NO                SET          NO
 Test6      YES               SET          NO
 * </pre>
 * 
 */
LOGUNIT_CLASS(TimeBasedRollingTest) {

        LOGUNIT_TEST_SUITE(TimeBasedRollingTest);
           LOGUNIT_TEST(test1);
           LOGUNIT_TEST(test2);
           LOGUNIT_TEST(test3);
           LOGUNIT_TEST(test4);
           LOGUNIT_TEST(test5);
           LOGUNIT_TEST(test6);
        LOGUNIT_TEST_SUITE_END();

    static LoggerPtr logger;

public:

  void setUp() {
      LoggerPtr root(Logger::getRootLogger());
    root->addAppender(
      new ConsoleAppender(new PatternLayout(
         LOG4CXX_STR("%d{ABSOLUTE} [%t] %level %c{2}#%M:%L - %m%n"))));
  }

  void tearDown() {
      LogManager::shutdown();
  }

  /**
   * Test rolling without compression, activeFileName left blank, no stop/start
   */
  void test1()  {
    PatternLayoutPtr layout(new PatternLayout(LOG4CXX_STR("%c{1} - %m%n")));
    RollingFileAppenderPtr rfa(new RollingFileAppender());
    rfa->setLayout(layout);

    LogString datePattern(LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss"));

    TimeBasedRollingPolicyPtr tbrp(new TimeBasedRollingPolicy());
    tbrp->setFileNamePattern(LOG4CXX_STR("output/test1-%d{yyyy-MM-dd_HH_mm_ss}"));
    Pool p;
    tbrp->activateOptions(p);
    rfa->setRollingPolicy(tbrp);
    rfa->activateOptions(p);
    logger->addAppender(rfa);

    SimpleDateFormat sdf(datePattern);
    LogString filenames[4];

    Pool pool;
    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 4; i++) {
      filenames[i] = LOG4CXX_STR("output/test1-");
      sdf.format(filenames[i], now, p);
      now += APR_USEC_PER_SEC;
    } }

    std::cout << "Waiting until next second and 100 millis.";
    delayUntilNextSecond(100);
    std::cout << "Done waiting.";

    { for (int i = 0; i < 5; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    for (int i = 0; i < 4; i++) {
      LogString witness(LOG4CXX_STR("witness/rolling/tbr-test1."));
      StringHelper::toString(i, pool, witness);
      LOGUNIT_ASSERT(Compare::compare(filenames[i], File(witness)));
    }
  }

  /**
   * No compression, with stop/restart, activeFileName left blank
   */
  void test2()  {
    LogString datePattern(LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss"));

    PatternLayoutPtr layout1(new PatternLayout(LOG4CXX_STR("%c{1} - %m%n")));
    RollingFileAppenderPtr rfa1(new RollingFileAppender());
    rfa1->setLayout(layout1);

    TimeBasedRollingPolicyPtr tbrp1(new TimeBasedRollingPolicy());
    tbrp1->setFileNamePattern(LOG4CXX_STR("output/test2-%d{yyyy-MM-dd_HH_mm_ss}"));
    Pool pool;
    tbrp1->activateOptions(pool);
    rfa1->setRollingPolicy(tbrp1);
    rfa1->activateOptions(pool);
    logger->addAppender(rfa1);

    SimpleDateFormat sdf(datePattern);
    LogString filenames[4];

    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 4; i++) {
      filenames[i] = LOG4CXX_STR("output/test2-");
      sdf.format(filenames[i], now, pool);
      now += APR_USEC_PER_SEC;
    } }

    delayUntilNextSecond(100);

    { for (int i = 0; i <= 2; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }


    logger->removeAppender(rfa1);
    rfa1->close();

    PatternLayoutPtr layout2(new PatternLayout(LOG4CXX_STR("%c{1} - %m%n")));
    RollingFileAppenderPtr rfa2 = new RollingFileAppender();
    rfa2->setLayout(layout2);

    TimeBasedRollingPolicyPtr tbrp2 = new TimeBasedRollingPolicy();
    tbrp2->setFileNamePattern(LOG4CXX_STR("output/test2-%d{yyyy-MM-dd_HH_mm_ss}"));
    tbrp2->activateOptions(pool);
    rfa2->setRollingPolicy(tbrp2);
    rfa2->activateOptions(pool);
    logger->addAppender(rfa2);

    { for (int i = 3; i <= 4; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    for (int i = 0; i < 4; i++) {
      LogString witness(LOG4CXX_STR("witness/rolling/tbr-test2."));
      StringHelper::toString(i, pool, witness);
      LOGUNIT_ASSERT(Compare::compare(filenames[i], File(witness)));
    }
  }

  /**
   * With compression, activeFileName left blank, no stop/restart
   */
  void test3() {
    Pool p;
    PatternLayoutPtr layout = new PatternLayout(LOG4CXX_STR("%c{1} - %m%n"));
    RollingFileAppenderPtr rfa = new RollingFileAppender();
    rfa->setAppend(false);
    rfa->setLayout(layout);

    LogString datePattern = LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss");

    TimeBasedRollingPolicyPtr tbrp = new TimeBasedRollingPolicy();
    tbrp->setFileNamePattern(LogString(LOG4CXX_STR("output/test3-%d{")) + datePattern + LogString(LOG4CXX_STR("}.gz")));
    tbrp->activateOptions(p);
    rfa->setRollingPolicy(tbrp);
    rfa->activateOptions(p);
    logger->addAppender(rfa);

    DateFormatPtr sdf = new SimpleDateFormat(datePattern);
    LogString filenames[4];

    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 4; i++) {
      filenames[i] = LOG4CXX_STR("output/test3-");
      sdf->format(filenames[i], now, p);
      filenames[i].append(LOG4CXX_STR(".gz"));
      now += APR_USEC_PER_SEC;
    } }

    filenames[3].resize(filenames[3].size() - 3);

    delayUntilNextSecond(100);

    { for (int i = 0; i < 5; i++) {
        std::string message("Hello---");
        message.append(p.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    LOGUNIT_ASSERT_EQUAL(true, File(filenames[0]).exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File(filenames[1]).exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File(filenames[2]).exists(p));

    LOGUNIT_ASSERT_EQUAL(true, Compare::compare(File(filenames[3]), File(LOG4CXX_STR("witness/rolling/tbr-test3.3"))));
  }

  /**
   * Without compression, activeFileName set,  with stop/restart
   */
  void test4()  {
    LogString datePattern = LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss");

    PatternLayoutPtr layout1 = new PatternLayout(LOG4CXX_STR("%c{1} - %m%n"));
    RollingFileAppenderPtr rfa1 = new RollingFileAppender();
    rfa1->setLayout(layout1);

    Pool pool;

    TimeBasedRollingPolicyPtr tbrp1 = new TimeBasedRollingPolicy();
    rfa1->setFile(LOG4CXX_STR("output/test4.log"));
    tbrp1->setFileNamePattern(LOG4CXX_STR("output/test4-%d{yyyy-MM-dd_HH_mm_ss}"));
    tbrp1->activateOptions(pool);
    rfa1->setRollingPolicy(tbrp1);
    rfa1->activateOptions(pool);
    logger->addAppender(rfa1);

    SimpleDateFormat sdf(datePattern);
    LogString filenames[4];

    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 3; i++) {
      filenames[i] = LOG4CXX_STR("output/test4-");
      sdf.format(filenames[i], now, pool);
      now += APR_USEC_PER_SEC;
    } }
    filenames[3] = LOG4CXX_STR("output/test4.log");

    std::cout << "Waiting until next second and 100 millis.";
    delayUntilNextSecond(100);
    std::cout << "Done waiting.";

    { for (int i = 0; i <= 2; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    logger->removeAppender(rfa1);
    rfa1->close();

    PatternLayoutPtr layout2 = new PatternLayout(LOG4CXX_STR("%c{1} - %m%n"));
    RollingFileAppenderPtr rfa2 = new RollingFileAppender();
    rfa2->setLayout(layout2);

    TimeBasedRollingPolicyPtr tbrp2 = new TimeBasedRollingPolicy();
    tbrp2->setFileNamePattern(LOG4CXX_STR("output/test4-%d{yyyy-MM-dd_HH_mm_ss}"));
    rfa2->setFile(LOG4CXX_STR("output/test4.log"));
    tbrp2->activateOptions(pool);
    rfa2->setRollingPolicy(tbrp2);
    rfa2->activateOptions(pool);
    logger->addAppender(rfa2);

    { for (int i = 3; i <= 4; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    for (int i = 0; i < 4; i++) {
      LogString witness(LOG4CXX_STR("witness/rolling/tbr-test4."));
      StringHelper::toString(i, pool, witness);
      LOGUNIT_ASSERT(Compare::compare(filenames[i], File(witness)));
    }
  }

  /**
   * No compression, activeFileName set,  without stop/restart
   */
  void test5()  {
    PatternLayoutPtr layout = new PatternLayout(LOG4CXX_STR("%c{1} - %m%n"));
    RollingFileAppenderPtr rfa = new RollingFileAppender();
    rfa->setLayout(layout);

    LogString datePattern(LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss"));

    TimeBasedRollingPolicyPtr tbrp = new TimeBasedRollingPolicy();
    tbrp->setFileNamePattern(LOG4CXX_STR("output/test5-%d{yyyy-MM-dd_HH_mm_ss}"));
    rfa->setFile(LOG4CXX_STR("output/test5.log"));
    Pool pool;

    tbrp->activateOptions(pool);
    rfa->setRollingPolicy(tbrp);
    rfa->activateOptions(pool);
    logger->addAppender(rfa);

    SimpleDateFormat sdf(datePattern);
    LogString filenames[4];

    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 3; i++) {
      filenames[i] = LOG4CXX_STR("output/test5-");
      sdf.format(filenames[i], now, pool);
      now += APR_USEC_PER_SEC;
    } }
    filenames[3] = LOG4CXX_STR("output/test5.log");

    std::cout << "Waiting until next second and 100 millis.";
    delayUntilNextSecond(100);
    std::cout << "Done waiting.";

    { for (int i = 0; i < 5; i++) {
        std::string message("Hello---");
        message.append(pool.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    for (int i = 0; i < 4; i++) {
      LogString witness(LOG4CXX_STR("witness/rolling/tbr-test5."));
      StringHelper::toString(i, pool, witness);
      LOGUNIT_ASSERT(Compare::compare(filenames[i], File(witness)));
    }
  }

  /**
   * With compression, activeFileName set, no stop/restart,
   */
  void test6() {
    Pool p;
    PatternLayoutPtr layout = new PatternLayout(LOG4CXX_STR("%c{1} - %m%n"));
    RollingFileAppenderPtr rfa = new RollingFileAppender();
    rfa->setAppend(false);
    rfa->setLayout(layout);

    LogString datePattern = LOG4CXX_STR("yyyy-MM-dd_HH_mm_ss");

    TimeBasedRollingPolicyPtr tbrp = new TimeBasedRollingPolicy();
    tbrp->setFileNamePattern(LogString(LOG4CXX_STR("output/test6-%d{")) + datePattern + LogString(LOG4CXX_STR("}.gz")));
    rfa->setFile(LOG4CXX_STR("output/test6.log"));
    tbrp->activateOptions(p);
    rfa->setRollingPolicy(tbrp);
    rfa->activateOptions(p);
    logger->addAppender(rfa);

    DateFormatPtr sdf = new SimpleDateFormat(datePattern);
    LogString filenames[4];

    apr_time_t now = apr_time_now();
    { for (int i = 0; i < 3; i++) {
      filenames[i] = LOG4CXX_STR("output/test6-");
      sdf->format(filenames[i], now, p);
      filenames[i].append(LOG4CXX_STR(".gz"));
      now += APR_USEC_PER_SEC;
    } }

    filenames[3] = LOG4CXX_STR("output/test6.log");

    delayUntilNextSecond(100);

    { for (int i = 0; i < 5; i++) {
        std::string message("Hello---");
        message.append(p.itoa(i));
        LOG4CXX_DEBUG(logger, message);
        apr_sleep(APR_USEC_PER_SEC/2);
    } }

    LOGUNIT_ASSERT_EQUAL(true, File(filenames[0]).exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File(filenames[1]).exists(p));
    LOGUNIT_ASSERT_EQUAL(true, File(filenames[2]).exists(p));

    LOGUNIT_ASSERT_EQUAL(true, Compare::compare(File(filenames[3]), File(LOG4CXX_STR("witness/rolling/tbr-test6.3"))));

  }

  void delayUntilNextSecond(int millis) {
    apr_time_t now = apr_time_now();
    apr_time_t next = ((now / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC
          + millis * 1000L;

    apr_sleep(next - now);
  }

  void delayUntilNextMinute(int seconds) {
    apr_time_t now = apr_time_now();
    apr_time_t next = ((now / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC
          + seconds * 1000000L;

    apr_sleep(next - now);
  }

};


LoggerPtr TimeBasedRollingTest::logger(Logger::getLogger("org.apache.log4j.TimeBasedRollingTest"));

LOGUNIT_TEST_SUITE_REGISTRATION(TimeBasedRollingTest);
