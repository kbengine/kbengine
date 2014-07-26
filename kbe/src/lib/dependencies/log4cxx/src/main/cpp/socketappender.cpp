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

#include <log4cxx/net/socketappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/objectoutputstream.h>
#include <apr_time.h>
#include <apr_atomic.h>
#include <apr_thread_proc.h>
#include <log4cxx/helpers/socketoutputstream.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

IMPLEMENT_LOG4CXX_OBJECT(SocketAppender)


// The default port number of remote logging server (4560)
int SocketAppender::DEFAULT_PORT                 = 4560;

// The default reconnection delay (30000 milliseconds or 30 seconds).
int SocketAppender::DEFAULT_RECONNECTION_DELAY   = 30000;



SocketAppender::SocketAppender()
: SocketAppenderSkeleton(DEFAULT_PORT, DEFAULT_RECONNECTION_DELAY) {
}

SocketAppender::SocketAppender(InetAddressPtr& address1, int port1)
: SocketAppenderSkeleton(address1, port1, DEFAULT_RECONNECTION_DELAY) {
    Pool p;
    activateOptions(p);
}

SocketAppender::SocketAppender(const LogString& host, int port1)
: SocketAppenderSkeleton(host, port1, DEFAULT_RECONNECTION_DELAY) {
    Pool p;
    activateOptions(p);
}

SocketAppender::~SocketAppender()
{
    finalize();
}

int SocketAppender::getDefaultDelay() const {
    return DEFAULT_RECONNECTION_DELAY;
}

int SocketAppender::getDefaultPort() const {
    return DEFAULT_PORT;
}

void SocketAppender::setSocket(log4cxx::helpers::SocketPtr& socket, Pool& p) {
    synchronized sync(mutex);
    oos = new ObjectOutputStream(new SocketOutputStream(socket), p);
}

void SocketAppender::cleanUp(Pool& p) {
    if (oos != 0) {
        try {
            oos->close(p);
            oos = 0;
        } catch(std::exception& e) {
        }
    }
}


void SocketAppender::append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) {
    if (oos != 0) {
        LogString ndcVal;
        event->getNDC(ndcVal);
        event->getThreadName();
        // Get a copy of this thread's MDC.
        event->getMDCCopy();
        try {
           event->write(*oos, p);
           oos->flush(p);
        } catch(std::exception& e) {
           oos = 0;
           LogLog::warn(LOG4CXX_STR("Detected problem with connection: "), e);
           if (getReconnectionDelay() > 0) {
               fireConnector();
           }
        }
    }
}
