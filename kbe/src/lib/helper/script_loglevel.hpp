#include <log4cxx/level.h>

namespace log4cxx
{
   class ScriptLevel : public Level
   {
      DECLARE_LOG4CXX_LEVEL(ScriptLevel)

   public:
        enum
        {
            SCRIPT_INT = Level::INFO_INT + 1000,
        };

      static LevelPtr getScript();


      ScriptLevel(int level, const LogString& name, int syslogEquivalent);

      /**
      Convert the string passed as argument to a level. If the
      conversion fails, then this method returns #DEBUG.
      */
      static LevelPtr toLevelLS(const LogString& sArg);

      /**
      Convert an integer passed as argument to a level. If the
      conversion fails, then this method returns #DEBUG.

      */
      static LevelPtr toLevel(int val);

      /**
      Convert an integer passed as argument to a level. If the
      conversion fails, then this method returns the specified default.
      */
      static LevelPtr toLevel(int val, const LevelPtr& defaultLevel);


      /**
      Convert the string passed as argument to a level. If the
      conversion fails, then this method returns the value of
      <code>defaultLevel</code>.
      */
        static LevelPtr toLevelLS(const LogString& sArg,
         const LevelPtr& defaultLevel);
   };
}