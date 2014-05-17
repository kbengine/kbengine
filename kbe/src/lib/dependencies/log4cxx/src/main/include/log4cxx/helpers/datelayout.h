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

#ifndef _LOG4CXX_HELPERS_DATE_LAYOUT_H
#define _LOG4CXX_HELPERS_DATE_LAYOUT_H

#include <log4cxx/layout.h>
#include <log4cxx/helpers/dateformat.h>
#include <log4cxx/helpers/timezone.h>

namespace log4cxx
{
        namespace helpers
        {
                /**
                This abstract layout takes care of all the date related options and
                formatting work.
                */
                class LOG4CXX_EXPORT DateLayout : public Layout
                {
                private:
                        LogString timeZoneID;
                        LogString dateFormatOption;

                protected:
                        DateFormatPtr dateFormat;

                public:
                        DateLayout(const LogString& dateLayoutOption);
                        virtual ~DateLayout();

                        virtual void activateOptions(log4cxx::helpers::Pool& p);
                        virtual void setOption(const LogString& option, const LogString& value);

                        /**
                        The value of the <b>DateFormat</b> option should be either an
                        argument to the constructor of helpers::DateFormat or one of
                        the strings <b>"NULL"</b>, <b>"RELATIVE"</b>, <b>"ABSOLUTE"</b>,
                        <b>"DATE"</b> or <b>"ISO8601</b>.
                        */
                        inline void setDateFormat(const LogString& dateFormat1)
                          { this->dateFormatOption.assign(dateFormat1); }

                        /**
                        Returns value of the <b>DateFormat</b> option.
                        */
                        inline const LogString& getDateFormat() const
                                { return dateFormatOption; }

                        /**
                        The <b>TimeZoneID</b> option is a time zone ID string in the format
                        expected by the <code>locale</code> C++ standard class.
                        */
                        inline void setTimeZone(const LogString& timeZone)
                                { this->timeZoneID.assign(timeZone); }

                        /**
                        Returns value of the <b>TimeZone</b> option.
                        */
                        inline const LogString& getTimeZone() const
                                { return timeZoneID; }

                        void formatDate(LogString &s,
                                        const spi::LoggingEventPtr& event,
                                        log4cxx::helpers::Pool& p) const;

                private:
                       //
                       //  prevent copy and assignment
                       DateLayout(const DateLayout&);
                       DateLayout& operator=(const DateLayout&);

                };
        }  // namespace helpers
} // namespace log4cxx

#endif // _LOG4CXX_HELPERS_DATE_LAYOUT_H
