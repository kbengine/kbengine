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

#ifndef _LOG4CXX_ASYNC_APPENDER_H
#define _LOG4CXX_ASYNC_APPENDER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/appenderattachableimpl.h>
#include <deque>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/thread.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/condition.h>


namespace log4cxx
{

        /**
        The AsyncAppender lets users log events asynchronously. It uses a
        bounded buffer to store logging events.

        <p>The AsyncAppender will collect the events sent to it and then
        dispatch them to all the appenders that are attached to it. You can
        attach multiple appenders to an AsyncAppender.

        <p>The AsyncAppender uses a separate thread to serve the events in
        its bounded buffer.

        <p><b>Important note:</b> The <code>AsyncAppender</code> can only
        be script configured using the {@link xml::DOMConfigurator DOMConfigurator}.
        */
        class LOG4CXX_EXPORT AsyncAppender :
                public virtual spi::AppenderAttachable,
                public virtual AppenderSkeleton
        {
        public:
                DECLARE_LOG4CXX_OBJECT(AsyncAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(AsyncAppender)
                        LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                        LOG4CXX_CAST_ENTRY(spi::AppenderAttachable)
                END_LOG4CXX_CAST_MAP()

                /**
                 * Create new instance.
                */
                AsyncAppender();
                
                /**
                 *  Destructor.
                 */
                virtual ~AsyncAppender();

                void addRef() const;
                void releaseRef() const;

                /**
                 * Add appender.
                 *
                 * @param newAppender appender to add, may not be null.
                */
                void addAppender(const AppenderPtr& newAppender);

                void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);

                /**
                Close this <code>AsyncAppender</code> by interrupting the
                dispatcher thread which will process all pending events before
                exiting.
                */
                void close();

                /**
                 * Get iterator over attached appenders.
                 * @return list of all attached appenders.
                */
                AppenderList getAllAppenders() const;
                
                /**
                 * Get appender by name.
                 *
                 * @param name name, may not be null.
                 * @return matching appender or null.
                */
                AppenderPtr getAppender(const LogString& name) const;

                /**
                 * Gets whether the location of the logging request call
                 * should be captured.
                 *
                 * @return the current value of the <b>LocationInfo</b> option.
                */
                bool getLocationInfo() const;
                /**
                * Determines if specified appender is attached.
                * @param appender appender.
                * @return true if attached.
                */
                bool isAttached(const AppenderPtr& appender) const;

                virtual bool requiresLayout() const;
                    
                /**
                 * Removes and closes all attached appenders.
                */
                void removeAllAppenders();

                /**
                 * Removes an appender.
                 * @param appender appender to remove.
                */
                void removeAppender(const AppenderPtr& appender);
                /**
                * Remove appender by name.
                * @param name name.
                */
                void removeAppender(const LogString& name);                        

                /**
                * The <b>LocationInfo</b> attribute is provided for compatibility
                * with log4j and has no effect on the log output.
                * @param flag new value.
                */
                void setLocationInfo(bool flag);

                /**
                * The <b>BufferSize</b> option takes a non-negative integer value.
                * This integer value determines the maximum size of the bounded
                * buffer.
                * */
                void setBufferSize(int size);

                /**
                 * Gets the current buffer size.
                 * @return the current value of the <b>BufferSize</b> option.
                */
                int getBufferSize() const;

                /**
                 * Sets whether appender should wait if there is no
                 * space available in the event buffer or immediately return.
                 *
                 * @param value true if appender should wait until available space in buffer.
                 */
                 void setBlocking(bool value);

                /**
                 * Gets whether appender should block calling thread when buffer is full.
                 * If false, messages will be counted by logger and a summary
                 * message appended after the contents of the buffer have been appended.
                 *
                 * @return true if calling thread will be blocked when buffer is full.
                 */
                 bool getBlocking() const;
                 
                 
                 /**
                  * Set appender properties by name.
                  * @param option property name.
                  * @param value property value.
                  */
                 void setOption(const LogString& option, const LogString& value);


        private:
                AsyncAppender(const AsyncAppender&);
                AsyncAppender& operator=(const AsyncAppender&);
                /**
                 * The default buffer size is set to 128 events.
                */
                enum { DEFAULT_BUFFER_SIZE = 128 };

                /**
                 * Event buffer.
                */
                LOG4CXX_LIST_DEF(LoggingEventList, log4cxx::spi::LoggingEventPtr);
                LoggingEventList buffer;

                /**
                 *  Mutex used to guard access to buffer and discardMap.
                 */
                ::log4cxx::helpers::Mutex bufferMutex;
                ::log4cxx::helpers::Condition bufferNotFull;
                ::log4cxx::helpers::Condition bufferNotEmpty;
    
                class DiscardSummary {
                private:
                    /**
                     * First event of the highest severity.
                    */
                    ::log4cxx::spi::LoggingEventPtr maxEvent;
                    
                    /**
                    * Total count of messages discarded.
                    */
                    int count;
                    
                public:
                    /**
                     * Create new instance.
                     *
                     * @param event event, may not be null.
                    */
                    DiscardSummary(const ::log4cxx::spi::LoggingEventPtr& event);
                    /** Copy constructor.  */
                    DiscardSummary(const DiscardSummary& src);
                    /** Assignment operator. */
                    DiscardSummary& operator=(const DiscardSummary& src);
                    
                    /**
                     * Add discarded event to summary.
                     *
                     * @param event event, may not be null.
                    */
                    void add(const ::log4cxx::spi::LoggingEventPtr& event);
                    
                    /**
                     * Create event with summary information.
                     *
                     * @return new event.
                     */
                     ::log4cxx::spi::LoggingEventPtr createEvent(::log4cxx::helpers::Pool& p);
                };

                /**
                  * Map of DiscardSummary objects keyed by logger name.
                */
                typedef std::map<LogString, DiscardSummary> DiscardMap;
                DiscardMap* discardMap;
                
                /**
                 * Buffer size.
                */
                int bufferSize;

                /**
                 * Nested appenders.
                */
                helpers::AppenderAttachableImplPtr appenders;

                /**
                 *  Dispatcher.
                 */
                helpers::Thread dispatcher;

                /**
                 * Should location info be included in dispatched messages.
                */
                bool locationInfo;

                /**
                 * Does appender block when buffer is full.
                */
                bool blocking;

                /**
                 *  Dispatch routine.
                 */
                static void* LOG4CXX_THREAD_FUNC dispatch(apr_thread_t* thread, void* data);

        }; // class AsyncAppender
        LOG4CXX_PTR_DEF(AsyncAppender);
}  //  namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif//  _LOG4CXX_ASYNC_APPENDER_H

