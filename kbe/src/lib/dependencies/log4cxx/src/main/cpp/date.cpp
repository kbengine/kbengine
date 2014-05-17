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
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/date.h>


#ifndef INT64_C
#define INT64_C(x) x ## LL
#endif


#include <apr_time.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(Date)


Date::Date() : time(apr_time_now()) {
}

Date::Date(log4cxx_time_t t) : time(t) {
}

Date::~Date() {
}

log4cxx_time_t Date::getMicrosecondsPerDay() {
   return APR_INT64_C(86400000000);
}

log4cxx_time_t Date::getMicrosecondsPerSecond() {
   return APR_USEC_PER_SEC;
}


log4cxx_time_t Date::getNextSecond() const {
    return ((time / APR_USEC_PER_SEC) + 1) * APR_USEC_PER_SEC;
}
