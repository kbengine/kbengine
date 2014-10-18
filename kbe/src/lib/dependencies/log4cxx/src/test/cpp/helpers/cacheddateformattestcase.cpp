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

#include <log4cxx/helpers/cacheddateformat.h>
#include "../logunit.h"
#include <log4cxx/helpers/absolutetimedateformat.h>
#include <log4cxx/helpers/relativetimedateformat.h>
#include <log4cxx/helpers/pool.h>
#include <locale>
#include "../insertwide.h"
#include <apr.h>
#include <apr_time.h>
#include "localechanger.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;

#define LOCALE_US "C"
#if defined(_WIN32)
#define LOCALE_JP "Japanese_japan"
#else
#define LOCALE_JP "ja_JP"
#endif


//Define INT64_C for compilers that don't have it
#if (!defined(INT64_C))
#define INT64_C(value)  value ##LL
#endif

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>


/**
   Unit test {@link CachedDateFormat}.
   
   */
LOGUNIT_CLASS(CachedDateFormatTestCase)
   {
     LOGUNIT_TEST_SUITE( CachedDateFormatTestCase );
     LOGUNIT_TEST( test1 );
     LOGUNIT_TEST( test2 );
     LOGUNIT_TEST( test3 );
     LOGUNIT_TEST( test4 );
#if LOG4CXX_HAS_STD_LOCALE
     LOGUNIT_TEST( test5 );
#endif
     LOGUNIT_TEST( test6 );
     LOGUNIT_TEST( test8 );
//   Gump doesn't like this test
//     LOGUNIT_TEST( test9 );
     LOGUNIT_TEST( test10 );
     LOGUNIT_TEST( test11);
     LOGUNIT_TEST( test12 );
     LOGUNIT_TEST( test13 );
     LOGUNIT_TEST( test14 );
     LOGUNIT_TEST( test15 );
     LOGUNIT_TEST( test16 );
     LOGUNIT_TEST( test17);
     LOGUNIT_TEST( test18);
     LOGUNIT_TEST( test19);
     LOGUNIT_TEST( test20);
     LOGUNIT_TEST( test21);
     LOGUNIT_TEST_SUITE_END();


#define MICROSECONDS_PER_DAY APR_INT64_C(86400000000)

   public:

  /**
   * Test multiple calls in close intervals.
   */
  void test1() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    DateFormatPtr baseFormatter(new AbsoluteTimeDateFormat());
    CachedDateFormat gmtFormat(baseFormatter, 1000000);
    gmtFormat.setTimeZone(TimeZone::getGMT());

    apr_time_t jul1 = MICROSECONDS_PER_DAY * 12601L;
    Pool p;

    LogString actual;

    gmtFormat.format(actual, jul1, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,000"), actual);
    actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, jul1 + 8000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,008"), actual);
    actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, jul1 + 17000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,017"), actual);
    actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, jul1 + 237000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,237"), actual);
    actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, jul1 + 1415000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:01,415"), actual);
    actual.erase(actual.begin(), actual.end());

  }

  /**
   *  Check for interaction between caches.
   */

  void test2() {
      apr_time_t jul2 = MICROSECONDS_PER_DAY * 12602;
      DateFormatPtr baseFormatter(new AbsoluteTimeDateFormat());
      CachedDateFormat gmtFormat(baseFormatter, 1000000);
      gmtFormat.setTimeZone(TimeZone::getGMT());

     DateFormatPtr chicagoBase(new AbsoluteTimeDateFormat());
     CachedDateFormat chicagoFormat(chicagoBase, 1000000);
     chicagoFormat.setTimeZone(TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")));

     Pool p;
     LogString actual;
     gmtFormat.format(actual, jul2, p);
     LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,000"), actual);

     actual.erase(actual.begin(), actual.end());
     chicagoFormat.format(actual, jul2, p);
     LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("19:00:00,000"), actual);

      actual.erase(actual.begin(), actual.end());
      gmtFormat.format(actual, jul2, p);
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,000"), actual);
  }

  /**
   * Test multiple calls in close intervals prior to 1 Jan 1970.
   */
  void test3() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    DateFormatPtr baseFormatter(new AbsoluteTimeDateFormat());
    CachedDateFormat gmtFormat(baseFormatter, 1000000);
    gmtFormat.setTimeZone(TimeZone::getGMT());

    apr_time_t ticks = MICROSECONDS_PER_DAY * -7;

    Pool p;

    LogString actual;


    gmtFormat.format(actual, ticks, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,000"), actual);
    actual.erase(actual.begin(), actual.end());

   //
   //   APR's explode_time method does not properly calculate tm_usec
   //     prior to 1 Jan 1970 on Unix
   gmtFormat.format(actual, ticks + 8000, p);
   LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,008"), actual);
   actual.erase(actual.begin(), actual.end());

   gmtFormat.format(actual, ticks + 17000, p);
   LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,017"), actual);
   actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, ticks + 237000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,237"), actual);
    actual.erase(actual.begin(), actual.end());

    gmtFormat.format(actual, ticks + 1423000, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:01,423"), actual);
  }

  void assertFormattedEquals(
      const DateFormatPtr& baseFormat,
      const CachedDateFormat& cachedFormat,
      apr_time_t date,
      Pool& p) {
        LogString expected;
        LogString actual;

        baseFormat->format(expected, date, p);
        cachedFormat.format(actual, date, p);


        LOGUNIT_ASSERT_EQUAL(expected, actual);
  }

  void test4() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
#if LOG4CXX_HAS_STD_LOCALE
    std::locale loco(LOCALE_US);
    std::locale* localeEN = &loco;
#else
    std::locale* localeEN = NULL;
#endif
    DateFormatPtr baseFormat(
         new SimpleDateFormat(LOG4CXX_STR("EEE, MMM dd, HH:mm:ss.SSS Z"), localeEN));
    CachedDateFormat cachedFormat(baseFormat, 1000000);
    //
    //   use a date in 2000 to attempt to confuse the millisecond locator
    apr_time_t ticks = MICROSECONDS_PER_DAY * 11141;

    Pool p;

    assertFormattedEquals(baseFormat, cachedFormat, ticks, p);
    assertFormattedEquals(baseFormat, cachedFormat, ticks + 8000, p);
    assertFormattedEquals(baseFormat, cachedFormat, ticks + 17000, p);
    assertFormattedEquals(baseFormat, cachedFormat, ticks + 237000, p);
    assertFormattedEquals(baseFormat, cachedFormat, ticks + 1415000, p);
  }


#if LOG4CXX_HAS_STD_LOCALE
  void test5() {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    LocaleChanger localeChange(LOCALE_JP);
    if (localeChange.isEffective()) {
        DateFormatPtr baseFormat(new SimpleDateFormat(
                   LOG4CXX_STR("EEE, MMM dd, HH:mm:ss.SSS Z")));
        CachedDateFormat cachedFormat(baseFormat, 1000000);
        //
        //   use a date in 2000 to attempt to confuse the millisecond locator
        apr_time_t ticks = MICROSECONDS_PER_DAY * 11141;

        Pool p;

        assertFormattedEquals(baseFormat, cachedFormat, ticks, p);
        assertFormattedEquals(baseFormat, cachedFormat, ticks + 8000, p);
        assertFormattedEquals(baseFormat, cachedFormat, ticks + 17000, p);
        assertFormattedEquals(baseFormat, cachedFormat, ticks + 237000, p);
        assertFormattedEquals(baseFormat, cachedFormat, ticks + 1415000, p);
    }
  }
#endif

  /**
   * Checks that numberFormat works as expected.
   */
  void test6() {
    LogString numb;
    Pool p;
    AbsoluteTimeDateFormat formatter;
    formatter.numberFormat(numb, 87, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("87"), numb);
  }


  /**
   * Set time zone on cached and check that it is effective.
   */
  void test8() {
    DateFormatPtr baseFormat(new SimpleDateFormat(LOG4CXX_STR("yyyy-MM-dd HH:mm:ss,SSS")));
    baseFormat->setTimeZone(TimeZone::getGMT());
    CachedDateFormat cachedFormat(baseFormat, 1000000);
    apr_time_t jul4 = MICROSECONDS_PER_DAY * 12603;

    Pool p;

    LogString actual;
    cachedFormat.format(actual, jul4, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("2004-07-04 00:00:00,000"), actual);

    cachedFormat.setTimeZone(TimeZone::getTimeZone(LOG4CXX_STR("GMT-6")));
    actual.erase(actual.begin(), actual.end());
    cachedFormat.format(actual, jul4, p);

    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("2004-07-03 18:00:00,000"), actual);
  }


