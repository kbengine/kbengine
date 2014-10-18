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

#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/fileinputstream.h>
#include "../insertwide.h"
#include "../logunit.h"

using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(PropertiesTestCase)
{
        LOGUNIT_TEST_SUITE(PropertiesTestCase);
                LOGUNIT_TEST(testLoad1);
        LOGUNIT_TEST_SUITE_END();

public:
        void testLoad1() {
          //
          //    read patternLayout1.properties
          FileInputStreamPtr propFile = 
            new FileInputStream(LOG4CXX_STR("input/patternLayout1.properties"));
          Properties properties;
          properties.load(propFile);
          LogString pattern(properties.getProperty(LOG4CXX_STR("log4j.appender.testAppender.layout.ConversionPattern")));
          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("%-5p - %m%n"), pattern);
        }
};


LOGUNIT_TEST_SUITE_REGISTRATION(PropertiesTestCase);
