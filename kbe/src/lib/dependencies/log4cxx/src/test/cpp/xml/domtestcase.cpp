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
#include <log4cxx/xml/domconfigurator.h>
#include "../logunit.h"
#include "../util/compare.h"
#include "xlevel.h"
#include "../util/controlfilter.h"
#include "../util/iso8601filter.h"
#include "../util/threadfilter.h"
#include "../util/transformer.h"
#include <iostream>
#include <log4cxx/file.h>
#include <log4cxx/fileappender.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#include "../testchar.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::xml;


#define TEST1_1A_PAT \
        "(DEBUG|INFO |WARN |ERROR|FATAL) \\w*\\.\\w* - Message [0-9]"

#define TEST1_1B_PAT "(DEBUG|INFO |WARN |ERROR|FATAL) root - Message [0-9]"

#define TEST1_2_PAT "^[0-9]\\{4\\}-[0-9]\\{2\\}-[0-9]\\{2\\} [0-9]\\{2\\}:[0-9]\\{2\\}:[0-9]\\{2\\},[0-9]\\{3\\} " \
        "\\[0x[0-9A-F]*]\\ (DEBUG|INFO|WARN|ERROR|FATAL) .* - Message [0-9]"

LOGUNIT_CLASS(DOMTestCase)
{
        LOGUNIT_TEST_SUITE(DOMTestCase);
                LOGUNIT_TEST(test1);
#if defined(_WIN32)
                LOGUNIT_TEST(test2);
#endif
                LOGUNIT_TEST(test3);
                LOGUNIT_TEST(test4);                
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        LoggerPtr logger;

        static const File TEMP_A1;
        static const File TEMP_A2;
        static const File FILTERED_A1;
        static const File FILTERED_A2;
        static const File TEMP_A1_2;
        static const File TEMP_A2_2;
        static const File FILTERED_A1_2;
        static const File FILTERED_A2_2;

public:
        void setUp()
        {
                root = Logger::getRootLogger();
                logger = Logger::getLogger(LOG4CXX_TEST_STR("org.apache.log4j.xml.DOMTestCase"));
        }

        void tearDown()
        {
                root->getLoggerRepository()->resetConfiguration();
        }


        void test1() {
                DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/DOMTestCase1.xml"));
                common();

                ControlFilter cf1;
                cf1 << TEST1_1A_PAT << TEST1_1B_PAT;

                ControlFilter cf2;
                cf2 << TEST1_2_PAT;

                ThreadFilter threadFilter;
                ISO8601Filter iso8601Filter;

                std::vector<Filter *> filters1;
                filters1.push_back(&cf1);

                std::vector<Filter *> filters2;
                filters2.push_back(&cf2);
                filters2.push_back(&threadFilter);
                filters2.push_back(&iso8601Filter);

                try
                {
                        Transformer::transform(TEMP_A1, FILTERED_A1, filters1);
                        Transformer::transform(TEMP_A2, FILTERED_A2, filters2);
                }
                catch(UnexpectedFormatException& e)
                {
                    std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                const File witness1(LOG4CXX_TEST_STR("witness/dom.A1.1"));
                const File witness2(LOG4CXX_TEST_STR("witness/dom.A2.1"));
                //   TODO: A1 doesn't contain duplicate entries
                //
                //                LOGUNIT_ASSERT(Compare::compare(FILTERED_A1, witness1));
                LOGUNIT_ASSERT(Compare::compare(FILTERED_A2, witness2));
        }

        //
        //   Same test but backslashes instead of forward
        //
        void test2() {
                DOMConfigurator::configure(LOG4CXX_TEST_STR("input\\xml\\DOMTestCase2.xml"));
                common();

                ThreadFilter threadFilter;
                ISO8601Filter iso8601Filter;

                std::vector<Filter *> filters1;

                std::vector<Filter *> filters2;
                filters2.push_back(&threadFilter);
                filters2.push_back(&iso8601Filter);

                try
                {
                        Transformer::transform(TEMP_A1_2, FILTERED_A1_2, filters1);
                        Transformer::transform(TEMP_A2_2, FILTERED_A2_2, filters2);
                }
                catch(UnexpectedFormatException& e)
                {
                    std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                const File witness1(LOG4CXX_TEST_STR("witness/dom.A1.2"));
                const File witness2(LOG4CXX_TEST_STR("witness/dom.A2.2"));
                //   TODO: A1 doesn't contain duplicate entries
                //
                //                LOGUNIT_ASSERT(Compare::compare(FILTERED_A1, witness1));
                LOGUNIT_ASSERT(Compare::compare(FILTERED_A2, witness2));
        }


        void common()
        {
                int i = 0;

                LOG4CXX_DEBUG(logger, "Message " << i);
                LOG4CXX_DEBUG(root, "Message " << i);

                i++;
                LOG4CXX_INFO(logger, "Message " << i);
                LOG4CXX_INFO(root, "Message " << i);

                i++;
                LOG4CXX_WARN(logger, "Message " << i);
                LOG4CXX_WARN(root, "Message " << i);

                i++;
                LOG4CXX_ERROR(logger, "Message " << i);
                LOG4CXX_ERROR(root, "Message " << i);

                i++;
                LOG4CXX_FATAL(logger, "Message " << i);
                LOG4CXX_FATAL(root, "Message " << i);

        }
      
        /**
         *   Creates a output file that ends with a superscript 3.
         *   Output file is checked by build.xml after completion.
         */  
        void test3() {
                DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/DOMTestCase3.xml"));
                LOG4CXX_INFO(logger, "File name is expected to end with a superscript 3");
#if LOG4CXX_LOGCHAR_IS_UTF8
                const logchar fname[] = { 0x6F, 0x75, 0x74, 0x70, 0x75, 0x74, 0x2F, 0x64, 0x6F, 0x6D, 0xC2, 0xB3, 0 };
#else
                const logchar fname[] = { 0x6F, 0x75, 0x74, 0x70, 0x75, 0x74, 0x2F, 0x64, 0x6F, 0x6D, 0xB3, 0 };
#endif
                File file;
                file.setPath(fname);
                Pool p;
                bool exists = file.exists(p);
                LOGUNIT_ASSERT(exists);
        }

        /**
         *   Creates a output file that ends with a ideographic 4.
         *   Output file is checked by build.xml after completion.
         */  
        void test4() {
                DOMConfigurator::configure(LOG4CXX_TEST_STR("input/xml/DOMTestCase4.xml"));
                LOG4CXX_INFO(logger, "File name is expected to end with an ideographic 4");
#if LOG4CXX_LOGCHAR_IS_UTF8
                const logchar fname[] = { 0x6F, 0x75, 0x74, 0x70, 0x75, 0x74, 0x2F, 0x64, 0x6F, 0x6D, 0xE3, 0x86, 0x95, 0 };
#else
                const logchar fname[] = { 0x6F, 0x75, 0x74, 0x70, 0x75, 0x74, 0x2F, 0x64, 0x6F, 0x6D, 0x3195, 0 };
#endif
                File file;
                file.setPath(fname);
                Pool p;
                bool exists = file.exists(p);
                LOGUNIT_ASSERT(exists);
        }
        
};

LOGUNIT_TEST_SUITE_REGISTRATION(DOMTestCase);

const File DOMTestCase::TEMP_A1(LOG4CXX_TEST_STR("output/temp.A1"));
const File DOMTestCase::TEMP_A2(LOG4CXX_TEST_STR("output/temp.A2"));
const File DOMTestCase::FILTERED_A1(LOG4CXX_TEST_STR("output/filtered.A1"));
const File DOMTestCase::FILTERED_A2(LOG4CXX_TEST_STR("output/filtered.A2"));

const File DOMTestCase::TEMP_A1_2(LOG4CXX_TEST_STR("output/temp.A1.2"));
const File DOMTestCase::TEMP_A2_2(LOG4CXX_TEST_STR("output/temp.A2.2"));
const File DOMTestCase::FILTERED_A1_2(LOG4CXX_TEST_STR("output/filtered.A1.2"));
const File DOMTestCase::FILTERED_A2_2(LOG4CXX_TEST_STR("output/filtered.A2.2"));