/**
 * Test of caching when less than three millisecond digits are specified.
 */
void test9() {
  std::locale localeUS(LOCALE_US);

  DateFormatPtr baseFormat = new SimpleDateFormat(
      LOG4CXX_STR("yyyy-MMMM-dd HH:mm:ss,SS Z"), &localeUS);
  DateFormatPtr cachedFormat = new CachedDateFormat(baseFormat, 1000000);
  TimeZonePtr cet = TimeZone::getTimeZone(LOG4CXX_STR("GMT+1"));
  cachedFormat->setTimeZone(cet);


   apr_time_exp_t c;
   memset(&c, 0, sizeof(c));
   c.tm_year = 104;
   c.tm_mon = 11;
   c.tm_mday = 12;
   c.tm_hour = 19;
   c.tm_sec = 37;
   c.tm_usec = 23000;

   apr_time_t dec12;
   apr_status_t stat = apr_time_exp_gmt_get(&dec12, &c);
   const apr_status_t statOK = 0;
   LOGUNIT_ASSERT_EQUAL(statOK, stat);

   Pool p;

   LogString s;
   cachedFormat->format(s, dec12, p);

   LOGUNIT_ASSERT_EQUAL(
       (LogString) LOG4CXX_STR("2004-December-12 20:00:37,23 +0100"), s);

    memset(&c, 0, sizeof(c));
    c.tm_year = 104;
    c.tm_mon = 11;
    c.tm_mday = 31;
    c.tm_hour = 23;
    c.tm_sec = 13;
    c.tm_usec = 905000;

    apr_time_t jan1;
    stat = apr_time_exp_gmt_get(&jan1, &c);
    LOGUNIT_ASSERT_EQUAL(statOK, stat);

    s.erase(s.begin(), s.end());
    cachedFormat->format(s, jan1, p);

    LOGUNIT_ASSERT_EQUAL(
       (LogString) LOG4CXX_STR("2005-January-01 00:00:13,905 +0100"), s);
}


