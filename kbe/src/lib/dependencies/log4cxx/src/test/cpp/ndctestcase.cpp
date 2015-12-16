
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

#include <log4cxx/ndc.h>
#include <log4cxx/file.h>
#include <log4cxx/logger.h>
#include <log4cxx/propertyconfigurator.h>
#include "insertwide.h"
#include "logunit.h"
#include "util/compare.h"



using namespace log4cxx;

LOGUNIT_CLASS(NDCTestCase)
{
         static File TEMP;
         static LoggerPtr logger;

        LOGUNIT_TEST_SUITE(NDCTestCase);
                LOGUNIT_TEST(testPushPop);
                LOGUNIT_TEST(test1);
                LOGUNIT_TEST(testInherit);
        LOGUNIT_TEST_SUITE_END();

public:

        void setUp() {
        }

        void tearDown() {
            logger->getLoggerRepository()->resetConfiguration();
        }

        /**
         *   Push and pop a value from the NDC
         */
        void testPushPop()
        {
                NDC::push("trivial context");
                LogString actual(NDC::pop());
                LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("trivial context"), actual);
        }


        void test1()  {
            PropertyConfigurator::configure(File("input/ndc/NDC1.properties"));
            common();
            LOGUNIT_ASSERT(Compare::compare(TEMP, File("witness/ndc/NDC.1")));
        }

        static void common() {
            commonLog();
            NDC::push("n1");
            commonLog();
            NDC::push("n2");
            NDC::push("n3");
            commonLog();
            NDC::pop();
            commonLog();
            NDC::clear();
            commonLog();
        }

        static void commonLog() {
            LOG4CXX_DEBUG(logger, "m1");
            LOG4CXX_INFO(logger, "m2");
            LOG4CXX_WARN(logger, "m3");
            LOG4CXX_ERROR(logger, "m4");
            LOG4CXX_FATAL(logger, "m5");
        }
        
        void testInherit() {
           NDC::push("hello");
           NDC::push("world");
           NDC::Stack* clone = NDC::cloneStack();
           NDC::clear();
           NDC::push("discard");
           NDC::inherit(clone);
           LogString expected1(LOG4CXX_STR("world"));
           LOGUNIT_ASSERT_EQUAL(expected1, NDC::pop());
           LogString expected2(LOG4CXX_STR("hello"));
           LOGUNIT_ASSERT_EQUAL(expected2, NDC::pop());
           LogString expected3;
           LOGUNIT_ASSERT_EQUAL(expected3, NDC::pop());
        }

};


File NDCTestCase::TEMP("output/temp");
LoggerPtr NDCTestCase::logger(Logger::getLogger("org.apache.log4j.NDCTestCase"));

LOGUNIT_TEST_SUITE_REGISTRATION(NDCTestCase);
