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

#include "appenderskeletontestcase.h"
#include "logunit.h"
#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/appenderskeleton.h>


using namespace log4cxx;
using namespace log4cxx::helpers;


void AppenderSkeletonTestCase::testDefaultThreshold() {
   ObjectPtrT<AppenderSkeleton> appender(createAppenderSkeleton());
   LevelPtr threshold(appender->getThreshold());
   LOGUNIT_ASSERT_EQUAL(Level::getAll()->toInt(), threshold->toInt());
}

void AppenderSkeletonTestCase::testSetOptionThreshold() {
    ObjectPtrT<AppenderSkeleton> appender(createAppenderSkeleton());
    appender->setOption(LOG4CXX_STR("threshold"), LOG4CXX_STR("debug"));
    LevelPtr threshold(appender->getThreshold());
    LOGUNIT_ASSERT_EQUAL(Level::getDebug()->toInt(), threshold->toInt());
}
