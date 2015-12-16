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
#if defined(_MSC_VER)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#define __STDC_CONSTANT_MACROS
#include <log4cxx/net/socketappenderskeleton.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/bytearrayoutputstream.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

SocketAppenderSkeleton::SocketAppenderSkeleton(int defaultPort, int reconnectionDelay1)
:  remoteHost(),
   address(),
   port(defaultPort),
   reconnectionDelay(reconnectionDelay1),
   locationInfo(false),
   thread() {
}

SocketAppenderSkeleton::SocketAppenderSkeleton(InetAddressPtr address1, int port1, int delay)
:
   remoteHost(),
   address(address1),
   port(port1),
   reconnectionDelay(delay),
   locationInfo(false),
   thread() {
    remoteHost = this->address->getHostName();
}

SocketAppenderSkeleton::SocketAppenderSkeleton(const LogString& host, int port1, int delay)
:   remoteHost(host),
    address(InetAddress::getByName(host)),
    port(port1),
    reconnectionDelay(delay),
    locationInfo(false),
    thread() {
}

SocketAppenderSkeleton::~SocketAppenderSkeleton()
{
        finalize();
}

void SocketAppenderSkeleton::activateOptions(Pool& p)
{
        AppenderSkeleton::activateOptions(p);
        connect(p);
}

void SocketAppenderSkeleton::close() {
    synchronized sync(mutex);
    if (closed) return;
    closed = true;
    cleanUp(pool);
    thread.interrupt();
}

void SocketAppenderSkeleton::connect(Pool& p) {
    if (address == 0) {
        LogLog::error(LogString(LOG4CXX_STR("No remote host is set for Appender named \"")) +
             name + LOG4CXX_STR("\"."));
    } else {
        cleanUp(p);
        try {
            SocketPtr socket(new Socket(address, port));
            setSocket(socket, p);
        } catch(SocketException& e) {
                LogString msg = LOG4CXX_STR("Could not connect to remote log4cxx server at [")
                        +address->getHostName()+LOG4CXX_STR("].");
                if(reconnectionDelay > 0)
                {
                        msg += LOG4CXX_STR(" We will try again later. ");
                }
                fireConnector(); // fire the connector thread
                LogLog::error(msg, e);
        }
    }
}

void SocketAppenderSkeleton::setOption(const LogString& option, const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("REMOTEHOST"), LOG4CXX_STR("remotehost")))
        {
                setRemoteHost(value);
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PORT"), LOG4CXX_STR("port")))
        {
                setPort(OptionConverter::toInt(value, getDefaultPort()));
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
        {
                setLocationInfo(OptionConverter::toBoolean(value, false));
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("RECONNECTIONDELAY"), LOG4CXX_STR("reconnectiondelay")))
        {
                setReconnectionDelay(OptionConverter::toInt(value, getDefaultDelay()));
        }
        else
        {
                AppenderSkeleton::setOption(option, value);
        }
}

void SocketAppenderSkeleton::fireConnector()
{
        synchronized sync(mutex);
        if (thread.isActive()) {
                thread.run(monitor, this);
        }
}

void* LOG4CXX_THREAD_FUNC SocketAppenderSkeleton::monitor(apr_thread_t* /* thread */, void* data) {
        SocketAppenderSkeleton* socketAppender = (SocketAppenderSkeleton*) data;
        SocketPtr socket;
        bool isClosed = socketAppender->closed;
        while(!isClosed)
        {
                try
                {
                        Thread::sleep(socketAppender->reconnectionDelay);
                        LogLog::debug(LogString(LOG4CXX_STR("Attempting connection to "))
                                + socketAppender->address->getHostName());
                        socket = new Socket(socketAppender->address, socketAppender->port);
                        Pool p;
                        socketAppender->setSocket(socket, p);
                        LogLog::debug(LOG4CXX_STR("Connection established. Exiting connector thread."));
                        return NULL;
                }
                catch(ConnectException&)
                {
                        LogLog::debug(LOG4CXX_STR("Remote host ")
                                +socketAppender->address->getHostName()
                                +LOG4CXX_STR(" refused connection."));
                }
                catch(IOException& e)
                {
                        LogString exmsg;
                        log4cxx::helpers::Transcoder::decode(e.what(), exmsg);

                        LogLog::debug(((LogString) LOG4CXX_STR("Could not connect to "))
                                 + socketAppender->address->getHostName()
                                 + LOG4CXX_STR(". Exception is ")
                                 + exmsg);
                }
                isClosed = socketAppender->closed;
        }

        LogLog::debug(LOG4CXX_STR("Exiting Connector.run() method."));
        return NULL;
}
