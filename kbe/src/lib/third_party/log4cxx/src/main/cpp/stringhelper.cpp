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

#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <algorithm>
#include <vector>
#include <apr_strings.h>
#include <log4cxx/helpers/pool.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#include <cctype>
#include <apr.h>


using namespace log4cxx;
using namespace log4cxx::helpers;

bool StringHelper::equalsIgnoreCase(const LogString& s1, const logchar* upper, const logchar* lower) {
  for (LogString::const_iterator iter = s1.begin();
       iter != s1.end();
       iter++, upper++, lower++) {
       if (*iter != *upper && *iter != * lower) return false;
  }
  return (*upper == 0);
}

bool StringHelper::equalsIgnoreCase(const LogString& s1, const LogString& upper, const LogString& lower) {
  LogString::const_iterator u = upper.begin();
  LogString::const_iterator l = lower.begin();
  LogString::const_iterator iter = s1.begin();
  for (;
       iter != s1.end() && u != upper.end() && l != lower.end();
       iter++, u++, l++) {
       if (*iter != *u && *iter != *l) return false;
  }
  return u == upper.end() && iter == s1.end();
}



LogString StringHelper::toLowerCase(const LogString& s)
{
        LogString d;
        std::transform(s.begin(), s.end(),
                std::insert_iterator<LogString>(d, d.begin()), tolower);
        return d;
}

LogString StringHelper::trim(const LogString& s)
{
        LogString::size_type pos = s.find_first_not_of(' ');
        if (pos == std::string::npos)
        {
                return LogString();
        }

        LogString::size_type n = s.find_last_not_of(' ') - pos + 1;
        return s.substr(pos, n);
}

bool StringHelper::startsWith(const LogString& s, const LogString& prefix)
{
    return s.compare(0, prefix.length(), prefix) == 0;
}

bool StringHelper::endsWith(const LogString& s, const LogString& suffix)
{
    if (suffix.length() <= s.length()) {
      return s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    return false;
}


int StringHelper::toInt(const LogString& s) {
    std::string as;
    Transcoder::encode(s, as);
    return atoi(as.c_str());
}

log4cxx_int64_t StringHelper::toInt64(const LogString& s) {
    std::string as;
    Transcoder::encode(s, as);
    return apr_atoi64(as.c_str());
}

void StringHelper::toString(int n, Pool& pool, LogString& s) {
  char* fmt = pool.itoa(n);
  Transcoder::decode(fmt, s);
}

void StringHelper::toString(bool val, LogString& dst) {
  if (val) {
    dst.append(LOG4CXX_STR("true"));
  } else {
    dst.append(LOG4CXX_STR("false"));
  }
}


void StringHelper::toString(log4cxx_int64_t n, Pool& pool, LogString& dst) {
  if (n >= INT_MIN && n <= INT_MAX) {
    toString((int) n, pool, dst);
  } else {
    const log4cxx_int64_t BILLION = APR_INT64_C(1000000000);
    int billions = (int) (n / BILLION);
    char* upper = pool.itoa(billions);
    int remain = (int) (n - billions * BILLION);
    if (remain < 0) remain *= -1;
    char* lower = pool.itoa(remain);
    Transcoder::decode(upper, dst);
    dst.append(9 - strlen(lower), 0x30 /* '0' */);
    Transcoder::decode(lower, dst);
  }
}


void StringHelper::toString(size_t n, Pool& pool, LogString& s) {
  toString((log4cxx_int64_t) n, pool, s);
}

LogString StringHelper::format(const LogString& pattern, const std::vector<LogString>& params) {

  LogString result;
  int i = 0;
  while(pattern[i] != 0) {
    if (pattern[i] == 0x7B /* '{' */ && pattern[i + 1] >= 0x30 /* '0' */ &&
        pattern[i + 1] <= 0x39 /* '9' */ && pattern[i + 2] == 0x7D /* '}' */) {
      int arg = pattern[i + 1] - 0x30 /* '0' */;
      result = result + params[arg];
      i += 3;
    } else {
      result = result + pattern[i];
      i++;
    }
  }

  return result;
}

