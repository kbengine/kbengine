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

#ifndef _LOG4CXX_HELPERS_RELATIVE_TIME_DATE_FORMAT_H
#define _LOG4CXX_HELPERS_RELATIVE_TIME_DATE_FORMAT_H

#include <log4cxx/helpers/dateformat.h>

namespace log4cxx
{
        namespace helpers
        {
                /**
                Formats a date by printing the number of seconds
                elapsed since the start of the application. This is the fastest
                printing DateFormat in the package.
                */
                class LOG4CXX_EXPORT RelativeTimeDateFormat : public DateFormat
                {
                public:
                        RelativeTimeDateFormat();
                        virtual void format(LogString &s,
                                        log4cxx_time_t tm,
                                        log4cxx::helpers::Pool& p) const;

                private:
                        log4cxx_time_t startTime;

                };
        }  // namespace helpers
} // namespace log4cxx

#endif // _LOG4CXX_HELPERS_RELATIVE_TIME_DATE_FORMAT_H
