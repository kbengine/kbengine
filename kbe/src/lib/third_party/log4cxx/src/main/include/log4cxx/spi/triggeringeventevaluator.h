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

#ifndef _LOG4CXX_SPI_TRIGGERING_EVENT_EVALUATOR_H
#define _LOG4CXX_SPI_TRIGGERING_EVENT_EVALUATOR_H

#include <log4cxx/spi/loggingevent.h>

namespace log4cxx
{
        namespace spi
        {
                /**
                Implementions of this interface allow certain appenders to decide
                when to perform an appender specific action.

                <p>For example the {@link net::SMTPAppender SMTPAppender} sends
                an email when the #isTriggeringEvent method returns
                <code>true</code> and adds the event to an internal buffer when the
                returned result is <code>false</code>.

                */
                class LOG4CXX_EXPORT TriggeringEventEvaluator : public virtual helpers::Object
                {
                public:
                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(TriggeringEventEvaluator)
                        /**
                        Is this the triggering event?
                        */
                        virtual bool isTriggeringEvent(const spi::LoggingEventPtr& event) = 0;
                };
            LOG4CXX_PTR_DEF(TriggeringEventEvaluator);
        }
}

#endif // _LOG4CXX_SPI_TRIGGERING_EVENT_EVALUATOR_H