/**
 * Test when millisecond position moves but length remains constant.
 */
void test10() {
#if LOG4CXX_HAS_STD_LOCALE
  std::locale loco("C");
  std::locale* localeUS = &loco;
#else
  std::locale* localeUS = NULL;
#endif
  DateFormatPtr baseFormat = new SimpleDateFormat(
      LOG4CXX_STR("MMMM SSS EEEEEE"), localeUS);
  DateFormatPtr cachedFormat = new CachedDateFormat(baseFormat, 1000000);
  TimeZonePtr cet = TimeZone::getTimeZone(LOG4CXX_STR("GMT+1"));
  cachedFormat->setTimeZone(cet);

  apr_time_exp_t c;
  memset(&c, 0, sizeof(c));
  c.tm_year = 104;
  c.tm_mon = 9;
  c.tm_mday = 5;
  c.tm_hour = 21;
  c.tm_sec = 37;
  c.tm_usec = 23000;

  apr_time_t oct5;
  apr_status_t stat = apr_time_exp_gmt_get(&oct5, &c);
  const apr_status_t statOK = 0;
  LOGUNIT_ASSERT_EQUAL(statOK, stat);

  Pool p;

  LogString s;
  cachedFormat->format(s, oct5, p);

  LOGUNIT_ASSERT_EQUAL(
    (LogString) LOG4CXX_STR("October 023 Tuesday"), s);

  memset(&c, 0, sizeof(c));
  c.tm_year = 104;
  c.tm_mon = 10;
  c.tm_mday = 1;
  c.tm_usec = 23000;

  apr_time_t nov1;
  stat = apr_time_exp_gmt_get(&nov1, &c);
  LOGUNIT_ASSERT_EQUAL(statOK, stat);

  s.erase(s.begin(), s.end());
  cachedFormat->format(s, nov1, p);

  LOGUNIT_ASSERT_EQUAL(
     (LogString) LOG4CXX_STR("November 023 Monday"), s);

   nov1 += 961000;
   s.erase(s.begin(), s.end());
   cachedFormat->format(s, nov1, p);

   LOGUNIT_ASSERT_EQUAL(
      (LogString) LOG4CXX_STR("November 984 Monday"), s);
}

