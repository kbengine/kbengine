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

#include <log4cxx/net/xmlsocketappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/xml/xmllayout.h>
#include <log4cxx/level.h>
#include <log4cxx/helpers/transform.h>
#include <apr_time.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/socketoutputstream.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;
using namespace log4cxx::xml;

IMPLEMENT_LOG4CXX_OBJECT(XMLSocketAppender)

// The default port number of remote logging server (4560)
int XMLSocketAppender::DEFAULT_PORT                 = 4560;

// The default reconnection delay (30000 milliseconds or 30 seconds).
int XMLSocketAppender::DEFAULT_RECONNECTION_DELAY   = 30000;

const int XMLSocketAppender::MAX_EVENT_LEN          = 1024;

XMLSocketAppender::XMLSocketAppender()
: SocketAppenderSkeleton(DEFAULT_PORT, DEFAULT_RECONNECTION_DELAY)
{
        layout = new XMLLayout();
}

XMLSocketAppender::XMLSocketAppender(InetAddressPtr address1, int port1)
: SocketAppenderSkeleton(address1, port1, DEFAULT_RECONNECTION_DELAY)
{
        layout = new XMLLayout();
        Pool p;
        activateOptions(p);
}

XMLSocketAppender::XMLSocketAppender(const LogString& host, int port1)
: SocketAppenderSkeleton(host, port1, DEFAULT_RECONNECTION_DELAY)
{
        layout = new XMLLayout();
        Pool p;
        activateOptions(p);
}

XMLSocketAppender::~XMLSocketAppender() {
    finalize();
}


int XMLSocketAppender::getDefaultDelay() const {
    return DEFAULT_RECONNECTION_DELAY;
}

int XMLSocketAppender::getDefaultPort() const {
    return DEFAULT_PORT;
}

void XMLSocketAppender::setSocket(log4cxx::helpers::SocketPtr& socket, Pool& p) {
    OutputStreamPtr os(new SocketOutputStream(socket));
    CharsetEncoderPtr charset(CharsetEncoder::getUTF8Encoder());
    synchronized sync(mutex);
    writer = new OutputStreamWriter(os, charset);
}

void XMLSocketAppender::cleanUp(Pool& p) {
    if (writer != 0) {
        try {
            writer->close(p);
            writer = 0;
        } catch(std::exception &e) {
        }
    }
}

void XMLSocketAppender::append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) {
    if (writer != 0) {
        LogString output;
        layout->format(output, event, p);
        try {
            writer->write(output, p);
            writer->flush(p);
        } catch(std::exception& e) {
           writer = 0;
           LogLog::warn(LOG4CXX_STR("Detected problem with connection: "), e);
           if (getReconnectionDelay() > 0) {
               fireConnector();
           }
        }
    }
}




