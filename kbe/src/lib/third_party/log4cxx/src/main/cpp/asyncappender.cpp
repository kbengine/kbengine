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

#include <log4cxx/asyncappender.h>


#include <log4cxx/helpers/loglog.h>
#include <log4cxx/spi/loggingevent.h>
#include <apr_thread_proc.h>
#include <apr_thread_mutex.h>
#include <apr_thread_cond.h>
#include <log4cxx/helpers/condition.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/stringhelper.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/optionconverter.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;


IMPLEMENT_LOG4CXX_OBJECT(AsyncAppender)


AsyncAppender::AsyncAppender()
: AppenderSkeleton(),
  buffer(),
  bufferMutex(pool),
  bufferNotFull(pool),
  bufferNotEmpty(pool),
  discardMap(new DiscardMap()),
  bufferSize(DEFAULT_BUFFER_SIZE),
  appenders(new AppenderAttachableImpl(pool)),
  dispatcher(),
  locationInfo(false),
  blocking(true) {
#if APR_HAS_THREADS
  dispatcher.run(dispatch, this);
#endif
}

AsyncAppender::~AsyncAppender()
{
        finalize();
        delete discardMap;
}

void AsyncAppender::addRef() const {
    ObjectImpl::addRef();
}

void AsyncAppender::releaseRef() const {
    ObjectImpl::releaseRef();
}

void AsyncAppender::addAppender(const AppenderPtr& newAppender)
{
        synchronized sync(appenders->getMutex());
        appenders->addAppender(newAppender);
}


void AsyncAppender::setOption(const LogString& option,
        const LogString& value) {
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("LOCATIONINFO"), LOG4CXX_STR("locationinfo"))) {
             setLocationInfo(OptionConverter::toBoolean(value, false));
        }
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize"))) {
             setBufferSize(OptionConverter::toInt(value, DEFAULT_BUFFER_SIZE));
        }
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BLOCKING"), LOG4CXX_STR("blocking"))) {
             setBlocking(OptionConverter::toBoolean(value, true));
        } else {
             AppenderSkeleton::setOption(option, value);
        }
}


void AsyncAppender::append(const spi::LoggingEventPtr& event, Pool& p) {
#if APR_HAS_THREADS
       //
        //   if dispatcher has died then
        //      append subsequent events synchronously
        //
        if (!dispatcher.isAlive() || bufferSize <= 0) {
            synchronized sync(appenders->getMutex());
            appenders->appendLoopOnAppenders(event, p);
            return;
        }

        // Set the NDC and thread name for the calling thread as these
        // LoggingEvent fields were not set at event creation time.
        LogString ndcVal;
        event->getNDC(ndcVal);
        event->getThreadName();
        // Get a copy of this thread's MDC.
        event->getMDCCopy();


        {
             synchronized sync(bufferMutex);
             while(true) {
                 int previousSize = buffer.size();
                 if (previousSize < bufferSize) {
                     buffer.push_back(event);
                     if (previousSize == 0) {
                        bufferNotEmpty.signalAll();
                     }
                     break;
                 }
             
                //
                //   Following code is only reachable if buffer is full
                //
                //
                //   if blocking and thread is not already interrupted
                //      and not the dispatcher then
                //      wait for a buffer notification
                bool discard = true;
                if (blocking
                    && !Thread::interrupted()
                    && !dispatcher.isCurrentThread()) {
                    try {
                        bufferNotFull.await(bufferMutex);
                        discard = false;
                    } catch (InterruptedException& e) {
                        //
                        //  reset interrupt status so
                        //    calling code can see interrupt on
                        //    their next wait or sleep.
                        Thread::currentThreadInterrupt();
                    }
                }

                //
                //   if blocking is false or thread has been interrupted
                //   add event to discard map.
                //
                if (discard) {
                    LogString loggerName = event->getLoggerName();
                    DiscardMap::iterator iter = discardMap->find(loggerName);
                    if (iter == discardMap->end()) {
                        DiscardSummary summary(event);
                        discardMap->insert(DiscardMap::value_type(loggerName, summary));
                    } else {
                        (*iter).second.add(event);
                    }
                    break;
                }
            }
        }
#else
        synchronized sync(appenders->getMutex());
        appenders->appendLoopOnAppenders(event, p);
#endif
  }
  

void AsyncAppender::close() {
    {
        synchronized sync(bufferMutex);
        closed = true;
        bufferNotEmpty.signalAll();
        bufferNotFull.signalAll();
    }
    
#if APR_HAS_THREADS
    try {
        dispatcher.join();
   } catch(InterruptedException& e) {
        Thread::currentThreadInterrupt();
        LogLog::error(LOG4CXX_STR("Got an InterruptedException while waiting for the dispatcher to finish,"), e);
    }
#endif
    
    {
        synchronized sync(appenders->getMutex());
        AppenderList appenderList = appenders->getAllAppenders();
        for (AppenderList::iterator iter = appenderList.begin();
             iter != appenderList.end();
             iter++) {
             (*iter)->close();
        }
    }
}

