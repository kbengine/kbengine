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

#if defined(_WIN32) && !defined(_WIN32_WCE)
#include <log4cxx/nt/nteventlogappender.h>
#include "../appenderskeletontestcase.h"
#include "windows.h"
#include <log4cxx/logger.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/patternlayout.h>
#include "../insertwide.h"
#include "../logunit.h"
#include <log4cxx/helpers/date.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::nt;
using namespace log4cxx::spi;

/**
   Unit tests of log4cxx::nt::NTEventLogAppender
 */
class NTEventLogAppenderTestCase : public AppenderSkeletonTestCase
{
   LOGUNIT_TEST_SUITE(NTEventLogAppenderTestCase);
                //
                //    tests inherited from AppenderSkeletonTestCase
                //
                LOGUNIT_TEST(testDefaultThreshold);
                LOGUNIT_TEST(testSetOptionThreshold);
                LOGUNIT_TEST(testHelloWorld);

   LOGUNIT_TEST_SUITE_END();


public:

        AppenderSkeleton* createAppenderSkeleton() const {
          return new log4cxx::nt::NTEventLogAppender();
        }

        void testHelloWorld() {
           DWORD expectedId = 1;
           HANDLE hEventLog = ::OpenEventLogW(NULL, L"log4cxx_test");
           if (hEventLog != NULL) {
               BOOL stat = GetNumberOfEventLogRecords(hEventLog, &expectedId);
               DWORD oldest;
               if(stat) stat = GetOldestEventLogRecord(hEventLog, &oldest);
               CloseEventLog(hEventLog);
               LOGUNIT_ASSERT(stat);
               expectedId += oldest;
           }


            Pool p;
            Date now;
            DWORD expectedTime = now.getTime() / Date::getMicrosecondsPerSecond();
            {
                NTEventLogAppenderPtr appender(new NTEventLogAppender());
                appender->setSource(LOG4CXX_STR("log4cxx_test"));
                LayoutPtr layout(new PatternLayout(LOG4CXX_STR("%c - %m%n")));
                appender->setLayout(layout);
                appender->activateOptions(p);

                LoggingEventPtr event(new LoggingEvent(
                    LOG4CXX_STR("org.foobar"), Level::getInfo(), LOG4CXX_STR("Hello,  World"), LOG4CXX_LOCATION));
                appender->doAppend(event, p);
            }
            hEventLog = ::OpenEventLogW(NULL, L"log4cxx_test");
            LOGUNIT_ASSERT(hEventLog != NULL);
            DWORD actualId;
            BOOL stat = GetNumberOfEventLogRecords(hEventLog, &actualId);
            DWORD oldest;
            if (stat) stat = GetOldestEventLogRecord(hEventLog, &oldest);
            actualId += oldest;
            actualId--;
            CloseEventLog(hEventLog);
            LOGUNIT_ASSERT(stat);
            LOGUNIT_ASSERT_EQUAL(expectedId, actualId);
        }
};

LOGUNIT_TEST_SUITE_REGISTRATION(NTEventLogAppenderTestCase);
#endif
