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

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>

#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "../logunit.h"
#include "../util/transformer.h"
#include "../util/compare.h"
#include "../util/controlfilter.h"
#include "../util/threadfilter.h"
#include "../util/linenumberfilter.h"
#include <iostream>
#include <log4cxx/file.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::xml;

#define TEST1_A_PAT "FALLBACK - test - Message [0-9]"
#define TEST1_B_PAT "FALLBACK - root - Message [0-9]"
#define TEST1_2_PAT \
        "^[0-9]\\{4\\}-[0-9]\\{2\\}-[0-9]\\{2\\} [0-9]\\{2\\}:[0-9]\\{2\\}:[0-9]\\{2\\},[0-9]\\{3\\} " \
        "\\[main]\\ (DEBUG|INFO|WARN|ERROR|FATAL) .* - Message [0-9]"

LOGUNIT_CLASS(ErrorHandlerTestCase)
{
        LOGUNIT_TEST_SUITE(ErrorHandlerTestCase);
                LOGUNIT_TEST(test1);
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        LoggerPtr logger;

    static const File TEMP;
    static const File FILTERED;


public:
        void setUp()
        {
                root = Logger::getRootLogger();
                logger = Logger::getLogger("test");
        }

        void tearDown()
        {
                logger->getLoggerRepository()->resetConfiguration();
        }

        void test1()
        {
                DOMConfigurator::configure("input/xml/fallback1.xml");
                common();

                ControlFilter cf;
                cf << TEST1_A_PAT << TEST1_B_PAT << TEST1_2_PAT;

                ThreadFilter threadFilter;
                LineNumberFilter lineNumberFilter;

                std::vector<Filter *> filters;
                filters.push_back(&cf);
                filters.push_back(&threadFilter);
                filters.push_back(&lineNumberFilter);

        common();

                try
                {
                        Transformer::transform(TEMP, FILTERED, filters);
                }
                catch(UnexpectedFormatException& e)
                {
            std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

        const File witness("witness/fallback");
                LOGUNIT_ASSERT(Compare::compare(FILTERED, witness));
        }

        void common()
        {
                int i = -1;

        std::ostringstream os;
        os << "Message " << ++ i;
                LOG4CXX_DEBUG(logger, os.str());
                LOG4CXX_DEBUG(root, os.str());

        os.str("");
        os << "Message " << ++i;
                LOG4CXX_INFO(logger, os.str());
                LOG4CXX_INFO(root, os.str());

        os.str("");
        os << "Message " << ++i;
                LOG4CXX_WARN(logger, os.str());
                LOG4CXX_WARN(root, os.str());

        os.str("");
        os << "Message " << ++i;
                LOG4CXX_ERROR(logger, os.str());
                LOG4CXX_ERROR(root, os.str());

        os.str("");
        os << "Message " << ++i;
                LOG4CXX_FATAL(logger, os.str());
                LOG4CXX_FATAL(root, os.str());
        }
};

//TODO: Not sure this test ever worked.  0.9.7 didn't call common
//   had nothing that attempted to dispatch any log events

//LOGUNIT_TEST_SUITE_REGISTRATION(ErrorHandlerTestCase);

const File ErrorHandlerTestCase::TEMP("output/temp");
const File ErrorHandlerTestCase::FILTERED("output/filtered");

