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
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/mdc.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/fileappender.h>

#include "util/compare.h"
#include "util/transformer.h"
#include "util/absolutedateandtimefilter.h"
#include "util/iso8601filter.h"
#include "util/absolutetimefilter.h"
#include "util/relativetimefilter.h"
#include "util/controlfilter.h"
#include "util/threadfilter.h"
#include "util/linenumberfilter.h"
#include "util/filenamefilter.h"
#include <iostream>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <apr_strings.h>
#include <log4cxx/helpers/pool.h>
#include "testchar.h"
#include "logunit.h"
#include <log4cxx/spi/loggerrepository.h>
#include <log4cxx/helpers/stringhelper.h>


#define REGEX_STR(x) x
#define PAT0 REGEX_STR("\\[[0-9A-FXx]*]\\ (DEBUG|INFO|WARN|ERROR|FATAL) .* - Message [0-9]\\{1,2\\}")
#define PAT1 ISO8601_PAT REGEX_STR(" ") PAT0
#define PAT2 ABSOLUTE_DATE_AND_TIME_PAT REGEX_STR(" ") PAT0
#define PAT3 ABSOLUTE_TIME_PAT REGEX_STR(" ") PAT0
#define PAT4 RELATIVE_TIME_PAT REGEX_STR(" ") PAT0
#define PAT5 REGEX_STR("\\[[0-9A-FXx]*]\\ (DEBUG|INFO|WARN|ERROR|FATAL) .* : Message [0-9]\\{1,2\\}")
#define PAT6 REGEX_STR("\\[[0-9A-FXx]*]\\ (DEBUG|INFO |WARN |ERROR|FATAL) .*patternlayouttest.cpp\\([0-9]\\{1,4\\}\\): Message [0-9]\\{1,3\\}")
#define PAT11a REGEX_STR("^(DEBUG|INFO |WARN |ERROR|FATAL) \\[[0-9A-FXx]*]\\ log4j.PatternLayoutTest: Message [0-9]\\{1,2\\}")
#define PAT11b REGEX_STR("^(DEBUG|INFO |WARN |ERROR|FATAL) \\[[0-9A-FXx]*]\\ root: Message [0-9]\\{1,2\\}")
#define PAT12 REGEX_STR("^\\[[0-9A-FXx]*]\\ (DEBUG|INFO |WARN |ERROR|FATAL) ")\
    REGEX_STR(".*patternlayouttest.cpp([0-9]\\{1,4\\}): ")\
    REGEX_STR("Message [0-9]\\{1,2\\}")
#define PAT_MDC_1 REGEX_STR("")

using namespace log4cxx;
using namespace log4cxx::helpers;

