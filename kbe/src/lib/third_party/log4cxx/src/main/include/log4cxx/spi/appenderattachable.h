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

#ifndef _LOG4CXX_SPI_APPENDER_ATTACHABLE_H_
#define _LOG4CXX_SPI_APPENDER_ATTACHABLE_H_

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/logstring.h>
#include <vector>
#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/helpers/object.h>
#include <log4cxx/appender.h>

namespace log4cxx
{

    namespace spi
    {

        /**
         * This Interface is for attaching Appenders to objects.
         */
        class LOG4CXX_EXPORT AppenderAttachable : public virtual helpers::Object
        {
        public:
          // Methods
         DECLARE_ABSTRACT_LOG4CXX_OBJECT(AppenderAttachable)
            /**
             * Add an appender.
             */
            virtual void addAppender(const AppenderPtr& newAppender) = 0;

            /**
             * Get all previously added appenders as an AppenderList.
             */
            virtual AppenderList getAllAppenders() const = 0;

            /**
             * Get an appender by name.
             */
            virtual AppenderPtr getAppender(const LogString& name) const = 0;

            /**
         Returns <code>true</code> if the specified appender is in list of
         attached attached, <code>false</code> otherwise.
         */
         virtual bool isAttached(const AppenderPtr& appender) const = 0;

            /**
             * Remove all previously added appenders.
             */
            virtual void removeAllAppenders() = 0;

            /**
             * Remove the appender passed as parameter from the list of appenders.
             */
            virtual void removeAppender(const AppenderPtr& appender) = 0;

            /**
             * Remove the appender with the name passed as parameter from the
             * list of appenders.
             */
            virtual void removeAppender(const LogString& name) = 0;

          // Dtor
            virtual ~AppenderAttachable(){}
        };


        LOG4CXX_PTR_DEF(AppenderAttachable);
        
    }
}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif //_LOG4CXX_SPI_APPENDER_ATTACHABLE_H_
