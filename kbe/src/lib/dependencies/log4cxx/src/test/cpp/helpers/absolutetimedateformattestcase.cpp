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
#include "../logunit.h" 
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/absolutetimedateformat.h>
#include "../insertwide.h"
#include <apr.h>
#include <apr_time.h>
#include <log4cxx/helpers/pool.h>

//Define INT64_C for compilers that don't have it
#if (!defined(INT64_C))
#define INT64_C(value)  value ## LL
#endif


using namespace log4cxx;
using namespace log4cxx::helpers;



/**
   Unit test {@link AbsoluteTimeDateFormat}.
   
   */
LOGUNIT_CLASS(AbsoluteTimeDateFormatTestCase) {
  LOGUNIT_TEST_SUITE(AbsoluteTimeDateFormatTestCase);
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

  /**
   * Asserts that formatting the provided date results
   * in the expected string.
   *
   * @param date Date date
   * @param timeZone TimeZone timezone for conversion
   * @param expected String expected string
   */
  private:
  void assertFormattedTime(apr_time_t date,
                           const TimeZonePtr& timeZone,
                           const LogString& expected) {
    AbsoluteTimeDateFormat formatter;
    formatter.setTimeZone(timeZone);
    LogString actual;
    Pool p;
    formatter.format(actual, date, p);
    LOGUNIT_ASSERT_EQUAL(expected, actual);
  }

#define MICROSECONDS_PER_DAY APR_INT64_C(86400000000)

  public:
  /**
   * Convert 02 Jan 2004 00:00:00 GMT for GMT.
   */
  void test1() {
    //
    //   02 Jan 2004 00:00 GMT
    //
    apr_time_t jan2 = MICROSECONDS_PER_DAY * 12419;
    assertFormattedTime(jan2, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,000"));
  }

  /**
   * Convert 03 Jan 2004 00:00:00 GMT for America/Chicago.
   */
  void test2() {
    //
    //   03 Jan 2004 00:00 GMT
    //       (asking for the same time at a different timezone
    //          will ignore the change of timezone)
    apr_time_t jan2 = MICROSECONDS_PER_DAY * 12420;
    assertFormattedTime(jan2, TimeZone::getTimeZone(LOG4CXX_STR("GMT-6")), LOG4CXX_STR("18:00:00,000"));
  }

  /**
   * Convert 29 Jun 2004 00:00:00 GMT for GMT.
   */
  void test3() {
    apr_time_t jun29 = MICROSECONDS_PER_DAY * 12599;
    assertFormattedTime(jun29, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,000"));
  }

  /**
   * Convert 29 Jun 2004 00:00:00 GMT for Chicago, daylight savings in effect.
   */
  void test4() {
    apr_time_t jun30 = MICROSECONDS_PER_DAY * 12600;
    //
    //   log4cxx doesn't support non-fixed timezones at this time
    //      passing the fixed equivalent to Chicago's Daylight Savings Time
    //
    assertFormattedTime(jun30, TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")), LOG4CXX_STR("19:00:00,000"));
  }

  /**
   * Test multiple calls in close intervals.
   */
  void test5() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    apr_time_t ticks = MICROSECONDS_PER_DAY * 12601;
    assertFormattedTime(ticks, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,000"));
    assertFormattedTime(ticks + 8000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,008"));
    assertFormattedTime(ticks + 17000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,017"));
    assertFormattedTime(ticks + 237000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,237"));
    assertFormattedTime(ticks + 1415000, TimeZone::getGMT(), LOG4CXX_STR("00:00:01,415"));
  }

  /**
   *  Check that caching does not disregard timezone.
   * This test would fail for revision 1.4 of AbsoluteTimeDateFormat.java.
   */
  void test6() {
    apr_time_t jul2 = MICROSECONDS_PER_DAY * 12602;
    assertFormattedTime(jul2, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,000"));
    assertFormattedTime(jul2, TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")), LOG4CXX_STR("19:00:00,000"));
  }

  /**
   * Test multiple calls in close intervals predating 1 Jan 1970.
   */
  void test7() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    apr_time_t ticks = MICROSECONDS_PER_DAY * -7;
    assertFormattedTime(ticks, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,000"));
#if defined(_WIN32)
    //
    //   These tests fail on Unix due to bug in APR's explode_time
    //
//    assertFormattedTime(ticks + 8000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,008"));
//    assertFormattedTime(ticks + 17000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,017"));
//    assertFormattedTime(ticks + 237000, TimeZone::getGMT(), LOG4CXX_STR("00:00:00,237"));
//    assertFormattedTime(ticks + 1415000, TimeZone::getGMT(), LOG4CXX_STR("00:00:01,415"));
#endif
  }

  /**
   * Checks that numberFormat works as expected.
   */
  void test8() {
    Pool p;
    LogString numb;
    AbsoluteTimeDateFormat formatter;
    formatter.numberFormat(numb, 87, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("87"), numb);
  }

};

LOGUNIT_TEST_SUITE_REGISTRATION(AbsoluteTimeDateFormatTestCase);

