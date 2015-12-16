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
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/iso8601dateformat.h>
#include "../logunit.h"
#define LOG4CXX_TEST
#include <log4cxx/private/log4cxx_private.h>
#if LOG4CXX_HAS_STD_LOCALE
#include <locale>
#endif
#include "../insertwide.h"
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/date.h>



using namespace log4cxx;
using namespace log4cxx::helpers;

/**
   Unit test {@link ISO8601DateFormat}.
   
   */
LOGUNIT_CLASS(ISO8601DateFormatTestCase) {
     LOGUNIT_TEST_SUITE( ISO8601DateFormatTestCase );
     LOGUNIT_TEST( test1 );
     LOGUNIT_TEST( test2 );
     LOGUNIT_TEST( test3 );
     LOGUNIT_TEST( test4 );
     LOGUNIT_TEST( test5 );
     LOGUNIT_TEST( test6 );
     LOGUNIT_TEST( test7 );
     LOGUNIT_TEST_SUITE_END();

  /**
   * Asserts that formatting the provided date results
   * in the expected string.
   *
   * @param date Date date
   * @param timeZone TimeZone timezone for conversion
   * @param expected String expected string
   */
  void assertFormattedTime(log4cxx_time_t date,
                           const TimeZonePtr& timeZone,
                           const LogString& expected) {
    ISO8601DateFormat formatter;
    formatter.setTimeZone(timeZone);
    LogString actual;
    Pool p;
    formatter.format(actual, date, p);
    LOGUNIT_ASSERT_EQUAL(expected, actual);
  }


public:
  /**
   * Convert 02 Jan 2004 00:00:00 GMT for GMT.
   */
  void test1() {
    log4cxx_time_t jan2 = Date::getMicrosecondsPerDay() * 12419;
    assertFormattedTime(jan2, TimeZone::getGMT(),
          LOG4CXX_STR("2004-01-02 00:00:00,000"));
  }

  /**
   * Convert 03 Jan 2004 00:00:00 GMT for America/Chicago.
   */
  void test2() {
    //
    //   03 Jan 2004 00:00 GMT
    //       (asking for the same time at a different timezone
    //          will ignore the change of timezone)
    log4cxx_time_t jan3 = Date::getMicrosecondsPerDay() * 12420;
    assertFormattedTime(jan3, TimeZone::getTimeZone(LOG4CXX_STR("GMT-6")),
          LOG4CXX_STR("2004-01-02 18:00:00,000"));
  }


  /**
   * Convert 30 Jun 2004 00:00:00 GMT for GMT.
   */
  void test3() {
    log4cxx_time_t jun30 = Date::getMicrosecondsPerDay() * 12599;
    assertFormattedTime(jun30, TimeZone::getGMT(),
          LOG4CXX_STR("2004-06-30 00:00:00,000"));
  }

  /**
   * Convert 1 Jul 2004 00:00:00 GMT for Chicago, daylight savings in effect.
   */
  void test4() {
    log4cxx_time_t jul1 = Date::getMicrosecondsPerDay() * 12600;
    assertFormattedTime(jul1, TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")),
           LOG4CXX_STR("2004-06-30 19:00:00,000"));
  }

  /**
   * Test multiple calls in close intervals.
   */
  void test5() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    log4cxx_time_t ticks =  Date::getMicrosecondsPerDay() * 12601;
    assertFormattedTime(ticks, TimeZone::getGMT(),
          LOG4CXX_STR("2004-07-02 00:00:00,000"));
    assertFormattedTime(ticks + 8000, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-02 00:00:00,008"));
    assertFormattedTime(ticks + 17000, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-02 00:00:00,017"));
    assertFormattedTime(ticks + 237000, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-02 00:00:00,237"));
    assertFormattedTime(ticks + 1415000, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-02 00:00:01,415"));
  }

  /**
   *  Check that caching does not disregard timezone.
   * This test would fail for revision 1.4 of DateTimeDateFormat.java.
   */
  void test6() {
    log4cxx_time_t jul3 =  Date::getMicrosecondsPerDay() * 12602;
    assertFormattedTime(jul3, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-03 00:00:00,000"));
    assertFormattedTime(jul3, TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")),
           LOG4CXX_STR("2004-07-02 19:00:00,000"));
    assertFormattedTime(jul3, TimeZone::getGMT(),
           LOG4CXX_STR("2004-07-03 00:00:00,000"));
  }

  /**
   * Checks that numberFormat is implemented.
   */
  void test7() {
    LogString number;
    ISO8601DateFormat formatter;
    Pool p;
    formatter.numberFormat(number, 87, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("87"), number);
  }



};


LOGUNIT_TEST_SUITE_REGISTRATION(ISO8601DateFormatTestCase);
