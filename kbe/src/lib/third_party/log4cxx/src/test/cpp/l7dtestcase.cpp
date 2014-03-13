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
#include <log4cxx/helpers/propertyresourcebundle.h>
#include <log4cxx/helpers/locale.h>

#include "util/compare.h"

#include <vector>
#include <sstream>

#include "testchar.h"
#include "logunit.h"
#include <log4cxx/spi/loggerrepository.h>


typedef std::basic_ostringstream<testchar> StringBuffer;

using namespace log4cxx;
using namespace log4cxx::helpers;

LOGUNIT_CLASS(L7dTestCase)
{
        LOGUNIT_TEST_SUITE(L7dTestCase);
                LOGUNIT_TEST(test1);
        LOGUNIT_TEST_SUITE_END();

        LoggerPtr root;
        ResourceBundlePtr bundles[3];

public:
        void setUp()
        {
                Locale localeUS(LOG4CXX_STR("en"), LOG4CXX_STR("US"));
                bundles[0] =
                        ResourceBundle::getBundle(LOG4CXX_STR("L7D"), localeUS);
                LOGUNIT_ASSERT(bundles[0] != 0);
 
                Locale localeFR(LOG4CXX_STR("fr"), LOG4CXX_STR("FR"));
                bundles[1] =
                        ResourceBundle::getBundle(LOG4CXX_STR("L7D"), localeFR);
                LOGUNIT_ASSERT(bundles[1] != 0);

                Locale localeCH(LOG4CXX_STR("fr"), LOG4CXX_STR("CH"));
                bundles[2] =
                        ResourceBundle::getBundle(LOG4CXX_STR("L7D"), localeCH);
                LOGUNIT_ASSERT(bundles[2] != 0);

                root = Logger::getRootLogger();
        }

        void tearDown()
        {
                root->getLoggerRepository()->resetConfiguration();
        }

        void test1()
        {
                PropertyConfigurator::configure(LOG4CXX_FILE("input/l7d1.properties"));

                log4cxx::helpers::Pool pool;

                for (int i = 0; i < 3; i++)
                {
                        root->setResourceBundle(bundles[i]);

                        LOG4CXX_L7DLOG(root, Level::getDebug(), LOG4CXX_TEST_STR("bogus1"));
                        LOG4CXX_L7DLOG(root, Level::getInfo(), LOG4CXX_TEST_STR("test"));
                        LOG4CXX_L7DLOG(root, Level::getWarn(), LOG4CXX_TEST_STR("hello_world"));


                        StringBuffer os;
                        os << (i + 1);
                        LOG4CXX_L7DLOG2(root, Level::getDebug(), LOG4CXX_TEST_STR("msg1"), os.str().c_str(),
                                 LOG4CXX_TEST_STR("log4j"));
                        LOG4CXX_L7DLOG2(root, Level::getError(), LOG4CXX_TEST_STR("bogusMsg"), os.str().c_str(),
                                 LOG4CXX_TEST_STR("log4j"));
                        LOG4CXX_L7DLOG2(root, Level::getError(), LOG4CXX_TEST_STR("msg1"), os.str().c_str(),
                                 LOG4CXX_TEST_STR("log4j"));
                        LOG4CXX_L7DLOG(root, Level::getInfo(), LOG4CXX_TEST_STR("bogus2"));
                }

                LOGUNIT_ASSERT(Compare::compare(LOG4CXX_FILE("output/temp"),
                LOG4CXX_FILE("witness/l7d.1")));
        }

};

LOGUNIT_TEST_SUITE_REGISTRATION(L7dTestCase);
