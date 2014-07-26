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
#include <log4cxx/simplelayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/level.h>
#include <log4cxx/filter/levelmatchfilter.h>
#include <log4cxx/filter/denyallfilter.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/spi/loggerrepository.h>

#include "../util/compare.h"
#include "../testchar.h"


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::filter;

LOGUNIT_CLASS(LevelMatchFilterTestCase)
{
        LOGUNIT_TEST_SUITE(LevelMatchFilterTestCase);
                LOGUNIT_TEST(accept);
                LOGUNIT_TEST(deny);
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        LoggerPtr logger;

public:
        void setUp()
        {
                root = Logger::getRootLogger();
                root->removeAllAppenders();
                logger = Logger::getLogger(LOG4CXX_TEST_STR("test"));
        }

        void tearDown()
        {
                root->getLoggerRepository()->resetConfiguration();
        }

        void accept()
        {
                // set up appender
                LayoutPtr layout = new SimpleLayout();
                AppenderPtr appender = new FileAppender(layout, ACCEPT_FILE, false);

                // create LevelMatchFilter
                LevelMatchFilterPtr matchFilter = new LevelMatchFilter();

                // attach match filter to appender
                appender->addFilter(matchFilter);

                // attach DenyAllFilter to end of filter chain to deny neutral
                // (non matching) messages
                spi::FilterPtr filter(new DenyAllFilter());
                appender->addFilter(filter);

                // set appender on root and set level to debug
                root->addAppender(appender);
                root->setLevel(Level::getDebug());

                LevelPtr levelArray[] =
                        { Level::getDebug(), Level::getInfo(), Level::getWarn(), Level::getError(), Level::getFatal() };

                int length = sizeof(levelArray)/sizeof(levelArray[0]);

                Pool pool;
                for (int x = 0; x < length; x++)
                {
                        // set the level to match
                        matchFilter->setLevelToMatch(levelArray[x]->toString());
                        LogString sbuf(LOG4CXX_STR("pass "));
                        StringHelper::toString(x, pool, sbuf);
                        sbuf.append(LOG4CXX_STR("; filter set to accept only "));
                        sbuf.append(levelArray[x]->toString());
                        sbuf.append(LOG4CXX_STR(" msgs"));
                        common(sbuf);
                }

                LOGUNIT_ASSERT(Compare::compare(ACCEPT_FILE, ACCEPT_WITNESS));
        }

        void deny()
        {
                // set up appender
                LayoutPtr layout = new SimpleLayout();
                AppenderPtr appender = new FileAppender(layout, DENY_FILE, false);

                // create LevelMatchFilter, set to deny matches
                LevelMatchFilterPtr matchFilter = new LevelMatchFilter();
                matchFilter->setAcceptOnMatch(false);

                // attach match filter to appender
                appender->addFilter(matchFilter);

                // set appender on root and set level to debug
                root->addAppender(appender);
                root->setLevel(Level::getDebug());

                LevelPtr levelArray[] =
                        { Level::getDebug(), Level::getInfo(), Level::getWarn(), Level::getError(), Level::getFatal() };

                int length = sizeof(levelArray)/sizeof(levelArray[0]);

                Pool pool;

                for (int x = 0; x < length; x++)
                {
                        // set the level to match
                        matchFilter->setLevelToMatch(levelArray[x]->toString());
                        LogString sbuf(LOG4CXX_STR("pass "));

                        StringHelper::toString(x, pool, sbuf);
                        sbuf.append(LOG4CXX_STR("; filter set to deny only "));
                        sbuf.append(levelArray[x]->toString());
                        sbuf.append(LOG4CXX_STR(" msgs"));
                        common(sbuf);
                }

                LOGUNIT_ASSERT(Compare::compare(DENY_FILE, DENY_WITNESS));
        }

        void common(const LogString& msg)
        {
                logger->debug(msg);
                logger->info(msg);
                logger->warn(msg);
                logger->error(msg);
                logger->fatal(msg);
        }

private:
        static const LogString ACCEPT_FILE;
        static const LogString ACCEPT_WITNESS;
        static const LogString DENY_FILE;
        static const LogString DENY_WITNESS;


};

const LogString LevelMatchFilterTestCase::ACCEPT_FILE(LOG4CXX_STR("output/LevelMatchFilter_accept"));
const LogString LevelMatchFilterTestCase::ACCEPT_WITNESS(LOG4CXX_STR("witness/LevelMatchFilter_accept"));
const LogString LevelMatchFilterTestCase::DENY_FILE(LOG4CXX_STR("output/LevelMatchFilter_deny"));
const LogString LevelMatchFilterTestCase::DENY_WITNESS(LOG4CXX_STR("witness/LevelMatchFilter_deny"));


LOGUNIT_TEST_SUITE_REGISTRATION(LevelMatchFilterTestCase);
