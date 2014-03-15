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


#include <log4cxx/logger.h>
#include <log4cxx/net/socketappender.h>
#include <log4cxx/ndc.h>
#include <log4cxx/mdc.h>
#include <log4cxx/asyncappender.h>

#include "socketservertestcase.h"
#include "../util/compare.h"
#include "../util/transformer.h"
#include "../util/controlfilter.h"
#include "../util/absolutedateandtimefilter.h"
#include "../util/threadfilter.h"
#include "../util/filenamefilter.h"
#include <apr_time.h>
#include <log4cxx/file.h>
#include <iostream>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>
#include "../testchar.h"
#include "../logunit.h"
#include <log4cxx/spi/loggerrepository.h>


//Define INT64_C for compilers that don't have it
#if (!defined(INT64_C))
#define INT64_C(value)  value ## LL
#endif

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

#define REGEX_STR(x) x
// %5p %x [%t] %c %m%n
// DEBUG T1 [thread] org.apache.log4j.net.SocketAppenderTestCase Message 1
#define PAT1 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) T1 \\[0x[0-9A-F]*]\\ ") \
        REGEX_STR(".* Message [0-9]\\{1,2\\}")

// DEBUG T2 [thread] patternlayouttest.cpp(?) Message 1
#define PAT2 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) T2 \\[0x[0-9A-F]*]\\ ") \
        REGEX_STR(".*socketservertestcase.cpp\\([0-9]\\{1,4\\}\\) Message [0-9]\\{1,2\\}")

// DEBUG T3 [thread] patternlayouttest.cpp(?) Message 1
#define PAT3 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) T3 \\[0x[0-9A-F]*]\\ ") \
        REGEX_STR(".*socketservertestcase.cpp\\([0-9]\\{1,4\\}\\) Message [0-9]\\{1,2\\}")

// DEBUG some T4 MDC-TEST4 [thread] SocketAppenderTestCase - Message 1
// DEBUG some T4 MDC-TEST4 [thread] SocketAppenderTestCase - Message 1
#define PAT4 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) some T4 MDC-TEST4 \\[0x[0-9A-F]*]\\") \
        REGEX_STR(" (root|SocketServerTestCase) - Message [0-9]\\{1,2\\}")
#define PAT5 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) some5 T5 MDC-TEST5 \\[0x[0-9A-F]*]\\") \
        REGEX_STR(" (root|SocketServerTestCase) - Message [0-9]\\{1,2\\}")
#define PAT6 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) some6 T6 client-test6 MDC-TEST6") \
        REGEX_STR(" \\[0x[0-9A-F]*]\\ (root|SocketServerTestCase) - Message [0-9]\\{1,2\\}")
#define PAT7 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) some7 T7 client-test7 MDC-TEST7") \
        REGEX_STR(" \\[0x[0-9A-F]*]\\ (root|SocketServerTestCase) - Message [0-9]\\{1,2\\}")

// DEBUG some8 T8 shortSocketServer MDC-TEST7 [thread] SocketServerTestCase - Message 1
#define PAT8 \
        REGEX_STR("^(DEBUG| INFO| WARN|ERROR|FATAL|LETHAL) some8 T8 shortSocketServer") \
        REGEX_STR(" MDC-TEST8 \\[0x[0-9A-F]*]\\ (root|SocketServerTestCase) - Message [0-9]\\{1,2\\}")



/**
 *  This test checks receipt of SocketAppender messages by the ShortSocketServer
 *  class from log4j.  That class must be started externally to this class
 *  for this test to succeed. 
 */