AppenderList AsyncAppender::getAllAppenders() const
{
        synchronized sync(appenders->getMutex());
        return appenders->getAllAppenders();
}

AppenderPtr AsyncAppender::getAppender(const LogString& name) const
{
        synchronized sync(appenders->getMutex());
        return appenders->getAppender(name);
}

bool AsyncAppender::isAttached(const AppenderPtr& appender) const
{
        synchronized sync(appenders->getMutex());
        return appenders->isAttached(appender);
}

bool AsyncAppender::requiresLayout() const {
    return false;
}

void AsyncAppender::removeAllAppenders()
{
    synchronized sync(appenders->getMutex());
    appenders->removeAllAppenders();
}

void AsyncAppender::removeAppender(const AppenderPtr& appender)
{
    synchronized sync(appenders->getMutex());
    appenders->removeAppender(appender);
}

void AsyncAppender::removeAppender(const LogString& name)
{
    synchronized sync(appenders->getMutex());
    appenders->removeAppender(name);
}

bool AsyncAppender::getLocationInfo() const {
    return locationInfo;
}

void AsyncAppender::setLocationInfo(bool flag) {
    locationInfo = flag;
}


void AsyncAppender::setBufferSize(int size)
{
    if (size < 0) {
          throw IllegalArgumentException(LOG4CXX_STR("size argument must be non-negative"));
    }
    synchronized sync(bufferMutex);
    bufferSize = (size < 1) ? 1 : size;
    bufferNotFull.signalAll();
}

int AsyncAppender::getBufferSize() const
{
        return bufferSize;
}

void AsyncAppender::setBlocking(bool value) {
    synchronized sync(bufferMutex);
    blocking = value;
    bufferNotFull.signalAll();
}

bool AsyncAppender::getBlocking() const {
    return blocking;
}

AsyncAppender::DiscardSummary::DiscardSummary(const LoggingEventPtr& event) : 
      maxEvent(event), count(1) {
}

AsyncAppender::DiscardSummary::DiscardSummary(const DiscardSummary& src) : 
      maxEvent(src.maxEvent), count(src.count) {
}

AsyncAppender::DiscardSummary& AsyncAppender::DiscardSummary::operator=(const DiscardSummary& src) {
      maxEvent = src.maxEvent;
      count = src.count; 
      return *this;
}

void AsyncAppender::DiscardSummary::add(const LoggingEventPtr& event) {
      if (event->getLevel()->toInt() > maxEvent->getLevel()->toInt()) {
        maxEvent = event;
      }
      count++;
}

LoggingEventPtr AsyncAppender::DiscardSummary::createEvent(Pool& p) {
    LogString msg(LOG4CXX_STR("Discarded "));
    StringHelper::toString(count, p, msg);
    msg.append(LOG4CXX_STR(" messages due to a full event buffer including: "));
    msg.append(maxEvent->getMessage()); 
    return new LoggingEvent(   
              maxEvent->getLoggerName(),
              maxEvent->getLevel(),
              msg,
              LocationInfo::getLocationUnavailable());
}


#if APR_HAS_THREADS
void* LOG4CXX_THREAD_FUNC AsyncAppender::dispatch(apr_thread_t* thread, void* data) {
    AsyncAppender* pThis = (AsyncAppender*) data;
    bool isActive = true;
    try {
        while (isActive) {
             //
             //   process events after lock on buffer is released.
             //
            Pool p;
            LoggingEventList events;
            {
                   synchronized sync(pThis->bufferMutex);
                   size_t bufferSize = pThis->buffer.size();
                   isActive = !pThis->closed;
               
                   while((bufferSize == 0) && isActive) {
                       pThis->bufferNotEmpty.await(pThis->bufferMutex);
                       bufferSize = pThis->buffer.size();
                       isActive = !pThis->closed;
                   }
                   for(LoggingEventList::iterator eventIter = pThis->buffer.begin();
                       eventIter != pThis->buffer.end();
                       eventIter++) {
                       events.push_back(*eventIter);
                   }
                   for(DiscardMap::iterator discardIter = pThis->discardMap->begin();
                       discardIter != pThis->discardMap->end();
                       discardIter++) {
                       events.push_back(discardIter->second.createEvent(p));
                   }
                   pThis->buffer.clear();
                   pThis->discardMap->clear();
                   pThis->bufferNotFull.signalAll();
            }
            
            for (LoggingEventList::iterator iter = events.begin();
                 iter != events.end();
                 iter++) {
                 synchronized sync(pThis->appenders->getMutex());
                 pThis->appenders->appendLoopOnAppenders(*iter, p);
            }
        }
    } catch(InterruptedException& ex) {
            Thread::currentThreadInterrupt();
    } catch(...) {
    }
    return 0;
}
#endif                
