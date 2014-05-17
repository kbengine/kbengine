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

#ifndef _LOG4CXX_FILTER_LEVEL_MATCH_FILTER_H
#define _LOG4CXX_FILTER_LEVEL_MATCH_FILTER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/spi/filter.h>
#include <log4cxx/level.h>

namespace log4cxx
{
        class Level;

        namespace filter
        {
                /**
                This is a very simple filter based on level matching.

                <p>The filter admits two options <b>LevelToMatch</b> and
                <b>AcceptOnMatch</b>. If there is an exact match between the value
                of the <b>LevelToMatch</b> option and the level of the {@link
                spi::LoggingEvent LoggingEvent}, then the #decide method returns {@link
                spi::Filter#ACCEPT ACCEPT} in case the <b>AcceptOnMatch</b>
                option value is set to <code>true</code>, if it is <code>false</code>
                then {@link spi::Filter#DENY DENY} is returned. If there is no match,
                {@link spi::Filter#NEUTRAL NEUTRAL} is returned.
                */

                class LOG4CXX_EXPORT LevelMatchFilter : public spi::Filter
                {
                private:
                        bool acceptOnMatch;
                        LevelPtr levelToMatch;

                public:
                        typedef spi::Filter BASE_CLASS;
                        DECLARE_LOG4CXX_OBJECT(LevelMatchFilter)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(LevelMatchFilter)
                                LOG4CXX_CAST_ENTRY_CHAIN(BASE_CLASS)
                        END_LOG4CXX_CAST_MAP()

                        LevelMatchFilter();

                        /**
                        Set options
                        */
                        virtual void setOption(const LogString& option,
                                const LogString& value);

                        void setLevelToMatch(const LogString& levelToMatch);

                        LogString getLevelToMatch() const;

                        inline void setAcceptOnMatch(bool acceptOnMatch1)
                                { this->acceptOnMatch = acceptOnMatch1; }

                        inline bool getAcceptOnMatch() const
                                { return acceptOnMatch; }

                        /**
                        Return the decision of this filter.

                        Returns {@link spi::Filter#NEUTRAL NEUTRAL} if the
                        <b>LevelToMatch</b> option is not set or if there is not match.
                        Otherwise, if there is a match, then the returned decision is
                        {@link spi::Filter#ACCEPT ACCEPT} if the <b>AcceptOnMatch</b>
                        property is set to <code>true</code>. The returned decision is
                        {@link spi::Filter#DENY DENY} if the
                        <b>AcceptOnMatch</b> property is set to false.
                        */
                        FilterDecision decide(const spi::LoggingEventPtr& event) const;
                }; // class LevelMatchFilter
            LOG4CXX_PTR_DEF(LevelMatchFilter);
        }  // namespace filter
} // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif // _LOG4CXX_FILTER_STRING_MATCH_FILTER_H
