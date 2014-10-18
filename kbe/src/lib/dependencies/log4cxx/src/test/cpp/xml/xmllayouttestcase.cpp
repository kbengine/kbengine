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

#include "../logunit.h"

#include <log4cxx/logger.h>
#include <log4cxx/xml/xmllayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/mdc.h>

#include "../util/transformer.h"
#include "../util/compare.h"
#include "../util/xmltimestampfilter.h"
#include "../util/xmllineattributefilter.h"
#include "../util/xmlthreadfilter.h"
#include "../util/filenamefilter.h"
#include <iostream>
#include <log4cxx/helpers/stringhelper.h>
#include "../testchar.h"
#include <log4cxx/spi/loggerrepository.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::xml;

#if defined(__LOG4CXX_FUNC__)
#undef __LOG4CXX_FUNC__
#define __LOG4CXX_FUNC__ "X::X()"
#else
#error __LOG4CXX_FUNC__ expected to be defined
#endif

class X
{
public:
        X()
        {
                LoggerPtr logger =
                        Logger::getLogger(LOG4CXX_TEST_STR("org.apache.log4j.xml.XMLLayoutTestCase$X"));
                LOG4CXX_INFO(logger, LOG4CXX_TEST_STR("in X() constructor"));
        }
};


LOGUNIT_CLASS(XMLLayoutTestCase)
{
        LOGUNIT_TEST_SUITE(XMLLayoutTestCase);
                LOGUNIT_TEST(basic);
                LOGUNIT_TEST(locationInfo);
                LOGUNIT_TEST(testCDATA);
                LOGUNIT_TEST(testNull);
                LOGUNIT_TEST(testMDC);
                LOGUNIT_TEST(testMDCEscaped);
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        LoggerPtr logger;

public:
        void setUp()
        {
                root = Logger::getRootLogger();
                root->setLevel(Level::getTrace());
                logger = Logger::getLogger(LOG4CXX_TEST_STR("org.apache.log4j.xml.XMLLayoutTestCase"));
                logger->setLevel(Level::getTrace());
        }

        void tearDown()
        {
                logger->getLoggerRepository()->resetConfiguration();
        }

        void basic()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.1"));
                const File filteredFile("output/filtered.xmlLayout.1");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                AppenderPtr appender(new FileAppender(xmlLayout, tempFileName, false));
                root->addAppender(appender);
                common();

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.1")));
        }

        void locationInfo()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.2"));
                const File filteredFile("output/filtered.xmlLayout.2");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                xmlLayout->setLocationInfo(true);
                root->addAppender(new FileAppender(xmlLayout, tempFileName, false));
                common();

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;
                FilenameFilter xmlFilenameFilter(__FILE__, "XMLLayoutTestCase.java");
                Filter line2XX("[23][0-9][0-9]", "X");
                Filter line5X("5[0-9]", "X");

                std::vector<Filter *> filters;
                filters.push_back(&xmlFilenameFilter);
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);
                filters.push_back(&line2XX);
                filters.push_back(&line5X);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.2")));
        }



