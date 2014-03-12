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
#include <log4cxx/spi/loggingevent.h>
#include "../util/serializationtesthelper.h"
#include <log4cxx/logmanager.h>
#include <log4cxx/ndc.h>
#include <log4cxx/mdc.h>
#include "../logunit.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::util;
using namespace log4cxx::spi;
using namespace std;


/**
   Unit tests for LoggingEvent
 */
LOGUNIT_CLASS(LoggingEventTest)
{
        LOGUNIT_TEST_SUITE(LoggingEventTest);
                LOGUNIT_TEST(testSerializationSimple);
                LOGUNIT_TEST(testSerializationWithLocation);
                LOGUNIT_TEST(testSerializationNDC);
                LOGUNIT_TEST(testSerializationMDC);
         LOGUNIT_TEST_SUITE_END();

public:
        void setUp() {
            NDC::clear();
            MDC::clear();
        }

        void tearDown()
        {
            LogManager::shutdown();
        }
        
        
  
        
  /**
   * Serialize a simple logging event and check it against
   * a witness.
   * @throws Exception if exception during test.
   */
  void testSerializationSimple() {
    LoggingEventPtr event =
      new LoggingEvent(
        LOG4CXX_STR("root"), Level::getInfo(), LOG4CXX_STR("Hello, world."), LocationInfo::getLocationUnavailable());
        
    LOGUNIT_ASSERT_EQUAL(true, SerializationTestHelper::compare(
      "witness/serialization/simple.bin", event, 237));
  }


  /**
   * Serialize a logging event with an exception and check it against
   * a witness.
   * @throws Exception if exception during test.
   *
   */
  void testSerializationWithLocation() {
    LoggingEventPtr event =
      new LoggingEvent(
        LOG4CXX_STR("root"), Level::getInfo(), LOG4CXX_STR("Hello, world."), LOG4CXX_LOCATION);

    LOGUNIT_ASSERT_EQUAL(true, SerializationTestHelper::compare(
      "witness/serialization/location.bin", event, 237));
  }

  /**
   * Serialize a logging event with ndc.
   * @throws Exception if exception during test.
   *
   */
  void testSerializationNDC() {
    NDC::push("ndc test");

    LoggingEventPtr event =
      new LoggingEvent(
        LOG4CXX_STR("root"), Level::getInfo(), LOG4CXX_STR("Hello, world."), LocationInfo::getLocationUnavailable());

    LOGUNIT_ASSERT_EQUAL(true, SerializationTestHelper::compare(
      "witness/serialization/ndc.bin", event, 237));
    }

  /**
   * Serialize a logging event with mdc.
   * @throws Exception if exception during test.
   *
   */
  void testSerializationMDC() {
    MDC::put("mdckey", "mdcvalue");
 
    LoggingEventPtr event =
      new LoggingEvent(
        LOG4CXX_STR("root"), Level::getInfo(), LOG4CXX_STR("Hello, world."), LocationInfo::getLocationUnavailable());

    LOGUNIT_ASSERT_EQUAL(true, SerializationTestHelper::compare(
      "witness/serialization/mdc.bin", event, 237));
  }

};

LOGUNIT_TEST_SUITE_REGISTRATION(LoggingEventTest);
