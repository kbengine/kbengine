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
#include <log4cxx/helpers/strftimedateformat.h>

#include <apr_time.h>
#include <log4cxx/helpers/transcoder.h>

using namespace log4cxx;
using namespace log4cxx::helpers;


StrftimeDateFormat::StrftimeDateFormat(const LogString& fmt)
  : timeZone(TimeZone::getDefault()) {
    log4cxx::helpers::Transcoder::encode(fmt, pattern);
}

StrftimeDateFormat::~StrftimeDateFormat() {
}


void StrftimeDateFormat::format(LogString& s, log4cxx_time_t time, Pool& /* p */ ) const  {
  apr_time_exp_t exploded;
  apr_status_t stat = timeZone->explode(&exploded, time);
  if (stat == APR_SUCCESS) {
    const apr_size_t bufSize = 255;
    char buf[bufSize];
    apr_size_t bufLen;
    stat = apr_strftime(buf, &bufLen, bufSize, pattern.c_str(), &exploded);
    if (stat == APR_SUCCESS) {
      log4cxx::helpers::Transcoder::decode(std::string(buf, bufLen), s);
    }
  }
}

void StrftimeDateFormat::setTimeZone(const TimeZonePtr& zone) {
  timeZone = zone;
}


