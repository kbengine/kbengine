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

#ifndef _LOG4CXX_TESTS_UTIL_FILTER_H
#define _LOG4CXX_TESTS_UTIL_FILTER_H

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <string>
#include <vector>
#include <map>
#include <log4cxx/helpers/exception.h>

#define BASIC_PAT "\\[0x[0-9A-F]*] (FATAL|ERROR|WARN|INFO|DEBUG)"
#define ISO8601_PAT "[0-9]\\{4\\}-[0-9]\\{2\\}-[0-9]\\{2\\} [0-9]\\{2\\}:[0-9]\\{2\\}:[0-9]\\{2\\},[0-9]\\{3\\}"
#define ABSOLUTE_DATE_AND_TIME_PAT \
        "[0-9]\\{1,2\\} .* 200[0-9] [0-9]\\{2\\}:[0-9]\\{2\\}:[0-9]\\{2\\},[0-9]\\{3\\}"
#define ABSOLUTE_TIME_PAT "[0-2][0-9]:[0-9][0-9]:[0-9][0-9],[0-9][0-9][0-9]"
#define RELATIVE_TIME_PAT "^[0-9]+"

namespace log4cxx
{
        class UnexpectedFormatException : public std::exception {
        };

        class Filter
        {
        public:
            Filter(const std::string& match, const std::string& replacement);
            Filter();
            virtual ~Filter();

            typedef std::pair<std::string, std::string> PatternReplacement;
            typedef std::vector <PatternReplacement> PatternList;
            const PatternList& getPatterns()  const{
                return patterns;
            }

        private:
            Filter(const Filter&);
            Filter& operator=(const Filter&);
        protected:
            PatternList patterns;
        };
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif //_LOG4CXX_TESTS_UTIL_FILTER_H