/**
 * Test that tests if caching is skipped if only "SS"
 *     is specified.
 */
void test11() {
   //
   //   Earlier versions could be tricked by "SS0" patterns.
   //
   LogString badPattern(LOG4CXX_STR("ss,SS0"));
   DateFormatPtr simpleFormat = new SimpleDateFormat(badPattern);
   DateFormatPtr gmtFormat = new CachedDateFormat(simpleFormat, 1000000);
   gmtFormat->setTimeZone(TimeZone::getGMT());

   //
   // The first request has to 100 ms after an ordinal second
   //    to push the literal zero out of the pattern check
   apr_time_t ticks = MICROSECONDS_PER_DAY * 11142L;
   apr_time_t jul2 = ticks + 120000;

   Pool p;

   LogString s;
   gmtFormat->format(s, jul2, p);

   LOGUNIT_ASSERT_EQUAL(
      (LogString) LOG4CXX_STR("00,1200"), s);

   jul2 = ticks + 87000;

   s.erase(s.begin(), s.end());
   gmtFormat->format(s, jul2, p);

   LOGUNIT_ASSERT_EQUAL(
      (LogString) LOG4CXX_STR("00,870"), s);
}

/**
 * Check pattern location for ISO8601
 */
void test12() {
   DateFormatPtr df = new SimpleDateFormat(LOG4CXX_STR("yyyy-MM-dd HH:mm:ss,SSS"));
   apr_time_t ticks = 11142L * MICROSECONDS_PER_DAY;

   Pool p;

   LogString formatted;
   df->format(formatted, ticks, p);

   int millisecondStart = CachedDateFormat::findMillisecondStart(ticks,
       formatted, df, p);
   LOGUNIT_ASSERT_EQUAL(20, millisecondStart);
}

/**
 * Check pattern location for DATE
 */
void test13() {
   DateFormatPtr df = new SimpleDateFormat(LOG4CXX_STR("yyyy-MM-dd"));
   apr_time_t ticks = 11142L * MICROSECONDS_PER_DAY;

   Pool p;

   LogString formatted;
   df->format(formatted, ticks, p);

   int millisecondStart = CachedDateFormat::findMillisecondStart(ticks,
       formatted, df, p);
   LOGUNIT_ASSERT_EQUAL((int) CachedDateFormat::NO_MILLISECONDS, millisecondStart);
}

/**
 * Check pattern location for ABSOLUTE
 */
void test14() {
   DateFormatPtr df = new SimpleDateFormat(LOG4CXX_STR("HH:mm:ss,SSS"));
   apr_time_t ticks = 11142L * MICROSECONDS_PER_DAY;

   Pool p;
   LogString formatted;
   df->format(formatted, ticks, p);

   int millisecondStart = CachedDateFormat::findMillisecondStart(ticks,
      formatted, df, p);
   LOGUNIT_ASSERT_EQUAL(9, millisecondStart);
}

