// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "script_loglevel.h"
#ifndef NO_USE_LOG4CXX
#include "log4cxx/helpers/stringhelper.h"

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_LEVEL(ScriptLevel)

//-------------------------------------------------------------------------------------
ScriptLevel::ScriptLevel(int level1, const LogString& name1, int syslogEquivalent1)
: Level(level1, name1, syslogEquivalent1)
{
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::getScriptInfo() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("S_INFO"), 7));
  return scriptlv;
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::getScriptWarning() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("S_WAR"), 7));
  return scriptlv;
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::getScriptError() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("S_ERR"), 7));
  return scriptlv;
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::getScriptDebug() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("S_DBG"), 7));
  return scriptlv;
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::getScript() {
  static const LevelPtr scriptlv(new ScriptLevel(ScriptLevel::SCRIPT_INT, LOG4CXX_STR("SCRIPT"), 7));
  return scriptlv;
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::toLevelLS(const LogString& sArg)
{
   return toLevelLS(sArg, getScript());
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::toLevel(int val)
{
   return toLevel(val, getScript());
}

//-------------------------------------------------------------------------------------
LevelPtr ScriptLevel::toLevel(int val, const LevelPtr& defaultLevel)
{
   switch(val)
   {
      case SCRIPT_INT: return getScript();
	  case SCRIPT_INFO: return getScriptInfo();
	  case SCRIPT_ERR: return getScriptError();
	  case SCRIPT_DBG: return getScriptDebug();
	  case SCRIPT_WAR: return getScriptWarning();

      default: return defaultLevel;
   }
}

//-------------------------------------------------------------------------------------
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
    else if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("S_INFO"), LOG4CXX_STR("s_info"))) {
      return getScriptInfo();
    }
    else if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("S_WAR"), LOG4CXX_STR("s_war"))) {
      return getScriptWarning();
    }
    else if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("S_ERR"), LOG4CXX_STR("s_err"))) {
      return getScriptError();
    }
    else if (StringHelper::equalsIgnoreCase(sArg,
          LOG4CXX_STR("S_DBG"), LOG4CXX_STR("s_dbg"))) {
      return getScriptDebug();
    }

    return Level::toLevel(sArg, defaultLevel);
}

//-------------------------------------------------------------------------------------
#endif
