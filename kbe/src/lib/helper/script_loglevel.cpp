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
