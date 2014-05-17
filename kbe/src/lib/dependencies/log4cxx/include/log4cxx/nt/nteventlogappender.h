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

#ifndef _LOG4CXX_NT_EVENT_LOG_APPENDER_HEADER_
#define _LOG4CXX_NT_EVENT_LOG_APPENDER_HEADER_

#include <log4cxx/appenderskeleton.h>


namespace log4cxx
{
        namespace nt
        {
                /**
                 * Appends log events to NT EventLog.
                 */
                class LOG4CXX_EXPORT NTEventLogAppender : public AppenderSkeleton
                {
                public:
                        DECLARE_LOG4CXX_OBJECT(NTEventLogAppender)
                        BEGIN_LOG4CXX_CAST_MAP()
                            LOG4CXX_CAST_ENTRY(NTEventLogAppender)
                            LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                        END_LOG4CXX_CAST_MAP()

                        NTEventLogAppender();
                        NTEventLogAppender(const LogString& server, const LogString& log,
                                const LogString& source, const LayoutPtr& layout);

                        virtual ~NTEventLogAppender();

                        virtual void activateOptions(log4cxx::helpers::Pool& p);
                        virtual void close();
                        virtual void setOption(const LogString& option, const LogString& value);

                        /**
                         * The SocketAppender does not use a layout. Hence, this method
                         * returns <code>false</code>.
                         *
                         */
                         bool requiresLayout() const
                                { return true; }

                        void setSource(const LogString& source)
                                { this->source.assign(source); }

                        const LogString& getSource() const
                                { return source; }

                        void setLog(const LogString& log)
                                { this->log.assign(log); }

                        const LogString& getLog() const
                                { return log; }

                        void setServer(const LogString& server)
                                { this->server.assign(server); }

                        const LogString& getServer() const
                                { return server; }


                protected:
                        //
                        //   these typedef are proxies for the real Win32 definitions
                        //     and need to be cast to the global definitions before
                        //     use with a Win32 API call
                        typedef void SID;
                        typedef void* HANDLE;

                        virtual void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);
                        static unsigned short getEventType(const spi::LoggingEventPtr& event);
                        static unsigned short getEventCategory(const spi::LoggingEventPtr& event);
                        /*
                         * Add this source with appropriate configuration keys to the registry.
                         */
                        void addRegistryInfo();

                        // Data
                        LogString server;
                        LogString log;
                        LogString source;
                        HANDLE hEventLog;
                        SID * pCurrentUserSID;
                        static LogString getErrorString(const LogString& function);

                private:
                        NTEventLogAppender(const NTEventLogAppender&);
                        NTEventLogAppender& operator=(const NTEventLogAppender&);
                }; // class NTEventLogAppender

                LOG4CXX_PTR_DEF(NTEventLogAppender);

    }  // namespace nt
} // namespace log4cxx

#endif //_LOG4CXX_NT_EVENT_LOG_APPENDER_HEADER_