/**
 * Check pattern location for single S
 */
void test15() {
   DateFormatPtr df = new SimpleDateFormat(LOG4CXX_STR("HH:mm:ss,S"));
   apr_time_t ticks = 11142L * MICROSECONDS_PER_DAY;

   Pool p;
   LogString formatted;
   df->format(formatted, ticks, p);

   int millisecondStart = CachedDateFormat::findMillisecondStart(ticks,
      formatted, df, p);
   LOGUNIT_ASSERT_EQUAL((int) CachedDateFormat::UNRECOGNIZED_MILLISECONDS, millisecondStart);
}

/**
 * Check pattern location for single SS
 */
void test16() {
   DateFormatPtr df = new SimpleDateFormat(LOG4CXX_STR("HH:mm:ss,SS"));
   apr_time_t ticks = 11142L * MICROSECONDS_PER_DAY;

   Pool p;
   LogString formatted;
   df->format(formatted, ticks, p);

   int millisecondStart =
      CachedDateFormat::findMillisecondStart(ticks, formatted, df, p);
   LOGUNIT_ASSERT_EQUAL((int) CachedDateFormat::UNRECOGNIZED_MILLISECONDS, millisecondStart);
}


/**
 * Check caching when multiple SSS appear in pattern
 */
void test17() {
    apr_time_t jul2 = 12602L * MICROSECONDS_PER_DAY;
    LogString badPattern(LOG4CXX_STR("HH:mm:ss,SSS HH:mm:ss,SSS"));
    DateFormatPtr simpleFormat = new SimpleDateFormat(badPattern);
    simpleFormat->setTimeZone(TimeZone::getGMT());
    DateFormatPtr cachedFormat = new CachedDateFormat(simpleFormat, 1000000);

    Pool p;
    LogString s;
    cachedFormat->format(s, jul2, p);

    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,000 00:00:00,000"), s);
    jul2 += 120000;

    s.erase(s.begin(), s.end());
    simpleFormat->format(s, jul2, p);
    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,120 00:00:00,120"), s);

    s.erase(s.begin(), s.end());
    cachedFormat->format(s, jul2, p);

    LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("00:00:00,120 00:00:00,120"), s) ;

    int maxValid = CachedDateFormat::getMaximumCacheValidity(badPattern);
    LOGUNIT_ASSERT_EQUAL(1000, maxValid);
}

/**
 * Check that patterns not containing microseconds
 * are reported as being able to be cached for a full second.
 */
void test18() {

    int maxValid =
       CachedDateFormat::getMaximumCacheValidity(
          LOG4CXX_STR("yyyy-MM-dd"));
    LOGUNIT_ASSERT_EQUAL(1000000, maxValid);
}

/**
 * Check that patterns not containing 3 microseconds
 * are reported as being able to be cached for a full second.
 */
void test19() {

    int maxValid =
       CachedDateFormat::getMaximumCacheValidity(
          LOG4CXX_STR("yyyy-MM-dd SSS"));
    LOGUNIT_ASSERT_EQUAL(1000000, maxValid);
}

/**
 * Check that patterns not containing 2 S's
 * are reported as being able to be cached for only a millisecond.
 */
void test20() {

    int maxValid =
       CachedDateFormat::getMaximumCacheValidity(
          LOG4CXX_STR("yyyy-MM-dd SS"));
    LOGUNIT_ASSERT_EQUAL(1000, maxValid);
}

/**
 * Check that patterns not containing multi S groups
 * are reported as being able to be cached for only a millisecond.
 */
void test21() {

    int maxValid =
       CachedDateFormat::getMaximumCacheValidity(
          LOG4CXX_STR("yyyy-MM-dd SSS SSS"));
    LOGUNIT_ASSERT_EQUAL(1000, maxValid);
}

};


LOGUNIT_TEST_SUITE_REGISTRATION(CachedDateFormatTestCase);
