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

#ifndef _LOG4CXX_NET_SOCKET_APPENDER_SKELETON_H
#define _LOG4CXX_NET_SOCKET_APPENDER_SKELETON_H

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/socket.h>
#include <log4cxx/helpers/thread.h>
#include <log4cxx/helpers/objectoutputstream.h>

namespace log4cxx
{

        namespace net
        {

                /**
                 *  Abstract base class for SocketAppender and XMLSocketAppender
                 */
        class LOG4CXX_EXPORT SocketAppenderSkeleton : public AppenderSkeleton
        {
        private:
                /**
                host name
                */
                LogString remoteHost;

                /**
                IP address
                */
                helpers::InetAddressPtr address;

                int port;
                int reconnectionDelay;
                bool locationInfo;

        public:
                SocketAppenderSkeleton(int defaultPort, int reconnectionDelay);
                        ~SocketAppenderSkeleton();

                /**
                Connects to remote server at <code>address</code> and <code>port</code>.
                */
                SocketAppenderSkeleton(helpers::InetAddressPtr address, int port, int reconnectionDelay);

                /**
                Connects to remote server at <code>host</code> and <code>port</code>.
                */
                SocketAppenderSkeleton(const LogString& host, int port, int reconnectionDelay);

                /**
                Connect to the specified <b>RemoteHost</b> and <b>Port</b>.
                */
                void activateOptions(log4cxx::helpers::Pool& p);
                
                void close();


                   /**
                * This appender does not use a layout. Hence, this method
                * returns <code>false</code>.
                *
                        */
                bool requiresLayout() const
                        { return false; }

                /**
                * The <b>RemoteHost</b> option takes a string value which should be
                * the host name of the server where a
                * Apache Chainsaw or compatible is running.
                * */
                inline void setRemoteHost(const LogString& host)
                        { address = helpers::InetAddress::getByName(host);
                        remoteHost.assign(host); }

                /**
                Returns value of the <b>RemoteHost</b> option.
                */
                inline const LogString& getRemoteHost() const
                        { return remoteHost; }

                /**
                The <b>Port</b> option takes a positive integer representing
                the port where the server is waiting for connections.
                */
                void setPort(int port1)
                        { this->port = port1; }

                /**
                Returns value of the <b>Port</b> option.
                */
                int getPort() const
                        { return port; }

                /**
                The <b>LocationInfo</b> option takes a boolean value. If true,
                the information sent to the remote host will include location
                information. By default no location information is sent to the server.
                */
                void setLocationInfo(bool locationInfo1)
                        { this->locationInfo = locationInfo1; }

                /**
                Returns value of the <b>LocationInfo</b> option.
                */
                bool getLocationInfo() const
                        { return locationInfo; }

                /**
                The <b>ReconnectionDelay</b> option takes a positive integer
                representing the number of milliseconds to wait between each
                failed connection attempt to the server. The default value of
                this option is 30000 which corresponds to 30 seconds.

                <p>Setting this option to zero turns off reconnection
                capability.
                */
                void setReconnectionDelay(int reconnectionDelay1)
                        { this->reconnectionDelay = reconnectionDelay1; }

                /**
                Returns value of the <b>ReconnectionDelay</b> option.
                */
                int getReconnectionDelay() const
                        { return reconnectionDelay; }

                void fireConnector();

                void setOption(const LogString& option,
                                const LogString& value);

           protected:

                virtual void setSocket(log4cxx::helpers::SocketPtr& socket, log4cxx::helpers::Pool& p) = 0;
                
                virtual void cleanUp(log4cxx::helpers::Pool& p) = 0;
                
                virtual int getDefaultDelay() const = 0;
                
                virtual int getDefaultPort() const = 0;

           private:
                void connect(log4cxx::helpers::Pool& p);
                   /**
                        The Connector will reconnect when the server becomes available
                        again.  It does this by attempting to open a new connection every
                        <code>reconnectionDelay</code> milliseconds.

                        <p>It stops trying whenever a connection is established. It will
                        restart to try reconnect to the server when previpously open
                        connection is droppped.
                        */

                   helpers::Thread thread;
                   static void* LOG4CXX_THREAD_FUNC monitor(apr_thread_t* thread, void* data);
                        SocketAppenderSkeleton(const SocketAppenderSkeleton&);
                        SocketAppenderSkeleton& operator=(const SocketAppenderSkeleton&);

        }; // class SocketAppenderSkeleton
    } // namespace net
} // namespace log4cxx

#endif // _LOG4CXX_NET_SOCKET_APPENDER_SKELETON_H

