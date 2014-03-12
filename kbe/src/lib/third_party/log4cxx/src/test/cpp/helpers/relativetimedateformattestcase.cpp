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
#include <log4cxx/helpers/relativetimedateformat.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/stringhelper.h>
#include "../insertwide.h"
#include "../logunit.h"
#include <log4cxx/helpers/date.h>



using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;


/**
   Unit test {@link RelativeTimeDateFormat} class.
   
   */
LOGUNIT_CLASS(RelativeTimeDateFormatTestCase) {
     LOGUNIT_TEST_SUITE(RelativeTimeDateFormatTestCase);
             LOGUNIT_TEST(test1);
             LOGUNIT_TEST(test2);
             LOGUNIT_TEST(test3);
     LOGUNIT_TEST_SUITE_END();


  public:

  /**
  *   Convert 2 Jan 2004
  */
  void test1() {
    log4cxx_time_t jan2 = Date::getMicrosecondsPerDay() * 12419;
    log4cxx_time_t preStartTime = LoggingEvent::getStartTime();

    RelativeTimeDateFormat formatter;

    Pool p;

    LogString actual;

    formatter.format(actual, jan2, p);

    log4cxx_time_t elapsed = log4cxx::helpers::StringHelper::toInt64(actual);


    LOGUNIT_ASSERT(preStartTime + elapsed*1000 > jan2 - 2000);
    LOGUNIT_ASSERT(preStartTime + elapsed*1000 < jan2 + 2000);
  }


    /**
     * Checks that numberFormat works as expected.
     */
    void test2() {
      LogString numb;
      Pool p;
      RelativeTimeDateFormat formatter;
      formatter.numberFormat(numb, 87, p);
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("87"), numb);
    }


  /**
   * Checks that setting timezone doesn't throw an exception.
   */
  void test3() {
    RelativeTimeDateFormat formatter;
    formatter.setTimeZone(TimeZone::getGMT());
  }

};

LOGUNIT_TEST_SUITE_REGISTRATION(RelativeTimeDateFormatTestCase);

