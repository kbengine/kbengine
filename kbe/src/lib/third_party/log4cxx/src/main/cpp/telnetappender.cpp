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

#include <log4cxx/net/telnetappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/synchronized.h>
#include <apr_thread_proc.h>
#include <apr_atomic.h>
#include <apr_strings.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/bytebuffer.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;

#if APR_HAS_THREADS

IMPLEMENT_LOG4CXX_OBJECT(TelnetAppender)

/** The default telnet server port */
const int TelnetAppender::DEFAULT_PORT = 23;

/** The maximum number of concurrent connections */
const int TelnetAppender::MAX_CONNECTIONS = 20;

TelnetAppender::TelnetAppender()
  : port(DEFAULT_PORT), connections(MAX_CONNECTIONS),
    encoding(LOG4CXX_STR("UTF-8")), 
    encoder(CharsetEncoder::getUTF8Encoder()), 
    serverSocket(NULL), sh()
{
   synchronized sync(mutex);
   activeConnections = 0;
}

TelnetAppender::~TelnetAppender()
{
        finalize();
        delete serverSocket;
}

void TelnetAppender::activateOptions(Pool& /* p */)
{
        if (serverSocket == NULL) {
                serverSocket = new ServerSocket(port);
                serverSocket->setSoTimeout(1000);                
        }
        sh.run(acceptConnections, this);
}

void TelnetAppender::setOption(const LogString& option,
        const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PORT"), LOG4CXX_STR("port")))
        {
                setPort(OptionConverter::toInt(value, DEFAULT_PORT));
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("ENCODING"), LOG4CXX_STR("encoding")))
        {
                setEncoding(value);
        }
        else
        {
                AppenderSkeleton::setOption(option, value);
        }
}

LogString TelnetAppender::getEncoding() const {
    synchronized sync(mutex);
    return encoding;
}

void TelnetAppender::setEncoding(const LogString& value) {
    synchronized sync(mutex);
    encoder = CharsetEncoder::getEncoder(value);
    encoding = value;
}


void TelnetAppender::close()
{
        synchronized sync(mutex);
        if (closed) return;
        closed = true;

        SocketPtr nullSocket;
        for(ConnectionList::iterator iter = connections.begin();
            iter != connections.end();
            iter++) {
            if (*iter != 0) {
                (*iter)->close();
                *iter = nullSocket;
            }
        }

        if (serverSocket != NULL) {
                try {
                    serverSocket->close();
                } catch(Exception&) {
                }
        }
        
        try {
            sh.join();
        } catch(Exception& ex) {
        }

        activeConnections = 0;
}


void TelnetAppender::write(ByteBuffer& buf) {
        for (ConnectionList::iterator iter = connections.begin();
             iter != connections.end();
             iter++) {
             if (*iter != 0) {
                try {
                    ByteBuffer b(buf.current(), buf.remaining());
                    (*iter)->write(b);
                } catch(Exception& ex) {
                    // The client has closed the connection, remove it from our list:
                    *iter = 0;
                    activeConnections--;
                }
            }
        }
}

void TelnetAppender::writeStatus(const SocketPtr& socket, const LogString& msg, Pool& p) {
        size_t bytesSize = msg.size() * 2;
        char* bytes = p.pstralloc(bytesSize);
                
        LogString::const_iterator msgIter(msg.begin());
        ByteBuffer buf(bytes, bytesSize);

        while(msgIter != msg.end()) {
            encoder->encode(msg, msgIter, buf);
            buf.flip();
            socket->write(buf);
            buf.clear();
        }
}

void TelnetAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{
        size_t count = activeConnections;
        if (count > 0) {
                LogString msg;
                this->layout->format(msg, event, pool);
                msg.append(LOG4CXX_STR("\r\n"));
                size_t bytesSize = msg.size() * 2;
                char* bytes = p.pstralloc(bytesSize);
                
                LogString::const_iterator msgIter(msg.begin());
                ByteBuffer buf(bytes, bytesSize);

                synchronized sync(this->mutex);
                while(msgIter != msg.end()) {
                    log4cxx_status_t stat = encoder->encode(msg, msgIter, buf);
                    buf.flip();
                    write(buf);
                    buf.clear();
                    if (CharsetEncoder::isError(stat)) {
                        LogString unrepresented(1, 0x3F /* '?' */);
                        LogString::const_iterator unrepresentedIter(unrepresented.begin());
                        stat = encoder->encode(unrepresented, unrepresentedIter, buf);
                        buf.flip();
                        write(buf);
                        buf.clear();
                        msgIter++;
                    }
                }
        }
}

void* APR_THREAD_FUNC TelnetAppender::acceptConnections(apr_thread_t* /* thread */, void* data) {
    TelnetAppender* pThis = (TelnetAppender*) data;

    // main loop; is left when This->closed is != 0 after an accept()
    while(true)
    {
        try
        {
                SocketPtr newClient = pThis->serverSocket->accept();
                bool done = pThis->closed;
                if (done) {
                        Pool p;
                        pThis->writeStatus(newClient, LOG4CXX_STR("Log closed.\r\n"), p);
                        newClient->close();
                        return NULL;
                }

                size_t count = pThis->activeConnections;
                if (count >= pThis->connections.size()) {
                        Pool p;
                        pThis->writeStatus(newClient, LOG4CXX_STR("Too many connections.\r\n"), p);
                        newClient->close();
                } else {
                        //
                        //   find unoccupied connection
                        //
                        synchronized sync(pThis->mutex);
                        for(ConnectionList::iterator iter = pThis->connections.begin();
                                iter != pThis->connections.end();
                                iter++) {
                                if (*iter == NULL) {
                                    *iter = newClient;
                                    pThis->activeConnections++;
                                    break;
                                }
                        }

                        Pool p;
                        LogString oss(LOG4CXX_STR("TelnetAppender v1.0 ("));
                        StringHelper::toString((int) count+1, p, oss);
                        oss += LOG4CXX_STR(" active connections)\r\n\r\n");
                        pThis->writeStatus(newClient, oss, p);
                }
        } catch(InterruptedIOException &e) {
                if (pThis->closed) {
                    return NULL;
                }
        } catch(Exception& e) {
                if (!pThis->closed) {
                    LogLog::error(LOG4CXX_STR("Encountered error while in SocketHandler loop."), e);
                } else {
                    return NULL;
                }
        }
    }

    return NULL;
}

#endif
