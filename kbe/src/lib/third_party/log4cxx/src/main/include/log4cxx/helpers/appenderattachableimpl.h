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

#ifndef _LOG4CXX_HELPERS_APPENDER_ATTACHABLE_IMPL_H
#define _LOG4CXX_HELPERS_APPENDER_ATTACHABLE_IMPL_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/spi/appenderattachable.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/pool.h>

namespace log4cxx
{
    namespace spi
    {
        class LoggingEvent;
        typedef helpers::ObjectPtrT<LoggingEvent> LoggingEventPtr;
    }

    namespace helpers
    {

        class LOG4CXX_EXPORT AppenderAttachableImpl :
            public virtual spi::AppenderAttachable,
            public virtual helpers::ObjectImpl
        {
        protected:
            /** Array of appenders. */
            AppenderList  appenderList;

        public:            
            /**
             *   Create new instance.
             *   @param pool pool, must be longer-lived than instance. 
             */
            AppenderAttachableImpl(Pool& pool);

            DECLARE_ABSTRACT_LOG4CXX_OBJECT(AppenderAttachableImpl)
            BEGIN_LOG4CXX_CAST_MAP()
                LOG4CXX_CAST_ENTRY(AppenderAttachableImpl)
                LOG4CXX_CAST_ENTRY(spi::AppenderAttachable)
            END_LOG4CXX_CAST_MAP()

            void addRef() const;
            void releaseRef() const;

                  // Methods
            /**
             * Add an appender.
             */
            virtual void addAppender(const AppenderPtr& newAppender);

            /**
             Call the <code>doAppend</code> method on all attached appenders.
            */
            int appendLoopOnAppenders(const spi::LoggingEventPtr& event,
                log4cxx::helpers::Pool& p);

            /**
             * Get all previously added appenders as an Enumeration.
             */
            virtual AppenderList getAllAppenders() const;

            /**
             * Get an appender by name.
             */
            virtual AppenderPtr getAppender(const LogString& name) const;

            /**
             Returns <code>true</code> if the specified appender is in the
             list of attached appenders, <code>false</code> otherwise.
            */
            virtual bool isAttached(const AppenderPtr& appender) const;

            /**
             * Remove all previously added appenders.
             */
            virtual void removeAllAppenders();

            /**
             * Remove the appender passed as parameter from the list of appenders.
             */
            virtual void removeAppender(const AppenderPtr& appender);

            /**
             * Remove the appender with the name passed as parameter from the
             * list of appenders.
             */
            virtual void removeAppender(const LogString& name);

            inline const log4cxx::helpers::Mutex& getMutex() const { return mutex; }

        private:
            log4cxx::helpers::Mutex mutex;
            AppenderAttachableImpl(const AppenderAttachableImpl&);
            AppenderAttachableImpl& operator=(const AppenderAttachableImpl&);
        };

        LOG4CXX_PTR_DEF(AppenderAttachableImpl);

    }
}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif //_LOG4CXX_HELPERS_APPENDER_ATTACHABLE_IMPL_H