#undef __LOG4CXX_FUNC__
#define __LOG4CXX_FUNC__ "void XMLLayoutTestCase::testCDATA()"



        void testCDATA()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.3"));
                const File filteredFile("output/filtered.xmlLayout.3");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                xmlLayout->setLocationInfo(true);
                FileAppenderPtr appender(new FileAppender(xmlLayout, tempFileName, false));
                root->addAppender(appender);

                LOG4CXX_TRACE(logger,
                        LOG4CXX_TEST_STR("Message with embedded <![CDATA[<hello>hi</hello>]]>."));
                LOG4CXX_DEBUG(logger,
                        LOG4CXX_TEST_STR("Message with embedded <![CDATA[<hello>hi</hello>]]>."));

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;
                FilenameFilter xmlFilenameFilter(__FILE__, "XMLLayoutTestCase.java");
                Filter line1xx("1[0-9][0-9]", "X");

                std::vector<Filter *> filters;
                filters.push_back(&xmlFilenameFilter);
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);
                filters.push_back(&line1xx);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.3")));
        }

        void testNull()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.null"));
                const File filteredFile("output/filtered.xmlLayout.null");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                FileAppenderPtr appender(new FileAppender(xmlLayout, tempFileName, false));
                root->addAppender(appender);

                LOG4CXX_DEBUG(logger, LOG4CXX_TEST_STR("hi"));
                LOG4CXX_DEBUG(logger, (char*) 0);
                LOG4CXX_DEBUG(logger, "hi");

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.null")));
        }

        void testMDC()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.mdc.1"));
                const File filteredFile("output/filtered.xmlLayout.mdc.1");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                xmlLayout->setProperties(true);
                FileAppenderPtr appender(new FileAppender(xmlLayout, tempFileName, false));
                root->addAppender(appender);

                MDC::clear();
                MDC::put(LOG4CXX_TEST_STR("key1"), LOG4CXX_TEST_STR("val1"));
                MDC::put(LOG4CXX_TEST_STR("key2"), LOG4CXX_TEST_STR("val2"));

                LOG4CXX_DEBUG(logger, LOG4CXX_TEST_STR("Hello"));

                MDC::clear();

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.mdc.1")));
        }

        // not incuded in the tests for the moment !
        void testMDCEscaped()
        {
                const LogString tempFileName(LOG4CXX_STR("output/temp.xmlLayout.mdc.2"));
                const File filteredFile("output/filtered.xmlLayout.mdc.2");

                XMLLayoutPtr xmlLayout = new XMLLayout();
                xmlLayout->setProperties(true);
                FileAppenderPtr appender(new FileAppender(xmlLayout, tempFileName, false));
                root->addAppender(appender);

                MDC::clear();
                MDC::put(LOG4CXX_TEST_STR("blahAttribute"), LOG4CXX_TEST_STR("<blah value='blah'>"));
                MDC::put(LOG4CXX_TEST_STR("<blahKey value='blah'/>"), LOG4CXX_TEST_STR("blahValue"));

                LOG4CXX_DEBUG(logger, LOG4CXX_TEST_STR("Hello"));

                MDC::clear();

                XMLTimestampFilter xmlTimestampFilter;
                XMLThreadFilter xmlThreadFilter;

                std::vector<Filter *> filters;
                filters.push_back(&xmlThreadFilter);
                filters.push_back(&xmlTimestampFilter);

                try
                {
                        Transformer::transform(tempFileName, filteredFile, filters);
                }
                catch(UnexpectedFormatException& e)
                {
                        std::cout << "UnexpectedFormatException :" << e.what() << std::endl;
                        throw;
                }

                LOGUNIT_ASSERT(Compare::compare(filteredFile, LOG4CXX_FILE("witness/xmlLayout.mdc.2")));
        }



#undef __LOG4CXX_FUNC__
#define __LOG4CXX_FUNC__ "void XMLLayoutTestCase::common()"



        void common()
        {
                int i = 0;
                X x;

            std::string msg("Message ");

                LOG4CXX_TRACE(logger, msg << i);
                LOG4CXX_TRACE(root, msg << i);

                i++;
                LOG4CXX_DEBUG(logger, msg << i);
                LOG4CXX_DEBUG(root, msg << i);

                i++;
                LOG4CXX_INFO(logger, msg << i);
                LOG4CXX_INFO(root, msg << i);

                i++;
                LOG4CXX_WARN(logger, msg << i);
                LOG4CXX_WARN(root, msg << i);

                i++;
                LOG4CXX_ERROR(logger, msg << i);
                LOG4CXX_ERROR(root, msg << i);

                i++;
                LOG4CXX_FATAL(logger, msg << i);
                LOG4CXX_FATAL(root, msg << i);

                i++;
                LOG4CXX_DEBUG(logger, "Message " << i);
                LOG4CXX_DEBUG(root, "Message " << i);

                i++;
                LOG4CXX_ERROR(logger, "Message " << i);
                LOG4CXX_ERROR(root, "Message " << i);
        }
};

LOGUNIT_TEST_SUITE_REGISTRATION(XMLLayoutTestCase);
