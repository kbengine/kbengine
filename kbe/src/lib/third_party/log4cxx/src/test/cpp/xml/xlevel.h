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

#include <log4cxx/level.h>

namespace log4cxx
{
   class XLevel : public Level
   {
      DECLARE_LOG4CXX_LEVEL(XLevel)

   public:
        enum
        {
            TRACE_INT = Level::DEBUG_INT - 1,
         LETHAL_INT = Level::FATAL_INT + 1
        };

      static LevelPtr getTrace();
      static LevelPtr getLethal();


      XLevel(int level, const LogString& name, int syslogEquivalent);
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
