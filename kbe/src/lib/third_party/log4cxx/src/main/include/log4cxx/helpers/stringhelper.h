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

#ifndef _LOG4CXX_HELPERS_STRING_HELPER_H
#define _LOG4CXX_HELPERS_STRING_HELPER_H

#include <log4cxx/logstring.h>
#include <vector>


namespace log4cxx
{
    namespace helpers
    {
        class Pool;
                /**
                String manipulation routines
                */
        class LOG4CXX_EXPORT StringHelper
        {
           public:
            static LogString trim(const LogString& s);
            static bool startsWith(const LogString& s, const LogString& suffix);
            static bool endsWith(const LogString& s, const LogString& suffix);
            static bool equalsIgnoreCase(const LogString& s1,
                 const logchar* upper, const logchar* lower);
            static bool equalsIgnoreCase(const LogString& s1,
                 const LogString& upper, const LogString& lower);


            static int toInt(const LogString& s);
            static log4cxx_int64_t toInt64(const LogString& s);

            static void toString(int i, log4cxx::helpers::Pool& pool, LogString& dst);
            static void toString(log4cxx_int64_t i, log4cxx::helpers::Pool& pool, LogString& dst);
            static void toString(size_t i, log4cxx::helpers::Pool& pool, LogString& dst);

            static void toString(bool val, LogString& dst);

            static LogString toLowerCase(const LogString& s);

            static LogString format(const LogString& pattern, const std::vector<LogString>& params);
        };
    }
}

#endif //_LOG4CXX_HELPERS_STRING_HELPER_H
