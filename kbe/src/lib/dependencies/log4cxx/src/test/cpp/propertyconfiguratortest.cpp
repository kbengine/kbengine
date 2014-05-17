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
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logmanager.h>
#include "vectorappender.h"
#include "logunit.h"

using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(PropertyConfiguratorTest)
{
        LOGUNIT_TEST_SUITE(PropertyConfiguratorTest);
                LOGUNIT_TEST(testInherited);
                LOGUNIT_TEST(testNull);
                LOGUNIT_TEST(testAppenderThreshold);
        LOGUNIT_TEST_SUITE_END();

public:
    void testInherited() {
        Properties props;
        props.put(LOG4CXX_STR("log4j.rootLogger"),LOG4CXX_STR("DEBUG,VECTOR1"));
        props.put(LOG4CXX_STR("log4j.logger.org.apache.log4j.PropertyConfiguratorTest"), LOG4CXX_STR("inherited,VECTOR2"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR1"), LOG4CXX_STR("org.apache.log4j.VectorAppender"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR2"), LOG4CXX_STR("org.apache.log4j.VectorAppender"));
        PropertyConfigurator::configure(props);
        LoggerPtr logger = Logger::getLogger("org.apache.log4j.PropertyConfiguratorTest");
        LOGUNIT_ASSERT_EQUAL((int) Level::DEBUG_INT,
                logger->getEffectiveLevel()->toInt());
        Logger::getRootLogger()->setLevel(Level::getError());
        LOGUNIT_ASSERT_EQUAL((int) Level::ERROR_INT,
                logger->getEffectiveLevel()->toInt());
        LogManager::resetConfiguration();
    }

    void testNull() {
        Properties props;
        props.put(LOG4CXX_STR("log4j.rootLogger"),LOG4CXX_STR("DEBUG,VECTOR1"));
        props.put(LOG4CXX_STR("log4j.logger.org.apache.log4j.PropertyConfiguratorTest"), LOG4CXX_STR("NuLL,VECTOR2"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR1"), LOG4CXX_STR("org.apache.log4j.VectorAppender"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR2"), LOG4CXX_STR("org.apache.log4j.VectorAppender"));
        PropertyConfigurator::configure(props);
        LoggerPtr logger = Logger::getLogger("org.apache.log4j.PropertyConfiguratorTest");
        LOGUNIT_ASSERT_EQUAL((int) Level::DEBUG_INT,
                logger->getEffectiveLevel()->toInt());
        Logger::getRootLogger()->setLevel(Level::getError());
        LOGUNIT_ASSERT_EQUAL((int) Level::ERROR_INT,
                logger->getEffectiveLevel()->toInt());
        LogManager::resetConfiguration();
    }

    void testAppenderThreshold() {
        Properties props;
        props.put(LOG4CXX_STR("log4j.rootLogger"), LOG4CXX_STR("ALL,VECTOR1"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR1"), LOG4CXX_STR("org.apache.log4j.VectorAppender"));
        props.put(LOG4CXX_STR("log4j.appender.VECTOR1.threshold"), LOG4CXX_STR("WARN"));
        PropertyConfigurator::configure(props);
        LoggerPtr root(Logger::getRootLogger());
        VectorAppenderPtr appender(root->getAppender(LOG4CXX_STR("VECTOR1")));
        LOGUNIT_ASSERT_EQUAL((int) Level::WARN_INT, appender->getThreshold()->toInt());
        LOG4CXX_INFO(root, "Info message");
        LOG4CXX_WARN(root, "Warn message");
        LOG4CXX_WARN(root, "Error message");
        LOGUNIT_ASSERT_EQUAL((size_t) 2, appender->vector.size());        
        LogManager::resetConfiguration();
    }

};


LOGUNIT_TEST_SUITE_REGISTRATION(PropertyConfiguratorTest);