LOGUNIT_CLASS(SocketServerTestCase)
{
        LOGUNIT_TEST_SUITE(SocketServerTestCase);
                LOGUNIT_TEST(test1);
                LOGUNIT_TEST(test2);
                LOGUNIT_TEST(test3);
                LOGUNIT_TEST(test4);
                LOGUNIT_TEST(test5);
                LOGUNIT_TEST(test6);
                LOGUNIT_TEST(test7);
                LOGUNIT_TEST(test8);
        LOGUNIT_TEST_SUITE_END();

        SocketAppenderPtr socketAppender;
        LoggerPtr logger;
        LoggerPtr root;

        class LineNumberFilter : public Filter {
        public:
                LineNumberFilter() {
                    patterns.push_back(PatternReplacement("cpp:[0-9]*", "cpp:XXX"));
                }
        };

public:
        void setUp()
        {
                logger = Logger::getLogger(LOG4CXX_STR("org.apache.log4j.net.SocketServerTestCase"));
                root = Logger::getRootLogger();
        }

        void tearDown()
        {
                socketAppender = 0;
                root->getLoggerRepository()->resetConfiguration();
                logger = 0;
                root = 0;
        }

        /**
        The pattern on the server side: %5p %x [%t] %c %m%n.

        We are testing NDC functionality across the wire.
        */
        void test1()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                root->addAppender(socketAppender1);
                common("test1", LOG4CXX_STR("T1"), LOG4CXX_STR("key1"), LOG4CXX_STR("MDC-TEST1"));
                delay(1);

                ControlFilter cf;
                cf << PAT1;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.1")));
        }

        void test2()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                root->addAppender(socketAppender1);
                common("test2", LOG4CXX_STR("T2"), LOG4CXX_STR("key2"), LOG4CXX_STR("MDC-TEST2"));
                delay(1);

                ControlFilter cf;
                cf << PAT2;

                ThreadFilter threadFilter;
                LineNumberFilter lineNumberFilter;
                LogString thisFile;
                FilenameFilter filenameFilter(__FILE__, "socketservertestcase.cpp");

                std::vector<Filter *> filters;
                filters.push_back(&filenameFilter);
                filters.push_back(&cf);
                filters.push_back(&threadFilter);
                filters.push_back(&lineNumberFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.2")));
        }

        void test3()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                root->addAppender(socketAppender1);
                common("test3", LOG4CXX_STR("T3"), LOG4CXX_STR("key3"), LOG4CXX_STR("MDC-TEST3"));
                delay(1);

                ControlFilter cf;
                cf << PAT3;

                ThreadFilter threadFilter;
                LineNumberFilter lineNumberFilter;
                LogString thisFile;
                FilenameFilter filenameFilter(__FILE__, "socketservertestcase.cpp");

                std::vector<Filter *> filters;
                filters.push_back(&filenameFilter);
                filters.push_back(&cf);
                filters.push_back(&threadFilter);
                filters.push_back(&lineNumberFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.3")));
        }

        void test4()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                root->addAppender(socketAppender1);
                NDC::push(LOG4CXX_TEST_STR("some"));
                common("test4", LOG4CXX_STR("T4"), LOG4CXX_STR("key4"), LOG4CXX_STR("MDC-TEST4"));
                NDC::pop();
                delay(1);

                ControlFilter cf;
                cf << PAT4;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.4")));
        }

        void test5()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                AsyncAppenderPtr asyncAppender = new AsyncAppender();

                root->addAppender(socketAppender1);
                root->addAppender(asyncAppender);

                NDC::push(LOG4CXX_TEST_STR("some5"));
                common("test5", LOG4CXX_STR("T5"), LOG4CXX_STR("key5"), LOG4CXX_STR("MDC-TEST5"));
                NDC::pop();
                delay(2);

                ControlFilter cf;
                cf << PAT5;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.5")));
        }

        void test6()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                AsyncAppenderPtr asyncAppender = new AsyncAppender();

                root->addAppender(socketAppender1);
                root->addAppender(asyncAppender);

                NDC::push(LOG4CXX_TEST_STR("some6"));
                MDC::put(LOG4CXX_TEST_STR("hostID"), LOG4CXX_TEST_STR("client-test6"));
                common("test6", LOG4CXX_STR("T6"), LOG4CXX_STR("key6"), LOG4CXX_STR("MDC-TEST6"));
                NDC::pop();
                MDC::remove(LOG4CXX_TEST_STR("hostID"));
                delay(2);

                ControlFilter cf;
                cf << PAT6;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.6")));
        }

        void test7()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);
                AsyncAppenderPtr asyncAppender = new AsyncAppender();

                root->addAppender(socketAppender1);
                root->addAppender(asyncAppender);

                NDC::push(LOG4CXX_TEST_STR("some7"));
                MDC::put(LOG4CXX_TEST_STR("hostID"), LOG4CXX_TEST_STR("client-test7"));
                common("test7", LOG4CXX_STR("T7"), LOG4CXX_STR("key7"), LOG4CXX_STR("MDC-TEST7"));
                NDC::pop();
                MDC::remove(LOG4CXX_TEST_STR("hostID"));
                delay(2);

                ControlFilter cf;
                cf << PAT7;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.7")));
        }

        void test8()
        {
                SocketAppenderPtr socketAppender1 =
                        new SocketAppender(LOG4CXX_STR("localhost"), PORT);

                root->addAppender(socketAppender1);

                NDC::push(LOG4CXX_TEST_STR("some8"));
                common("test8", LOG4CXX_STR("T8"), LOG4CXX_STR("key8"), LOG4CXX_STR("MDC-TEST8"));
                NDC::pop();
                delay(2);

                ControlFilter cf;
                cf << PAT8;

                ThreadFilter threadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/socketServer.8")));
        }

        void common(const std::string& testName, const LogString& dc, const LogString& key, const LogString& val)
        {
                int i = -1;
                NDC::push(dc);
                MDC::put(key, val);
                
                logger->setLevel(Level::getDebug());
                root->setLevel(Level::getDebug());

                LOG4CXX_TRACE(logger, "Message " << i);
                i++;

                logger->setLevel(Level::getTrace());
                root->setLevel(Level::getTrace());
                
                LOG4CXX_TRACE(logger, "Message " << ++i);
                LOG4CXX_TRACE(root, "Message " << ++i);

                LOG4CXX_DEBUG(logger, "Message " << ++i);
                LOG4CXX_DEBUG(root, "Message " << ++i);
                
                LOG4CXX_INFO(logger, "Message "  << ++i);
                LOG4CXX_WARN(logger, "Message " << ++i);
                LOG4CXX_FATAL(logger, "Message " << ++i); //5
                
                std::string exceptionMsg("\njava.lang.Exception: Just testing\n"
                    "\tat org.apache.log4j.net.SocketServerTestCase.common(SocketServerTestCase.java:XXX)\n"
                    "\tat org.apache.log4j.net.SocketServerTestCase.");
                exceptionMsg.append(testName);
                exceptionMsg.append("(SocketServerTestCase.java:XXX)\n"
                    "\tat junit.framework.TestCase.runTest(TestCase.java:XXX)\n"
                    "\tat junit.framework.TestCase.runBare(TestCase.java:XXX)\n"
                    "\tat junit.framework.TestResult$1.protect(TestResult.java:XXX)\n"
                    "\tat junit.framework.TestResult.runProtected(TestResult.java:XXX)\n"
                    "\tat junit.framework.TestResult.run(TestResult.java:XXX)\n"
                    "\tat junit.framework.TestCase.run(TestCase.java:XXX)\n"
                    "\tat junit.framework.TestSuite.runTest(TestSuite.java:XXX)\n"
                    "\tat junit.framework.TestSuite.run(TestSuite.java:XXX)");

                
                LOG4CXX_DEBUG(logger, "Message " << ++i << exceptionMsg);
                LOG4CXX_ERROR(root, "Message " << ++i << exceptionMsg);

                NDC::pop();
                MDC::remove(key);
        }

        void delay(int secs)
        {
                apr_sleep(APR_USEC_PER_SEC * secs);
        }

        private:
        static const File TEMP;
        static const File FILTERED;
};

const File SocketServerTestCase::TEMP("output/temp");
const File SocketServerTestCase::FILTERED("output/filtered");

LOGUNIT_TEST_SUITE_REGISTRATION_DISABLED(SocketServerTestCase)
