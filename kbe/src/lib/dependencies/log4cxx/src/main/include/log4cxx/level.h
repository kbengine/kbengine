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

#ifndef _LOG4CXX_LEVEL_H
#define _LOG4CXX_LEVEL_H


#include <log4cxx/logstring.h>
#include <limits.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/objectptr.h>


namespace log4cxx
{
    class Level;
    /** smart pointer to a Level instance */
    LOG4CXX_PTR_DEF(Level);

        /**
        Defines the minimum set of levels recognized by the system, that is
        <code>OFF</code>, <code>FATAL</code>, <code>ERROR</code>,
        <code>WARN</code>, <code>INFO</code>, <code>DEBUG</code> and
        <code>ALL</code>.
        <p>The <code>Level</code> class may be subclassed to define a larger
        level set.
        */
        class LOG4CXX_EXPORT Level : public helpers::ObjectImpl
        {
        public:
                class LOG4CXX_EXPORT LevelClass : public helpers::Class
                {
                public:
                        LevelClass() : helpers::Class() {}

                        virtual LogString getName() const {
                            return LOG4CXX_STR("Level");
                        }

                        virtual LevelPtr toLevel(const LogString& sArg) const
                        { return Level::toLevelLS(sArg); }

                        virtual LevelPtr toLevel(int val) const
                        { return Level::toLevel(val); }
                };

                DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(Level, LevelClass)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(Level)
                END_LOG4CXX_CAST_MAP()

                /**
                Instantiate a Level object.
                */
                Level(int level,
                       const LogString& name,
                       int syslogEquivalent);

                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                * @param sArg level name.
                */
                static LevelPtr toLevel(const std::string& sArg);
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns the value of
                <code>defaultLevel</code>.
                * @param sArg level name.
                * @param defaultLevel level to return if no match.
                * @return 
                */
                static LevelPtr toLevel(const std::string& sArg,
                        const LevelPtr& defaultLevel);
                /**
                 *  Get the name of the level in the current encoding.
                 *  @param name buffer to which name is appended.
                 */
                void toString(std::string& name) const;

#if LOG4CXX_WCHAR_T_API
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                * @param sArg level name.
                */
                static LevelPtr toLevel(const std::wstring& sArg);
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns the value of
                <code>defaultLevel</code>.
                * @param sArg level name.
                * @param defaultLevel level to return if no match.
                * @return 
                */
                static LevelPtr toLevel(const std::wstring& sArg,
                        const LevelPtr& defaultLevel);
                /**
                 *  Get the name of the level.
                 *  @param name buffer to which name is appended.
                 */
                void toString(std::wstring& name) const;
#endif
#if LOG4CXX_UNICHAR_API
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                * @param sArg level name.
                */
                static LevelPtr toLevel(const std::basic_string<UniChar>& sArg);
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns the value of
                <code>defaultLevel</code>.
                * @param sArg level name.
                * @param defaultLevel level to return if no match.
                * @return 
                */
                static LevelPtr toLevel(const std::basic_string<UniChar>& sArg,
                        const LevelPtr& defaultLevel);
                /**
                 *  Get the name of the level.
                 *  @param name buffer to which name is appended.
                 */
                void toString(std::basic_string<UniChar>& name) const;
#endif
#if LOG4CXX_CFSTRING_API
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                * @param sArg level name.
                */
                static LevelPtr toLevel(const CFStringRef& sArg);
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns the value of
                <code>defaultLevel</code>.
                * @param sArg level name.
                * @param defaultLevel level to return if no match.
                * @return 
                */
                static LevelPtr toLevel(const CFStringRef& sArg,
                        const LevelPtr& defaultLevel);
                /**
                 *  Get the name of the level.
                 *  @param name buffer to which name is appended.
                 */
                void toString(CFStringRef& name) const;
#endif
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                * @param sArg level name.
                */
                static LevelPtr toLevelLS(const LogString& sArg);
                /**
                Convert the string passed as argument to a level. If the
                conversion fails, then this method returns the value of
                <code>defaultLevel</code>.
                * @param sArg level name.
                * @param defaultLevel level to return if no match.
                * @return 
                */
                static LevelPtr toLevelLS(const LogString& sArg,
                        const LevelPtr& defaultLevel);
                /**
                Returns the string representation of this level.
                * @return level name.
                */
                LogString toString() const;

                /**
                Convert an integer passed as argument to a level. If the
                conversion fails, then this method returns DEBUG.
                */
                static LevelPtr toLevel(int val);

                /**
                Convert an integer passed as argument to a level. If the
                conversion fails, then this method returns the specified default.
                */
                static LevelPtr toLevel(int val, const LevelPtr& defaultLevel);

                enum {
                    OFF_INT = INT_MAX,
                    FATAL_INT = 50000,
                    ERROR_INT = 40000,
                    WARN_INT = 30000,
                    INFO_INT = 20000,
                    DEBUG_INT = 10000,
                    TRACE_INT = 5000,
                    ALL_INT = INT_MIN
                };


                static LevelPtr getAll();
                static LevelPtr getFatal();
                static LevelPtr getError();
                static LevelPtr getWarn();
                static LevelPtr getInfo();
                static LevelPtr getDebug();
                static LevelPtr getTrace();
                static LevelPtr getOff();


                /**
                Two levels are equal if their level fields are equal.
                */
                virtual bool equals(const LevelPtr& level) const;

                inline bool operator==(const Level& level1) const
                        { return (this->level == level1.level); }

                inline bool operator!=(const Level& level1) const
                        { return (this->level != level1.level); }

                /**
                Return the syslog equivalent of this level as an integer.
                */
                inline int getSyslogEquivalent() const {
                  return syslogEquivalent;
                }


                /**
                Returns <code>true</code> if this level has a higher or equal
                level than the level passed as argument, <code>false</code>
                otherwise.

                <p>You should think twice before overriding the default
                implementation of <code>isGreaterOrEqual</code> method.

                */
                virtual bool isGreaterOrEqual(const LevelPtr& level) const;


                /**
                Returns the integer representation of this level.
                */
                inline int toInt() const {
                  return level;
                }

        private:
                int level;
                LogString name;
                int syslogEquivalent;
                Level(const Level&);
                Level& operator=(const Level&);
        };
}

#define DECLARE_LOG4CXX_LEVEL(level)\
public:\
        class Class##level : public Level::LevelClass\
{\
public:\
        Class##level() : Level::LevelClass() {}\
        virtual LogString getName() const { return LOG4CXX_STR(#level); } \
        virtual LevelPtr toLevel(const LogString& sArg) const\
        { return level::toLevelLS(sArg); }\
        virtual LevelPtr toLevel(int val) const\
        { return level::toLevel(val); }\
};\
DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(level, Class##level)

#define IMPLEMENT_LOG4CXX_LEVEL(level) \
IMPLEMENT_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS(level, Class##level)


#endif //_LOG4CXX_LEVEL_H
