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
#include <log4cxx/spi/rootlogger.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/level.h>
#include <log4cxx/appender.h>

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

RootLogger::RootLogger(Pool& pool, const LevelPtr& level1) : 
    Logger(pool, LOG4CXX_STR("root"))
{
   setLevel(level1);
}

const LevelPtr& RootLogger::getEffectiveLevel() const
{
   return level;
}

void RootLogger::setLevel(const LevelPtr& level1)
{
   if(level1 == 0)
   {
      LogLog::error(LOG4CXX_STR("You have tried to set a null level to root."));
   }
   else
   {

      this->level = level1;
   }
}