LOGUNIT_CLASS(PatternLayoutTest)
{
        LOGUNIT_TEST_SUITE(PatternLayoutTest);
                LOGUNIT_TEST(test1);
                LOGUNIT_TEST(test2);
                LOGUNIT_TEST(test3);
                LOGUNIT_TEST(test4);
                LOGUNIT_TEST(test5);
                LOGUNIT_TEST(test6);
                LOGUNIT_TEST(test7);
                LOGUNIT_TEST(test8);
                LOGUNIT_TEST(test9);
                LOGUNIT_TEST(test10);
                LOGUNIT_TEST(test11);
                LOGUNIT_TEST(test12);
                LOGUNIT_TEST(testMDC1);
                LOGUNIT_TEST(testMDC2);
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        LoggerPtr logger;

public:
        void setUp()
        {
                root = Logger::getRootLogger();
                MDC::clear();
                logger = Logger::getLogger(LOG4CXX_TEST_STR("java.org.apache.log4j.PatternLayoutTest"));
        }

        void tearDown()
        {
                MDC::clear();
                root->getLoggerRepository()->resetConfiguration();
        }

        void test1()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout1.properties"));
                common();
                LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/patternLayout.1")));
        }

        void test2()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout2.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT1;
                ISO8601Filter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.2")));
        }

        void test3()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout3.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT1;
                ISO8601Filter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.3")));
        }

        // Output format:
        // 06 avr. 2002 18:30:58,937 [12345] DEBUG atternLayoutTest - Message 0
        void test4()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout4.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT2;
                AbsoluteDateAndTimeFilter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.4")));
        }

        void test5()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout5.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT2;
                AbsoluteDateAndTimeFilter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.5")));
        }

        void test6()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout6.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT3;
                AbsoluteTimeFilter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.6")));
        }

        void test7()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout7.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT3;
                AbsoluteTimeFilter filter2;
                ThreadFilter filter3;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.7")));
        }

        void test8()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout8.properties"));
                common();


                ControlFilter filter1;
                filter1 << PAT4;
                //
                //   combo of relative time and thread identifier
                //     (the \\\\1 preserve a leading space)
                Filter filter2(".*]", "[main]");

                std::vector<Filter *> filters;

                filters.push_back(&filter1);
                filters.push_back(&filter2);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.8")));
        }

        void test9()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout9.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT5;
                ThreadFilter filter2;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.9")));
        }

        void test10()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout10.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT6;
                ThreadFilter filter2;
                LineNumberFilter filter3;
                FilenameFilter filenameFilter(__FILE__, "patternlayouttest.cpp");


                std::vector<Filter *> filters;
                filters.push_back(&filenameFilter);
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);


                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.10")));
        }

        void test11()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout11.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT11a << PAT11b;
                ThreadFilter filter2;

                std::vector<Filter *> filters;
                filters.push_back(&filter1);
                filters.push_back(&filter2);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.11")));
        }

        void test12()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout12.properties"));
                common();

                ControlFilter filter1;
                filter1 << PAT12;
                ThreadFilter filter2;
                LineNumberFilter filter3;
                FilenameFilter filenameFilter(__FILE__, "patternlayouttest.cpp");

                std::vector<Filter *> filters;
                filters.push_back(&filenameFilter);
                filters.push_back(&filter1);
                filters.push_back(&filter2);
                filters.push_back(&filter3);

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(FILTERED, LOG4CXX_FILE("witness/patternLayout.12")));
        }

        void testMDC1()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/patternLayout.mdc.1.properties"));
                MDC::put(LOG4CXX_TEST_STR("key1"), LOG4CXX_TEST_STR("va11"));
                MDC::put(LOG4CXX_TEST_STR("key2"), LOG4CXX_TEST_STR("va12"));
                logger->debug(LOG4CXX_TEST_STR("Hello World"));
                MDC::clear();

                LOGUNIT_ASSERT(Compare::compare(TEMP, LOG4CXX_FILE("witness/patternLayout.mdc.1")));
        }

        void testMDC2()
        {
                LogString OUTPUT_FILE   = LOG4CXX_STR("output/patternLayout.mdc.2");
                File WITNESS_FILE  = LOG4CXX_FILE("witness/patternLayout.mdc.2");

                LogString mdcMsgPattern1 = LOG4CXX_STR("%m : %X%n");
                LogString mdcMsgPattern2 = LOG4CXX_STR("%m : %X{key1}%n");
                LogString mdcMsgPattern3 = LOG4CXX_STR("%m : %X{key2}%n");
                LogString mdcMsgPattern4 = LOG4CXX_STR("%m : %X{key3}%n");
                LogString mdcMsgPattern5 = LOG4CXX_STR("%m : %X{key1},%X{key2},%X{key3}%n");

                // set up appender
                PatternLayoutPtr layout = new PatternLayout(LOG4CXX_STR("%m%n"));
                AppenderPtr appender = new FileAppender(layout, OUTPUT_FILE, false);

                // set appender on root and set level to debug
                root->addAppender(appender);
                root->setLevel(Level::getDebug());

                // output starting message
                root->debug(LOG4CXX_TEST_STR("starting mdc pattern test"));

                layout->setConversionPattern(mdcMsgPattern1);
                log4cxx::helpers::Pool pool;
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("empty mdc, no key specified in pattern"));

                layout->setConversionPattern(mdcMsgPattern2);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("empty mdc, key1 in pattern"));

                layout->setConversionPattern(mdcMsgPattern3);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("empty mdc, key2 in pattern"));

                layout->setConversionPattern(mdcMsgPattern4);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("empty mdc, key3 in pattern"));

                layout->setConversionPattern(mdcMsgPattern5);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("empty mdc, key1, key2, and key3 in pattern"));

                MDC::put(LOG4CXX_TEST_STR("key1"), LOG4CXX_TEST_STR("value1"));
                MDC::put(LOG4CXX_TEST_STR("key2"), LOG4CXX_TEST_STR("value2"));

                layout->setConversionPattern(mdcMsgPattern1);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("filled mdc, no key specified in pattern"));

                layout->setConversionPattern(mdcMsgPattern2);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("filled mdc, key1 in pattern"));

                layout->setConversionPattern(mdcMsgPattern3);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("filled mdc, key2 in pattern"));

                layout->setConversionPattern(mdcMsgPattern4);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("filled mdc, key3 in pattern"));

                layout->setConversionPattern(mdcMsgPattern5);
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("filled mdc, key1, key2, and key3 in pattern"));

                MDC::remove(LOG4CXX_TEST_STR("key1"));
                MDC::remove(LOG4CXX_TEST_STR("key2"));

                layout->setConversionPattern(LOG4CXX_STR("%m%n"));
                layout->activateOptions(pool);
                root->debug(LOG4CXX_TEST_STR("finished mdc pattern test"));

                LOGUNIT_ASSERT(Compare::compare(OUTPUT_FILE, WITNESS_FILE));
        }

       std::string createMessage(Pool& pool, int i) {
         std::string msg("Message ");
         msg.append(pool.itoa(i));
         return msg;
       }

        void common()
        {
                int i = -1;

                Pool pool;


                LOG4CXX_DEBUG(logger, createMessage(pool, ++i));
                LOG4CXX_DEBUG(root, createMessage(pool, i));

                LOG4CXX_INFO(logger, createMessage(pool, ++i));
                LOG4CXX_INFO(root, createMessage(pool, i));

                LOG4CXX_WARN(logger, createMessage(pool, ++i));
                LOG4CXX_WARN(root, createMessage(pool, i));

                LOG4CXX_ERROR(logger, createMessage(pool, ++i));
                LOG4CXX_ERROR(root, createMessage(pool, i));

                LOG4CXX_FATAL(logger, createMessage(pool, ++i));
                LOG4CXX_FATAL(root, createMessage(pool, i));
        }

private:
        static const LogString FILTERED;
        static const LogString TEMP;

};

const LogString PatternLayoutTest::TEMP(LOG4CXX_STR("output/temp"));
const LogString PatternLayoutTest::FILTERED(LOG4CXX_STR("output/filtered"));


LOGUNIT_TEST_SUITE_REGISTRATION(PatternLayoutTest);
