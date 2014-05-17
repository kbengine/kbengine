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

#include "xlevel.h"
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_LEVEL(XLevel)


XLevel::XLevel(int level1, const LogString& name1, int syslogEquivalent1)
: Level(level1, name1, syslogEquivalent1)
{
}

LevelPtr XLevel::getTrace() {
  static const LevelPtr trace(new XLevel(XLevel::TRACE_INT, LOG4CXX_STR("TRACE"), 7));
  return trace;
}

LevelPtr XLevel::getLethal() {
  static const LevelPtr lethal(new XLevel(XLevel::LETHAL_INT, LOG4CXX_STR("LETHAL"), 0));
  return lethal;
}

LevelPtr XLevel::toLevelLS(const LogString& sArg)
{
   return toLevelLS(sArg, getTrace());
}


LevelPtr XLevel::toLevel(int val)
{
   return toLevel(val, getTrace());
}

LevelPtr XLevel::toLevel(int val, const LevelPtr& defaultLevel)
{
   switch(val)
   {
      case TRACE_INT: return getTrace();
      case LETHAL_INT: return getLethal();
      default: return defaultLevel;
   }
}

LevelPtr XLevel::toLevelLS(const LogString& sArg, const LevelPtr& defaultLevel)
{
   if (sArg.empty())
    {
       return defaultLevel;
    }

    if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("TRACE"), LOG4CXX_STR("trace"))) {
      return getTrace();
    }

    if (StringHelper::equalsIgnoreCase(sArg,
           LOG4CXX_STR("LETHAL"), LOG4CXX_STR("lethal"))) {
      return getLethal();
    }

    return Level::toLevel(sArg, defaultLevel);
}

