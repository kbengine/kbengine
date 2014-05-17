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

#ifndef _LOG4CXX_NET_TELNET_APPENDER_H
#define _LOG4CXX_NET_TELNET_APPENDER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif



#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/socket.h>
#include <log4cxx/helpers/serversocket.h>
#include <log4cxx/helpers/thread.h>
#include <vector>
#include <log4cxx/helpers/charsetencoder.h>

namespace log4cxx
{
        namespace helpers {
             class ByteBuffer;
        }
        namespace net
        {
/**
<p>The TelnetAppender is a log4cxx appender that specializes in
writing to a read-only socket.  The output is provided in a
telnet-friendly way so that a log can be monitored over TCP/IP.
Clients using telnet connect to the socket and receive log data.
This is handy for remote monitoring, especially when monitoring a
servlet.

<p>Here is a list of the available configuration options:

<table border=1>
<tr>
<td align=center><b>Name</b></td>
<td align=center><b>Requirement</b></td>
<td align=center><b>Description</b></td>
<td align=center><b>Sample Value</b></td>
</tr>

<tr>
<td>Port</td>
<td>optional</td>
<td>This parameter determines the port to use for announcing log events.  The default port is 23 (telnet).</td>
<td>5875</td>
</table>
*/
        class LOG4CXX_EXPORT TelnetAppender : public AppenderSkeleton
                {
                class SocketHandler;
                friend class SocketHandler;
                private:
                        static const int DEFAULT_PORT;
                        static const int MAX_CONNECTIONS;
                        int port;

                public:
                        DECLARE_LOG4CXX_OBJECT(TelnetAppender)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(TelnetAppender)
                                LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                        END_LOG4CXX_CAST_MAP()

                        TelnetAppender();
                        ~TelnetAppender();

                        /**
                        This appender requires a layout to format the text to the
                        attached client(s). */
                        virtual bool requiresLayout() const
                                { return true; }
                                
                        LogString getEncoding() const;
                        void setEncoding(const LogString& value);
        

                        /** all of the options have been set, create the socket handler and
                        wait for connections. */
                        void activateOptions(log4cxx::helpers::Pool& p);

                                                /**
                                                Set options
                                                */
                        virtual void setOption(const LogString& option, const LogString& value);

                                                /**
                                                Returns value of the <b>Port</b> option.
                                                */
                        int getPort() const
                                { return port; }

                                                /**
                                                The <b>Port</b> option takes a positive integer representing
                                                the port where the server is waiting for connections.
                                                */
                        void setPort(int port1)
                        { this->port = port1; }


                        /** shuts down the appender. */
                        void close();

                protected:
                        /** Handles a log event.  For this appender, that means writing the
                        message to each connected client.  */
                        virtual void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) ;

                        //---------------------------------------------------------- SocketHandler:

                private:
                        //   prevent copy and assignment statements
                        TelnetAppender(const TelnetAppender&);
                        TelnetAppender& operator=(const TelnetAppender&);

                        typedef log4cxx::helpers::SocketPtr Connection;
                        LOG4CXX_LIST_DEF(ConnectionList, Connection);
                        
                        void write(log4cxx::helpers::ByteBuffer&);
                        void writeStatus(const log4cxx::helpers::SocketPtr& socket, const LogString& msg, log4cxx::helpers::Pool& p);
                        ConnectionList connections;
                        LogString encoding;
                        log4cxx::helpers::CharsetEncoderPtr encoder;
                        helpers::ServerSocket* serverSocket;
                        helpers::Thread sh;
                        size_t activeConnections;
                        static void* LOG4CXX_THREAD_FUNC acceptConnections(apr_thread_t* thread, void* data);
                }; // class TelnetAppender
                
                LOG4CXX_PTR_DEF(TelnetAppender);
    } // namespace net
} // namespace log4cxx


#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif // _LOG4CXX_NET_TELNET_APPENDER_H

