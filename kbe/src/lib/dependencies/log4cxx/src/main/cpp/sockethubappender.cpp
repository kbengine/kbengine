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

#include <log4cxx/net/sockethubappender.h>

#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/serversocket.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/synchronized.h>
#include <apr_atomic.h>
#include <apr_thread_proc.h>
#include <log4cxx/helpers/objectoutputstream.h>
#include <log4cxx/helpers/socketoutputstream.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;
using namespace log4cxx::spi;

#if APR_HAS_THREADS

IMPLEMENT_LOG4CXX_OBJECT(SocketHubAppender)

int SocketHubAppender::DEFAULT_PORT = 4560;

SocketHubAppender::~SocketHubAppender()
{
        finalize();
}

SocketHubAppender::SocketHubAppender()
 : port(DEFAULT_PORT), streams(), locationInfo(false), thread()
{
}

SocketHubAppender::SocketHubAppender(int port1)
 : port(port1), streams(), locationInfo(false), thread()
{
        startServer();
}

void SocketHubAppender::activateOptions(Pool& /* p */ )
{
        startServer();
}

void SocketHubAppender::setOption(const LogString& option,
        const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PORT"), LOG4CXX_STR("port")))
        {
                setPort(OptionConverter::toInt(value, DEFAULT_PORT));
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo")))
        {
                setLocationInfo(OptionConverter::toBoolean(value, true));
        }
        else
        {
                AppenderSkeleton::setOption(option, value);
        }
}


void SocketHubAppender::close()
{
        {
            synchronized sync(mutex);
            if (closed) {
                return;
            }
            closed = true;
        }

        LogLog::debug(LOG4CXX_STR("closing SocketHubAppender ") + getName());
        //
        //  wait until the server thread completes
        //
        thread.join();

        synchronized sync(mutex);
        // close all of the connections
        LogLog::debug(LOG4CXX_STR("closing client connections"));
        for (std::vector<helpers::ObjectOutputStreamPtr>::iterator iter = streams.begin();
             iter != streams.end();
             iter++) {
                 if ( (*iter) != NULL) {
                         try {
                                (*iter)->close(pool);
                         } catch(SocketException& e) {
                                LogLog::error(LOG4CXX_STR("could not close socket: "), e);
                         }
                 }
         }
        streams.erase(streams.begin(), streams.end());


        LogLog::debug(LOG4CXX_STR("SocketHubAppender ")
              + getName() + LOG4CXX_STR(" closed"));
}

void SocketHubAppender::append(const spi::LoggingEventPtr& event, Pool& p)
{

        // if no open connections, exit now
        if(streams.empty())
        {
                return;
        }
        
        LogString ndcVal;
        event->getNDC(ndcVal);
        event->getThreadName();
        // Get a copy of this thread's MDC.
        event->getMDCCopy();
       

        // loop through the current set of open connections, appending the event to each
        std::vector<ObjectOutputStreamPtr>::iterator it = streams.begin();
        std::vector<ObjectOutputStreamPtr>::iterator itEnd = streams.end();
        while(it != itEnd)
        {
                // list size changed unexpectedly? Just exit the append.
                if (*it == 0)
                {
                        break;
                }

                try
                {
                        event->write(**it, p);
                        (*it)->flush(p);
                        it++;
                }
                catch(std::exception& e)
                {
                        // there was an io exception so just drop the connection
                        it = streams.erase(it);
                        LogLog::debug(LOG4CXX_STR("dropped connection"), e);
                }
        }
}

void SocketHubAppender::startServer()
{
        thread.run(monitor, this);
}

void* APR_THREAD_FUNC SocketHubAppender::monitor(apr_thread_t* /* thread */, void* data) {
        SocketHubAppender* pThis = (SocketHubAppender*) data;

        ServerSocket* serverSocket = 0;

        try
        {
                serverSocket = new ServerSocket(pThis->port);
                serverSocket->setSoTimeout(1000);
        }
        catch (SocketException& e)
        {
                LogLog::error(LOG4CXX_STR("exception setting timeout, shutting down server socket."), e);
                delete serverSocket;
                return NULL;
        }

        bool stopRunning = pThis->closed;
        while (!stopRunning)
        {
                SocketPtr socket;
                try
                {
                        socket = serverSocket->accept();
                }
                catch (InterruptedIOException&)
                {
                        // timeout occurred, so just loop
                }
                catch (SocketException& e)
                {
                        LogLog::error(LOG4CXX_STR("exception accepting socket, shutting down server socket."), e);
                        stopRunning = true;
                }
                catch (IOException& e)
                {
                        LogLog::error(LOG4CXX_STR("exception accepting socket."), e);
                }

                // if there was a socket accepted
                if (socket != 0)
                {
                        try
                        {
                                InetAddressPtr remoteAddress = socket->getInetAddress();
                                LogLog::debug(LOG4CXX_STR("accepting connection from ")
                                       + remoteAddress->getHostName()
                                       + LOG4CXX_STR(" (")
                                       + remoteAddress->getHostAddress()
                                       + LOG4CXX_STR(")"));

                                // add it to the oosList.
                                synchronized sync(pThis->mutex);
                                OutputStreamPtr os(new SocketOutputStream(socket));
                                Pool p;
                                ObjectOutputStreamPtr oos(new ObjectOutputStream(os, p));
                                pThis->streams.push_back(oos);
                        }
                        catch (IOException& e)
                        {
                                LogLog::error(LOG4CXX_STR("exception creating output stream on socket."), e);
                        }
                }
                stopRunning = (stopRunning || pThis->closed);
        }
        delete serverSocket;
        return NULL;
}

#endif
