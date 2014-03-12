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

#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/datetimedateformat.h>
#include "../logunit.h"
#include <log4cxx/helpers/pool.h>
#include "../insertwide.h"
#include <apr.h>
#include <apr_time.h>
#include <sstream>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

#define LOCALE_US "C"
#if defined(_WIN32)
#define LOCALE_FR "French_france"
#else
#define LOCALE_FR "fr_FR"
#endif

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>


#if LOG4CXX_HAS_STD_LOCALE
#include <locale>
#include "localechanger.h"
#define MAKE_LOCALE(ptr, id)     \
std::locale loco(id);            \
std::locale* ptr = &loco;
#else
#define MAKE_LOCALE(ptr, id)     \
std::locale* ptr = NULL;
#endif


/**
   Unit test {@link DateTimeDateFormat}.
   
   
*/
LOGUNIT_CLASS(DateTimeDateFormatTestCase)
{
  LOGUNIT_TEST_SUITE( DateTimeDateFormatTestCase );
  LOGUNIT_TEST( test1 );
  LOGUNIT_TEST( test2 );
  LOGUNIT_TEST( test3 );
  LOGUNIT_TEST( test4 );
  LOGUNIT_TEST( test5 );
  LOGUNIT_TEST( test6 );
#if LOG4CXX_HAS_STD_LOCALE
  LOGUNIT_TEST( test7 );
  LOGUNIT_TEST( test8 );
#endif
  LOGUNIT_TEST_SUITE_END();



private:

#define MICROSECONDS_PER_DAY APR_INT64_C(86400000000)


  /**
   Asserts that formatting the provided date results in the expected string.

  @param date Date date
   @param timeZone TimeZone timezone for conversion
   @param expected String expected string
  */
  void assertFormattedTime( apr_time_t date, const std::locale* locale,
       const TimeZonePtr& timeZone, const LogString& expected )
       {
         DateTimeDateFormat formatter(locale);
         formatter.setTimeZone(timeZone);
         LogString actual;
         Pool p;
         formatter.format(actual, date, p);
         LOGUNIT_ASSERT_EQUAL( expected, actual );
  }
public:

  /** Convert 02 Jan 2004 00:00:00 GMT for GMT. */
  void test1()
  {
    //
    //   02 Jan 2004 00:00 GMT
    //
    apr_time_t jan2 = MICROSECONDS_PER_DAY * 12419;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( jan2, localeUS, TimeZone::getGMT(), LOG4CXX_STR("02 Jan 2004 00:00:00,000"));
  }

  /** Convert 03 Jan 2004 00:00:00 GMT for America/Chicago. */
  void test2()
  {
    //
    //   03 Jan 2004 00:00 GMT
    apr_time_t jan3 = MICROSECONDS_PER_DAY * 12420;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( jan3, localeUS,
             TimeZone::getTimeZone(LOG4CXX_STR("GMT-6")),
             LOG4CXX_STR("02 Jan 2004 18:00:00,000"));
  }


  /** Convert 30 Jun 2004 00:00:00 GMT for GMT. */
  void test3()
  {
    apr_time_t jun30 = MICROSECONDS_PER_DAY * 12599;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( jun30, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("30 Jun 2004 00:00:00,000"));
  }

  /** Convert 29 Jun 2004 00:00:00 GMT for Chicago, daylight savings in effect. */
  void test4()
  {
    apr_time_t jul1 = MICROSECONDS_PER_DAY * 12600;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( jul1, localeUS,
           TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")),
           LOG4CXX_STR("30 Jun 2004 19:00:00,000"));
  }

  /** Test multiple calls in close intervals. */
  void test5()
  {
    //   subsequent calls within one minute
    //     are optimized to reuse previous formatted value
    //     make a couple of nearly spaced calls
    apr_time_t ticks = MICROSECONDS_PER_DAY * 12601;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( ticks, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("02 Jul 2004 00:00:00,000"));
    assertFormattedTime( ticks + 8000, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("02 Jul 2004 00:00:00,008"));
    assertFormattedTime( ticks + 17000, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("02 Jul 2004 00:00:00,017"));
    assertFormattedTime( ticks + 237000, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("02 Jul 2004 00:00:00,237"));
    assertFormattedTime( ticks + 1415000, localeUS, TimeZone::getGMT(),
           LOG4CXX_STR("02 Jul 2004 00:00:01,415"));
  }

  /** Check that caching does not disregard timezone. This test would fail for revision 1.4 of DateTimeDateFormat.java. */
  void test6()
  {
    apr_time_t jul3 = MICROSECONDS_PER_DAY * 12602;
    MAKE_LOCALE(localeUS, LOCALE_US);
    assertFormattedTime( jul3, localeUS, TimeZone::getGMT(),
        LOG4CXX_STR("03 Jul 2004 00:00:00,000"));
    assertFormattedTime( jul3, localeUS,
          TimeZone::getTimeZone(LOG4CXX_STR("GMT-5")),
          LOG4CXX_STR("02 Jul 2004 19:00:00,000"));
    assertFormattedTime( jul3, localeUS, TimeZone::getGMT(),
          LOG4CXX_STR("03 Jul 2004 00:00:00,000"));
  }

#if LOG4CXX_HAS_STD_LOCALE
  LogString formatDate(const std::locale& locale, const tm& date, const LogString& fmt) {
        //
        //  output the using STL
        //
        std::basic_ostringstream<logchar> buffer;
#if defined(_USEFAC)
         _USEFAC(locale, std::time_put<logchar>)
             .put(buffer, buffer, &date, fmt.c_str(), fmt.c_str() + fmt.length());
#else
#if defined(_RWSTD_NO_TEMPLATE_ON_RETURN_TYPE)
         const std::time_put<logchar>& facet = std::use_facet(locale, (std::time_put<logchar>*) 0);
#else
         const std::time_put<logchar>& facet = std::use_facet<std::time_put<logchar> >(locale);
#endif
         facet.put(buffer, buffer, buffer.fill(), &date, fmt.c_str(), fmt.c_str() + fmt.length());
#endif
        return buffer.str();
  }

  /** Check that format is locale sensitive. */
  void test7()
  {
    apr_time_t avr11 = MICROSECONDS_PER_DAY * 12519;
    LocaleChanger localeChange(LOCALE_FR);
    if (localeChange.isEffective()) {
        LogString formatted;
        Pool p;
        SimpleDateFormat formatter(LOG4CXX_STR("MMM"));
        formatter.format(formatted, avr11, p);

        std::locale localeFR(LOCALE_FR);
        struct tm avr11tm = { 0, 0, 0, 11, 03, 104 };
        LogString expected(formatDate(localeFR, avr11tm, LOG4CXX_STR("%b")));

        LOGUNIT_ASSERT_EQUAL(expected, formatted);
    }
  }

  /** Check that format is locale sensitive. */
  void test8()
  {
    apr_time_t apr11 = MICROSECONDS_PER_DAY * 12519;
    LocaleChanger localeChange(LOCALE_US);
    if (localeChange.isEffective()) {
        LogString formatted;
        Pool p;
        SimpleDateFormat formatter(LOG4CXX_STR("MMM"));
        formatter.setTimeZone(TimeZone::getGMT());
        formatter.format(formatted, apr11, p);

        std::locale localeUS(LOCALE_US);
        struct tm apr11tm = { 0, 0, 0, 11, 03, 104 };
        LogString expected(formatDate(localeUS, apr11tm, LOG4CXX_STR("%b")));

        LOGUNIT_ASSERT_EQUAL(expected, formatted);
    }
  }
#endif

};

LOGUNIT_TEST_SUITE_REGISTRATION(DateTimeDateFormatTestCase);

