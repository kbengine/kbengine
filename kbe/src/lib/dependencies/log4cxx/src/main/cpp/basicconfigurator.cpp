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
#include <log4cxx/logstring.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>

using namespace log4cxx;

void BasicConfigurator::configure()
{
   LogManager::getLoggerRepository()->setConfigured(true);
   LoggerPtr root = Logger::getRootLogger();
        static const LogString TTCC_CONVERSION_PATTERN(LOG4CXX_STR("%r [%t] %p %c %x - %m%n"));
   LayoutPtr layout(new PatternLayout(TTCC_CONVERSION_PATTERN));
   AppenderPtr appender(new ConsoleAppender(layout));
   root->addAppender(appender);
}

void BasicConfigurator::configure(const AppenderPtr& appender)
{
   LoggerPtr root = Logger::getRootLogger();
   root->addAppender(appender);
}

void BasicConfigurator::resetConfiguration()
{
   LogManager::resetConfiguration();
}
