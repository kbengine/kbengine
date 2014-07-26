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

#ifndef _LOG4CXX_VARIA_FALLBACK_ERROR_HANDLER_H
#define _LOG4CXX_VARIA_FALLBACK_ERROR_HANDLER_H

#include <log4cxx/spi/errorhandler.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/appender.h>
#include <log4cxx/logger.h>
#include <vector>

namespace log4cxx
{
        namespace varia
        {
                /**
                The <code>FallbackErrorHandler</code> implements the ErrorHandler
                interface such that a secondary appender may be specified.  This
                secondary appender takes over if the primary appender fails for
                whatever reason.

                <p>The error message is printed on <code>System.err</code>, and
                logged in the new secondary appender.
                */
                class LOG4CXX_EXPORT FallbackErrorHandler :
                        public virtual spi::ErrorHandler,
                        public virtual helpers::ObjectImpl
                {
                private:
                        AppenderPtr backup;
                        AppenderPtr primary;
                        std::vector<LoggerPtr> loggers;

                public:
                        DECLARE_LOG4CXX_OBJECT(FallbackErrorHandler)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(spi::OptionHandler)
                                LOG4CXX_CAST_ENTRY(spi::ErrorHandler)
                        END_LOG4CXX_CAST_MAP()

                        FallbackErrorHandler();
                        void addRef() const;
                        void releaseRef() const;


                        /**
                        <em>Adds</em> the logger passed as parameter to the list of
                        loggers that we need to search for in case of appender failure.
                        */
                        void setLogger(const LoggerPtr& logger);


                        /**
                        No options to activate.
                        */
                        void activateOptions(log4cxx::helpers::Pool& p);
                        void setOption(const LogString& option, const LogString& value);


                        /**
                        Prints the message and the stack trace of the exception on
                        <code>System.err</code>.
                        */
                        void error(const LogString& message, const std::exception& e,
                                int errorCode) const;

                        /**
                        Prints the message and the stack trace of the exception on
                        <code>System.err</code>.
                        */
                        void error(const LogString& message, const std::exception& e,
                                int errorCode, const spi::LoggingEventPtr& event) const;


                        /**
                        Print a the error message passed as parameter on
                        <code>System.err</code>.
                        */
                        void error(const LogString& /* message */) const {}

                        /**
                        Return the backup appender.
                        */
                        const AppenderPtr& getBackupAppender() const
                                { return backup; }

                        /**
                        The appender to which this error handler is attached.
                        */
                        void setAppender(const AppenderPtr& primary);

                        /**
                        Set the backup appender.
                        */
                        void setBackupAppender(const AppenderPtr& backup);
                };
        }  // namespace varia
} // namespace log4cxx

#endif //_LOG4CXX_VARIA_FALLBACK_ERROR_HANDLER_H

