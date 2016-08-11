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

#ifndef _LOG4CXX_SPI_FILTER_H
#define _LOG4CXX_SPI_FILTER_H

#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/spi/loggingevent.h>

namespace log4cxx
{
        namespace spi
        {
                class Filter;
                LOG4CXX_PTR_DEF(Filter);


        /**
        Users should extend this class to implement customized logging
        event filtering. Note that Logger and
        AppenderSkeleton, the parent class of all standard
        appenders, have built-in filtering rules. It is suggested that you
        first use and understand the built-in rules before rushing to write
        your own custom filters.

        <p>This abstract class assumes and also imposes that filters be
        organized in a linear chain. The {@link #decide
        decide(LoggingEvent)} method of each filter is called sequentially,
        in the order of their addition to the chain.

        <p>The {@link #decide decide(LoggingEvent)} method must return one
        of the integer constants #DENY, #NEUTRAL or
        #ACCEPT.

        <p>If the value #DENY is returned, then the log event is
        dropped immediately without consulting with the remaining
        filters.

        <p>If the value #NEUTRAL is returned, then the next filter
        in the chain is consulted. If there are no more filters in the
        chain, then the log event is logged. Thus, in the presence of no
        filters, the default behaviour is to log all logging events.

        <p>If the value #ACCEPT is returned, then the log
        event is logged without consulting the remaining filters.

        <p>The philosophy of log4cxx filters is largely inspired from the
        Linux ipchains.

        <p>Note that filtering is only supported by the {@link
        xml::DOMConfigurator DOMConfigurator}.
        */
                class LOG4CXX_EXPORT Filter : public virtual OptionHandler,
                        public virtual helpers::ObjectImpl
                {
                  /**
                  Points to the next filter in the filter chain.
                  */
                  FilterPtr next;
                public:
                        Filter();

						// added for VS2015
						#if _MSC_VER >= 1900
						Filter(Filter && o)
							: helpers::ObjectImpl(std::move(o))
							, next(o.next)
						{ }

						Filter& operator=(Filter && o)
						{
							helpers::ObjectImpl::operator=(std::move(o));
							next = o.next;
							return *this;
						}
						#endif
						// end of added for VS2015

                        void addRef() const;
                        void releaseRef() const;

                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(Filter)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(Filter)
                                LOG4CXX_CAST_ENTRY(spi::OptionHandler)
                        END_LOG4CXX_CAST_MAP()

                        log4cxx::spi::FilterPtr getNext() const;
                        void setNext(const log4cxx::spi::FilterPtr& newNext);
 
            enum FilterDecision
            {
            /**
            The log event must be dropped immediately without consulting
                        with the remaining filters, if any, in the chain.  */
                        DENY = -1,
            /**
            This filter is neutral with respect to the log event. The
            remaining filters, if any, should be consulted for a final decision.
            */
                        NEUTRAL = 0,
            /**
            The log event must be logged immediately without consulting with
            the remaining filters, if any, in the chain.
                        */
                        ACCEPT = 1

                        };


            /**
            Usually filters options become active when set. We provide a

            default do-nothing implementation for convenience.
            */
            void activateOptions(log4cxx::helpers::Pool& p);
            void setOption(const LogString& option, const LogString& value);

            /**
            <p>If the decision is <code>DENY</code>, then the event will be
            dropped. If the decision is <code>NEUTRAL</code>, then the next
            filter, if any, will be invoked. If the decision is ACCEPT then
            the event will be logged without consulting with other filters in
            the chain.

            @param event The LoggingEvent to decide upon.
            @return The decision of the filter.  */
            virtual FilterDecision decide(const LoggingEventPtr& event) const = 0;
                };
        }
}

#endif //_LOG4CXX_SPI_FILTER_H
