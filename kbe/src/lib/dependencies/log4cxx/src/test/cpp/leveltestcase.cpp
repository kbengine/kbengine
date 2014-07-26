
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

#include <log4cxx/level.h>
#include "testchar.h"
#include "logunit.h"

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#endif

using namespace log4cxx;

LOGUNIT_CLASS(LevelTestCase)
{
        LOGUNIT_TEST_SUITE(LevelTestCase);
                LOGUNIT_TEST(testToLevelFatal);
                LOGUNIT_TEST(testTraceInt);
                LOGUNIT_TEST(testTrace);
                LOGUNIT_TEST(testIntToTrace);
                LOGUNIT_TEST(testStringToTrace);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(testWideStringToTrace);
#endif                
#if LOG4CXX_UNICHAR_API
                LOGUNIT_TEST(testUniCharStringToTrace);
#endif                
#if LOG4CXX_CFSTRING_API
                LOGUNIT_TEST(testCFStringToTrace);
#endif                
        LOGUNIT_TEST_SUITE_END();

public:
        void testToLevelFatal()
        {
                LevelPtr level(Level::toLevel(LOG4CXX_TEST_STR("fATal")));
                LOGUNIT_ASSERT_EQUAL((int) Level::FATAL_INT, level->toInt());
        }
        
    /**
     * Tests Level::TRACE_INT.
     */
  void testTraceInt() {
      LOGUNIT_ASSERT_EQUAL(5000, (int) Level::TRACE_INT);
  }

    /**
     * Tests Level.TRACE.
     */
  void testTrace() {
      LOGUNIT_ASSERT(Level::getTrace()->toString() == LOG4CXX_STR("TRACE"));
      LOGUNIT_ASSERT_EQUAL(5000, Level::getTrace()->toInt());
      LOGUNIT_ASSERT_EQUAL(7, Level::getTrace()->getSyslogEquivalent());
  }

    /**
     * Tests Level.toLevel(Level.TRACE_INT).
     */
  void testIntToTrace() {
      LevelPtr trace(Level::toLevel(5000));
      LOGUNIT_ASSERT(trace->toString() == LOG4CXX_STR("TRACE"));
  }

    /**
     * Tests Level.toLevel("TRACE");
     */
  void testStringToTrace() {
        LevelPtr trace(Level::toLevel("TRACE"));
      LOGUNIT_ASSERT(trace->toString() == LOG4CXX_STR("TRACE"));
  }

#if LOG4CXX_WCHAR_T_API
    /**
     * Tests Level.toLevel(L"TRACE");
     */
  void testWideStringToTrace() {
        LevelPtr trace(Level::toLevel(L"TRACE"));
        LOGUNIT_ASSERT(trace->toString() == LOG4CXX_STR("TRACE"));
  }
#endif  

#if LOG4CXX_UNICHAR_API
    /**
     * Tests Level.toLevel("TRACE");
     */
  void testUniCharStringToTrace() {
        const log4cxx::UniChar name[] = { 'T', 'R', 'A', 'C', 'E', 0 };
        LevelPtr trace(Level::toLevel(name));
        LOGUNIT_ASSERT(trace->toString() == LOG4CXX_STR("TRACE"));
  }
#endif  

#if LOG4CXX_CFSTRING_API
    /**
     * Tests Level.toLevel(CFSTR("TRACE"));
     */
  void testCFStringToTrace() {
        LevelPtr trace(Level::toLevel(CFSTR("TRACE")));
        LOGUNIT_ASSERT(trace->toString() == LOG4CXX_STR("TRACE"));
  }
#endif  
        

};

LOGUNIT_TEST_SUITE_REGISTRATION(LevelTestCase);
