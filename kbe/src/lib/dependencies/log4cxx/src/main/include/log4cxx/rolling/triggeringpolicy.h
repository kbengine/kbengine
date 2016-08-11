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


#if !defined(_LOG4CXX_ROLLING_TRIGGER_POLICY_H)
#define _LOG4CXX_ROLLING_TRIGGER_POLICY_H


#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/appender.h>

namespace log4cxx {
    class File;

    namespace rolling {

        /**
         * A <code>TriggeringPolicy</code> controls the conditions under which rollover
         * occurs. Such conditions include time of day, file size, an
         * external event or a combination thereof.
         *
         * 
         * 
         * */

        class LOG4CXX_EXPORT TriggeringPolicy :
              public virtual spi::OptionHandler,
              public virtual helpers::ObjectImpl {
              DECLARE_ABSTRACT_LOG4CXX_OBJECT(TriggeringPolicy)
              BEGIN_LOG4CXX_CAST_MAP()
                      LOG4CXX_CAST_ENTRY(TriggeringPolicy)
                      LOG4CXX_CAST_ENTRY(spi::OptionHandler)
              END_LOG4CXX_CAST_MAP()
        public:
			TriggeringPolicy() {}
             virtual ~TriggeringPolicy();
             void addRef() const;
             void releaseRef() const;

			 // added for VS2015
			 #if _MSC_VER >= 1900
			 TriggeringPolicy(TriggeringPolicy && o)
				 : helpers::ObjectImpl(std::move(o))
			 { }

			 TriggeringPolicy& operator=(TriggeringPolicy && o)
			 {
				 helpers::ObjectImpl::operator=(std::move(o));
				 return *this;
			 }
			 #endif
			 // end of added for VS2015

            /**
             * Determines if a rollover may be appropriate at this time.  If
             * true is returned, RolloverPolicy.rollover will be called but it
             * can determine that a rollover is not warranted.
             *
             * @param appender A reference to the appender.
             * @param event A reference to the currently event.
             * @param filename The filename for the currently active log file.
             * @param fileLength Length of the file in bytes.
             * @return true if a rollover should occur.
             */
            virtual bool isTriggeringEvent(
              Appender* appender,
              const log4cxx::spi::LoggingEventPtr& event,
              const LogString& filename,
              size_t fileLength) = 0;

        };

        LOG4CXX_PTR_DEF(TriggeringPolicy);


    }
}

#endif
