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
#include <log4cxx/simplelayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/level.h>
#include <log4cxx/filter/levelrangefilter.h>

#include "../util/compare.h"

#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include "../testchar.h"
#include "../logunit.h"
#include <log4cxx/spi/loggerrepository.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::filter;



LOGUNIT_CLASS(LevelRangeFilterTestCase)
{
        LOGUNIT_TEST_SUITE(LevelRangeFilterTestCase);
                LOGUNIT_TEST(accept);
                LOGUNIT_TEST(neutral);
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
                LevelRangeFilterPtr rangeFilter = new LevelRangeFilter();

                // set it to accept on a match
                rangeFilter->setAcceptOnMatch(true);

                // attach match filter to appender
                appender->addFilter(rangeFilter);

                // set appender on root and set level to debug
                root->addAppender(appender);
                root->setLevel(Level::getDebug());

                int passCount = 0;
                LogString sbuf(LOG4CXX_STR("pass "));

                Pool pool;
                StringHelper::toString(passCount, pool, sbuf);

                sbuf.append(LOG4CXX_STR("; no min or max set"));
                common(sbuf);
                passCount++;

                // test with a min set
                rangeFilter->setLevelMin(Level::getWarn());
                sbuf.assign(LOG4CXX_STR("pass "));
                StringHelper::toString(passCount, pool, sbuf);
                sbuf.append(LOG4CXX_STR("; min set to WARN, max not set"));
                common(sbuf);
                passCount++;

                // create a clean filter
                appender->clearFilters();
                rangeFilter = new LevelRangeFilter();
                appender->addFilter(rangeFilter);

                //test with max set
                rangeFilter->setLevelMax(Level::getWarn());
                sbuf.assign(LOG4CXX_STR("pass "));
                StringHelper::toString(passCount, pool, sbuf);
                sbuf.append(LOG4CXX_STR("; min not set, max set to WARN"));
                common(sbuf);
                passCount++;


                LevelPtr levelArray[] =
                        { Level::getDebug(), Level::getInfo(), Level::getWarn(),
                 Level::getError(), Level::getFatal() };

                int length = sizeof(levelArray)/sizeof(levelArray[0]);

                for (int x = 0; x < length; x++)
                {
                        // set the min level to match
                        rangeFilter->setLevelMin(levelArray[x]);

                        for (int y = length - 1; y >= 0; y--)
                        {
                                // set max level to match
                                rangeFilter->setLevelMax(levelArray[y]);

                                sbuf.assign(LOG4CXX_STR("pass "));
                                StringHelper::toString(passCount, pool, sbuf);
                                sbuf.append(LOG4CXX_STR("; filter set to accept between "));
                                sbuf.append(levelArray[x]->toString());
                                sbuf.append(LOG4CXX_STR(" and "));
                                sbuf.append(levelArray[y]->toString());
                                sbuf.append(LOG4CXX_STR(" msgs"));
                                common(sbuf);

                                // increment passCount
                                passCount++;
                        }
                }


                LOGUNIT_ASSERT(Compare::compare(ACCEPT_FILE, ACCEPT_WITNESS));
        }

        void neutral()
        {
                // set up appender
                LayoutPtr layout = new SimpleLayout();
                AppenderPtr appender = new FileAppender(layout, NEUTRAL_FILE, false);

                // create LevelMatchFilter
                LevelRangeFilterPtr rangeFilter = new LevelRangeFilter();

                // set it to accept on a match
                rangeFilter->setAcceptOnMatch(true);

                // attach match filter to appender
                appender->addFilter(rangeFilter);

                // set appender on root and set level to debug
                root->addAppender(appender);
                root->setLevel(Level::getDebug());

                int passCount = 0;
                LogString sbuf(LOG4CXX_STR("pass "));

                Pool pool;
                StringHelper::toString(passCount, pool, sbuf);

                // test with no min or max set
                sbuf.append(LOG4CXX_STR("; no min or max set"));
                common(sbuf);
                passCount++;

                // test with a min set
                rangeFilter->setLevelMin(Level::getWarn());
                sbuf.assign(LOG4CXX_STR("pass "));

                StringHelper::toString(passCount, pool, sbuf);
                sbuf.append(LOG4CXX_STR("; min set to WARN, max not set"));
                common(sbuf);
                passCount++;

                // create a clean filter
                appender->clearFilters();
                rangeFilter = new LevelRangeFilter();
                appender->addFilter(rangeFilter);

                //test with max set
                rangeFilter->setLevelMax(Level::getWarn());
                sbuf.assign(LOG4CXX_STR("pass "));

                StringHelper::toString(passCount, pool, sbuf);

                sbuf.append(LOG4CXX_STR("; min not set, max set to WARN"));
                common(sbuf);
                passCount++;

                LevelPtr levelArray[] =
                        { Level::getDebug(), Level::getInfo(), Level::getWarn(),
                 Level::getError(), Level::getFatal() };

                int length = sizeof(levelArray)/sizeof(levelArray[0]);

                for (int x = 0; x < length; x++)
                {
                        // set the min level to match
                        rangeFilter->setLevelMin(levelArray[x]);

                        for (int y = length - 1; y >= 0; y--)
                        {
                                // set max level to match
                                rangeFilter->setLevelMax(levelArray[y]);

                                sbuf.assign(LOG4CXX_STR("pass "));
                                StringHelper::toString(passCount, pool, sbuf);
                                sbuf.append(LOG4CXX_STR("; filter set to accept between "));
                                sbuf.append(levelArray[x]->toString());
                                sbuf.append(LOG4CXX_STR(" and "));
                                sbuf.append(levelArray[y]->toString());
                                sbuf.append(LOG4CXX_STR(" msgs"));
                                common(sbuf);

                                // increment passCount
                                passCount++;
                        }
                }

                LOGUNIT_ASSERT(Compare::compare(NEUTRAL_FILE, NEUTRAL_WITNESS));
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
        static const LogString NEUTRAL_FILE;
        static const LogString NEUTRAL_WITNESS;

};


const LogString LevelRangeFilterTestCase::ACCEPT_FILE(LOG4CXX_STR("output/LevelRangeFilter_accept"));
const LogString LevelRangeFilterTestCase::ACCEPT_WITNESS(LOG4CXX_STR("witness/LevelRangeFilter_accept"));
const LogString LevelRangeFilterTestCase::NEUTRAL_FILE(LOG4CXX_STR("output/LevelRangeFilter_neutral"));
const LogString LevelRangeFilterTestCase::NEUTRAL_WITNESS(LOG4CXX_STR("witness/LevelRangeFilter_neutral"));

LOGUNIT_TEST_SUITE_REGISTRATION(LevelRangeFilterTestCase);
