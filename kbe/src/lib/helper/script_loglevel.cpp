/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "script_loglevel.hpp"
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_LEVEL(ScriptLevel)


ScriptLevel::ScriptLevel(int level1, const LogString& name1, int syslogEquivalent1)
: Level(level1, name1, syslogEquivalent1)
{
}

LevelPtr ScriptLevel::getScript() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("SCRIPT"), 7));
  return scriptlv;
}


LevelPtr ScriptLevel::toLevelLS(const LogString& sArg)
{
   return toLevelLS(sArg, getScript());
}


LevelPtr ScriptLevel::toLevel(int val)
{
   return toLevel(val, getScript());
}

LevelPtr ScriptLevel::toLevel(int val, const LevelPtr& defaultLevel)
{
   switch(val)
   {
      case SCRIPT_INT: return getScript();
      default: return defaultLevel;
   }
}

LevelPtr ScriptLevel::toLevelLS(const LogString& sArg, const LevelPtr& defaultLevel)
{
   if (sArg.empty())
    {
       return defaultLevel;
    }

    if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("SCRIPT"), LOG4CXX_STR("script"))) {
      return getScript();
    }

    return Level::toLevel(sArg, defaultLevel);
}

