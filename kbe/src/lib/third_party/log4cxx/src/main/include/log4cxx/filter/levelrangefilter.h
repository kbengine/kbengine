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

#ifndef _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H
#define _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H

#include <log4cxx/spi/filter.h>
#include <log4cxx/level.h>

namespace log4cxx
{
        namespace filter
        {
                /**
                This is a very simple filter based on level matching, which can be
                used to reject messages with priorities outside a certain range.

                <p>The filter admits three options <b>LevelMin</b>, <b>LevelMax</b>
                and <b>AcceptOnMatch</b>.

                <p>If the level of the {@link spi::LoggingEvent LoggingEvent} is not
                between Min and Max (inclusive), then {@link spi::Filter#DENY DENY}
                is returned.

                <p> If the Logging event level is within the specified range, then if
                <b>AcceptOnMatch</b> is true, {@link spi::Filter#ACCEPT ACCEPT} is
                returned, and if <b>AcceptOnMatch</b> is false,
                {@link spi::Filter#NEUTRAL NEUTRAL} is returned.

                <p>If <code>LevelMin</code>w is not defined, then there is no
                minimum acceptable level (ie a level is never rejected for
                being too "low"/unimportant).  If <code>LevelMax</code> is not
                defined, then there is no maximum acceptable level (ie a
                level is never rejected for beeing too "high"/important).

                <p>Refer to the {@link
                AppenderSkeleton#setThreshold setThreshold} method
                available to <code>all</code> appenders extending
                AppenderSkeleton for a more convenient way to
                filter out events by level.
                */

                class LOG4CXX_EXPORT LevelRangeFilter : public spi::Filter
                {
                private:
                        /**
                        Do we return ACCEPT when a match occurs. Default is
                        <code>false</code>, so that later filters get run by default
                        */
                        bool acceptOnMatch;
                        LevelPtr levelMin;
                        LevelPtr levelMax;

                public:
                        typedef spi::Filter BASE_CLASS;
                        DECLARE_LOG4CXX_OBJECT(LevelRangeFilter)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(LevelRangeFilter)
                                LOG4CXX_CAST_ENTRY_CHAIN(BASE_CLASS)
                        END_LOG4CXX_CAST_MAP()

                        LevelRangeFilter();

                        /**
                        Set options
                        */
                        virtual void setOption(const LogString& option,
                                const LogString& value);

                        /**
                        Set the <code>LevelMin</code> option.
                        */
                        void setLevelMin(const LevelPtr& levelMin1)
                                { this->levelMin = levelMin1; }

                        /**
                        Get the value of the <code>LevelMin</code> option.
                        */
                        const LevelPtr& getLevelMin() const
                                { return levelMin; }

                        /**
                        Set the <code>LevelMax</code> option.
                        */
                        void setLevelMax(const LevelPtr& levelMax1)
                                { this->levelMax = levelMax1; }

                        /**
                        Get the value of the <code>LevelMax</code> option.
                        */
                        const LevelPtr& getLevelMax() const
                                { return levelMax; }

                        /**
                        Set the <code>AcceptOnMatch</code> option.
                        */
                        inline void setAcceptOnMatch(bool acceptOnMatch1)
                                { this->acceptOnMatch = acceptOnMatch1; }

                        /**
                        Get the value of the <code>AcceptOnMatch</code> option.
                        */
                        inline bool getAcceptOnMatch() const
                                { return acceptOnMatch; }

                        /**
                        Return the decision of this filter.

                        Returns {@link spi::Filter#NEUTRAL NEUTRAL} if the
                        <b>LevelToMatch</b> option is not set or if there is not match.
                        Otherwise, if there is a match, then the returned decision is
                        {@link spi::Filter#ACCEPT ACCEPT} if the
                        <b>AcceptOnMatch</b> property is set to <code>true</code>. The
                        returned decision is {@link spi::Filter#DENY DENY} if the
                        <b>AcceptOnMatch</b> property is set to false.
                        */
                        FilterDecision decide(const spi::LoggingEventPtr& event) const;
                }; // class LevelRangeFilter
            LOG4CXX_PTR_DEF(LevelRangeFilter);
        }  // namespace filter
} // namespace log4cxx

#endif // _LOG4CXX_FILTER_LEVEL_RANGE_FILTER_H
