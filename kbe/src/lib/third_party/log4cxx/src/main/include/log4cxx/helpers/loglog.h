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

#ifndef _LOG4CXX_HELPERS_LOG_LOG_H
#define _LOG4CXX_HELPERS_LOG_LOG_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/mutex.h>
#include <exception>

namespace log4cxx
{
        namespace helpers
        {
                /**
                This class used to output log statements from within the log4cxx package.

                <p>Log4cxx components cannot make log4cxx logging calls. However, it is
                sometimes useful for the user to learn about what log4cxx is
                doing. You can enable log4cxx internal logging by calling the
                <b>#setInternalDebugging</b> method.

                <p>All log4cxx internal debug calls go to standard output
                where as internal error messages are sent to
                standard error output. All internal messages are prepended with
                the string "log4cxx: ".
                */
                class LOG4CXX_EXPORT LogLog
                {
                private:
                        bool debugEnabled;

                  /**
                         In quietMode not even errors generate any output.
                   */
                        bool quietMode;
                        Mutex mutex;
                        LogLog();
                        LogLog(const LogLog&);
                        LogLog& operator=(const LogLog&);
                        static LogLog& getInstance();
 

                public:
                        /**
                        Allows to enable/disable log4cxx internal logging.
                        */
                        static void setInternalDebugging(bool enabled);

                        /**
                        This method is used to output log4cxx internal debug
                        statements. Output goes to the standard output.
                        */
                        static void debug(const LogString& msg);
                        static void debug(const LogString& msg, const std::exception& e);


                        /**
                        This method is used to output log4cxx internal error
                        statements. There is no way to disable error statements.
                        Output goes to stderr.
                        */
                        static void error(const LogString& msg);
                        static void error(const LogString& msg, const std::exception& e);


                        /**
                        In quiet mode LogLog generates strictly no output, not even
                        for errors.

                        @param quietMode <code>true</code> for no output.
                        */
                        static void setQuietMode(bool quietMode);     

                        /**
                        This method is used to output log4cxx internal warning
                        statements. There is no way to disable warning statements.
                        Output goes to stderr.
                        */
                        static void warn(const LogString&  msg);
                        static void warn(const LogString&  msg, const std::exception& e);

                        private:
                        static void emit(const LogString& msg);
                        static void emit(const std::exception& ex);
                };
        }  // namespace helpers
} // namespace log4cxx

#define LOGLOG_DEBUG(log) { \
        log4cxx::helpers::LogLog::debug(log) ; }

#define LOGLOG_WARN(log) { \
        log4cxx::helpers::LogLog::warn(log) ; }

#define LOGLOG_ERROR(log) { \
        log4cxx::helpers::LogLog::warn(log); }

#endif //_LOG4CXX_HELPERS_LOG_LOG_H
