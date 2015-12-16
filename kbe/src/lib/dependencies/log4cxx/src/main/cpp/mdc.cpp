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

#include <log4cxx/mdc.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/threadspecificdata.h>

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#endif


using namespace log4cxx;
using namespace log4cxx::helpers;

MDC::MDC(const std::string& key1, const std::string& value) : key()
{
        Transcoder::decode(key1, key);
        LOG4CXX_DECODE_CHAR(v, value);
        putLS(key, v);
}

MDC::~MDC()
{
        LogString prevVal;
        remove(key, prevVal);
}

void MDC::putLS(const LogString& key, const LogString& value)
{
        ThreadSpecificData::put(key, value);
}

void MDC::put(const std::string& key, const std::string& value)
{
        LOG4CXX_DECODE_CHAR(lkey, key);
        LOG4CXX_DECODE_CHAR(lvalue, value);
        putLS(lkey, lvalue);
}

bool MDC::get(const LogString& key, LogString& value)
{
        ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
        if (data != 0) {
            Map& map = data->getMap();

            Map::iterator it = map.find(key);
            if (it != map.end()) {
                value.append(it->second);
                return true;
            }
            data->recycle();
        }
        return false;
}

std::string MDC::get(const std::string& key)
{
        LOG4CXX_DECODE_CHAR(lkey, key);
        LogString lvalue;
        if (get(lkey, lvalue)) {
          LOG4CXX_ENCODE_CHAR(value, lvalue);
          return value;
        }
        return std::string();
}

bool MDC::remove(const LogString& key, LogString& value)
{
        ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
        if (data != 0) {
            Map& map = data->getMap();
            Map::iterator it;
            if ((it = map.find(key)) != map.end()) {
                value = it->second;
                map.erase(it);
                data->recycle();
                return true;
            }
        }
        return false;
}

std::string MDC::remove(const std::string& key)
{
        LOG4CXX_DECODE_CHAR(lkey, key);
        LogString lvalue;
        if (remove(lkey, lvalue)) {
          LOG4CXX_ENCODE_CHAR(value, lvalue);
          return value;
        }
        return std::string();
}


void MDC::clear()
{
        ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
        if (data != 0) {
            Map& map = data->getMap();
            map.erase(map.begin(), map.end());
            data->recycle();
        }
}


#if LOG4CXX_WCHAR_T_API
MDC::MDC(const std::wstring& key1, const std::wstring& value) : key()
{
        Transcoder::decode(key1, key);
        LOG4CXX_DECODE_WCHAR(v, value);
        putLS(key, v);
}

std::wstring MDC::get(const std::wstring& key)
{
        LOG4CXX_DECODE_WCHAR(lkey, key);
        LogString lvalue;
        if (get(lkey, lvalue)) {
          LOG4CXX_ENCODE_WCHAR(value, lvalue);
          return value;
        }
        return std::wstring();
}

void MDC::put(const std::wstring& key, const std::wstring& value)
{
        LOG4CXX_DECODE_WCHAR(lkey, key);
        LOG4CXX_DECODE_WCHAR(lvalue, value);
        putLS(lkey, lvalue);
}


std::wstring MDC::remove(const std::wstring& key)
{
        LOG4CXX_DECODE_WCHAR(lkey, key);
        LogString lvalue;
        if (remove(lkey, lvalue)) {
          LOG4CXX_ENCODE_WCHAR(value, lvalue);
          return value;
        }
        return std::wstring();
}
#endif

#if LOG4CXX_UNICHAR_API
MDC::MDC(const std::basic_string<UniChar>& key1, const std::basic_string<UniChar>& value) {
        Transcoder::decode(key1, key);
        LOG4CXX_DECODE_UNICHAR(v, value);
        putLS(key, v);
}

std::basic_string<log4cxx::UniChar> MDC::get(const std::basic_string<log4cxx::UniChar>& key)
{
        LOG4CXX_DECODE_UNICHAR(lkey, key);
        LogString lvalue;
        if (get(lkey, lvalue)) {
          LOG4CXX_ENCODE_UNICHAR(value, lvalue);
          return value;
        }
        return std::basic_string<UniChar>();
}

void MDC::put(const std::basic_string<UniChar>& key, const std::basic_string<log4cxx::UniChar>& value)
{
        LOG4CXX_DECODE_UNICHAR(lkey, key);
        LOG4CXX_DECODE_UNICHAR(lvalue, value);
        putLS(lkey, lvalue);
}


std::basic_string<log4cxx::UniChar> MDC::remove(const std::basic_string<log4cxx::UniChar>& key)
{
        LOG4CXX_DECODE_UNICHAR(lkey, key);
        LogString lvalue;
        if (remove(lkey, lvalue)) {
          LOG4CXX_ENCODE_UNICHAR(value, lvalue);
          return value;
        }
        return std::basic_string<UniChar>();
}
#endif

#if LOG4CXX_CFSTRING_API

MDC::MDC(const CFStringRef& key1, const CFStringRef& value) {
        Transcoder::decode(key1, key);
        LOG4CXX_DECODE_CFSTRING(v, value);
        putLS(key, v);
}

CFStringRef MDC::get(const CFStringRef& key)
{
        LOG4CXX_DECODE_CFSTRING(lkey, key);
        LogString lvalue;
        if (get(lkey, lvalue)) {
          LOG4CXX_ENCODE_CFSTRING(value, lvalue);
          return value;
        }
        return CFSTR("");
}

void MDC::put(const CFStringRef& key, const CFStringRef& value)
{
        LOG4CXX_DECODE_CFSTRING(lkey, key);
        LOG4CXX_DECODE_CFSTRING(lvalue, value);
        putLS(lkey, lvalue);
}


CFStringRef MDC::remove(const CFStringRef& key)
{
        LOG4CXX_DECODE_CFSTRING(lkey, key);
        LogString lvalue;
        if (remove(lkey, lvalue)) {
          LOG4CXX_ENCODE_CFSTRING(value, lvalue);
          return value;
        }
        return CFSTR("");
}
#endif

