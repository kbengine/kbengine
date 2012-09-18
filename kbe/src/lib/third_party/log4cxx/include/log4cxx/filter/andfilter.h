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

#ifndef _LOG4CXX_FILTER_ANDFILTER_H
#define _LOG4CXX_FILTER_ANDFILTER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/spi/filter.h>

namespace log4cxx
{
    namespace filter
    {

/**
 * A filter that 'and's the results of any number of contained filters together.
 *
 * For the filter to process events, all contained filters must return Filter::ACCEPT.
 *
 * If the contained filters do not return Filter::ACCEPT, Filter::NEUTRAL is returned.
 *
 * If acceptOnMatch is set to true, Filter::ACCEPT is returned.
 * If acceptOnMatch is set to false, Filter::DENY is returned.
 *
 * Here is an example config that will accept only events that contain BOTH
 * a DEBUG level AND 'test' in the message:
 *
 *&lt;appender name="STDOUT" class="org.apache.log4j.ConsoleAppender"&gt;
 * &lt;filter class="org.apache.log4j.filter.AndFilter"&gt;
 *  &lt;filter class="org.apache.log4j.filter.LevelMatchFilter"&gt;
 *        &lt;param name="levelToMatch" value="DEBUG" /&gt;
 *        &lt;param name="acceptOnMatch" value="true" /&gt;
 *  &lt;/filter>
 *  &lt;filter class="org.apache.log4j.filter.StringMatchFilter"&gt;
 *        &lt;param name="stringToMatch" value="test" /&gt;
 *        &lt;param name="acceptOnMatch" value="true" /&gt;
 *  &lt;/filter>
 *  &lt;param name="acceptOnMatch" value="false"/&gt;
 * &lt;/filter&gt;
 * &lt;filter class="org.apache.log4j.filter.DenyAllFilter"/&gt;
 *&lt;layout class="org.apache.log4j.SimpleLayout"/&gt;
 *&lt;/appender&gt;
 *
 * To accept all events EXCEPT those events that contain a
 * DEBUG level and 'test' in the message:
 * change the AndFilter's acceptOnMatch param to false and remove the DenyAllFilter
 *
 * NOTE: If you are defining a filter that is only relying on logging event content
 * (no external or filter-managed state), you could opt instead
 * to use an ExpressionFilter with one of the following expressions:
 *
 * LEVEL == DEBUG && MSG ~= 'test'
 * or
 * ! ( LEVEL == DEBUG && MSG ~= 'test' )
 *
 * 
 */
        class LOG4CXX_EXPORT AndFilter:public log4cxx::spi::Filter
        {
          private:
            log4cxx::spi::FilterPtr headFilter;
            log4cxx::spi::FilterPtr tailFilter;
            bool acceptOnMatch;
                 AndFilter(const AndFilter &);
                  AndFilter & operator=(const AndFilter &);


          public:
                  DECLARE_LOG4CXX_OBJECT(AndFilter)
                  BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(log4cxx::spi::Filter)
                  END_LOG4CXX_CAST_MAP()

                  AndFilter();

            void addFilter(const log4cxx::spi::FilterPtr & filter);

            void setAcceptOnMatch(bool acceptOnMatch);

            FilterDecision decide(const spi::LoggingEventPtr & event) const;
        };

    }
}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif
