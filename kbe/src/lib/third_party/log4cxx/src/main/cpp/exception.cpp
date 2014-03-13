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
#include <log4cxx/helpers/exception.h>
#include <string.h>
#include <string>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

Exception::Exception(const LogString& msg1) {
  std::string m;
  Transcoder::encode(msg1, m);
  size_t len = m.size();
  if (len > MSG_SIZE) {
      len = MSG_SIZE;
  }
#if defined(__STDC_LIB_EXT1__) || defined(__STDC_SECURE_LIB__)
  memcpy_s(msg, sizeof msg, m.data(), len);
#else
  memcpy(msg, m.data(), len);
#endif
  msg[len] = 0;
}

Exception::Exception(const char* m) {
#if defined(__STDC_LIB_EXT1__) || defined(__STDC_SECURE_LIB__)
  strncpy_s(msg, sizeof msg, m, MSG_SIZE);
#else
  strncpy(msg, m, MSG_SIZE);
#endif
  msg[MSG_SIZE] = 0;
}


Exception::Exception(const Exception& src) : std::exception() {
#if defined(__STDC_LIB_EXT1__) || defined(__STDC_SECURE_LIB__)
      strcpy_s(msg, sizeof msg, src.msg);
#else
      strcpy(msg, src.msg);
#endif
}

Exception& Exception::operator=(const Exception& src) {
#if defined(__STDC_LIB_EXT1__) || defined(__STDC_SECURE_LIB__)
      strcpy_s(msg, sizeof msg, src.msg);
#else
      strcpy(msg, src.msg);
#endif
  return *this;
}

const char* Exception::what() const throw() {
  return msg;
}

RuntimeException::RuntimeException(log4cxx_status_t stat)
     : Exception(formatMessage(stat)) {
}

RuntimeException::RuntimeException(const LogString& msg1)
     : Exception(msg1) {
}

RuntimeException::RuntimeException(const RuntimeException& src)
      : Exception(src) {
}

RuntimeException& RuntimeException::operator=(const RuntimeException& src) {
      Exception::operator=(src);
      return *this;
}

LogString RuntimeException::formatMessage(log4cxx_status_t stat) {
   LogString s(LOG4CXX_STR("RuntimeException: return code = "));
   Pool p;
   StringHelper::toString(stat, p, s);
   return s;
}

NullPointerException::NullPointerException(const LogString& msg1)
     : RuntimeException(msg1) {
}

NullPointerException::NullPointerException(const NullPointerException& src)
      : RuntimeException(src) {
}

NullPointerException& NullPointerException::operator=(const NullPointerException& src) {
      RuntimeException::operator=(src);
      return *this;
}

IllegalArgumentException::IllegalArgumentException(const LogString& msg1)
     : RuntimeException(msg1) {
}

IllegalArgumentException::IllegalArgumentException(const IllegalArgumentException& src)
      : RuntimeException(src) {
}

IllegalArgumentException& IllegalArgumentException::operator=(const IllegalArgumentException& src) {
      RuntimeException::operator=(src);
      return *this;
}

IOException::IOException()
     : Exception(LOG4CXX_STR("IO exception")) {
}

IOException::IOException(log4cxx_status_t stat)
    : Exception(formatMessage(stat)) {
}


IOException::IOException(const LogString& msg1)
     : Exception(msg1) {
}

IOException::IOException(const IOException& src)
      : Exception(src) {
}

IOException& IOException::operator=(const IOException& src) {
      Exception::operator=(src);
      return *this;
}

LogString IOException::formatMessage(log4cxx_status_t stat) {
   LogString s(LOG4CXX_STR("IO Exception : status code = "));
   Pool p;
   StringHelper::toString(stat, p, s);
   return s;
}


MissingResourceException::MissingResourceException(const LogString& key)
    : Exception(formatMessage(key)) {
}


MissingResourceException::MissingResourceException(const MissingResourceException& src)
      : Exception(src) {
}

MissingResourceException& MissingResourceException::operator=(const MissingResourceException& src) {
      Exception::operator=(src);
      return *this;
}

LogString MissingResourceException::formatMessage(const LogString& key) {
   LogString s(LOG4CXX_STR("MissingResourceException: resource key = \""));
   s.append(key);
   s.append(LOG4CXX_STR("\"."));
   return s;
}

PoolException::PoolException(log4cxx_status_t stat)
    : Exception(formatMessage(stat)) {
}

PoolException::PoolException(const PoolException &src)
   : Exception(src) {
}

PoolException& PoolException::operator=(const PoolException& src) {
     Exception::operator=(src);
     return *this;
}

LogString PoolException::formatMessage(log4cxx_status_t) {
     return LOG4CXX_STR("Pool exception");
}


TranscoderException::TranscoderException(log4cxx_status_t stat)
    : Exception(formatMessage(stat)) {
}

TranscoderException::TranscoderException(const TranscoderException &src)
   : Exception(src) {
}

TranscoderException& TranscoderException::operator=(const TranscoderException& src) {
     Exception::operator=(src);
     return *this;
}

LogString TranscoderException::formatMessage(log4cxx_status_t) {
     return LOG4CXX_STR("Transcoder exception");
}


MutexException::MutexException(log4cxx_status_t stat)
     : Exception(formatMessage(stat)) {
}

MutexException::MutexException(const MutexException &src)
     : Exception(src) {
}

MutexException& MutexException::operator=(const MutexException& src) {
      Exception::operator=(src);
      return *this;
}

LogString MutexException::formatMessage(log4cxx_status_t stat) {
      LogString s(LOG4CXX_STR("Mutex exception: stat = "));
      Pool p;
      StringHelper::toString(stat, p, s);
      return s;
}

InterruptedException::InterruptedException() : Exception(LOG4CXX_STR("Thread was interrupted")) {
}

InterruptedException::InterruptedException(log4cxx_status_t stat)
     : Exception(formatMessage(stat)) {
}

InterruptedException::InterruptedException(const InterruptedException &src)
     : Exception(src) {
}

InterruptedException& InterruptedException::operator=(const InterruptedException& src) {
      Exception::operator=(src);
      return *this;
}

LogString InterruptedException::formatMessage(log4cxx_status_t stat) {
      LogString s(LOG4CXX_STR("InterruptedException: stat = "));
      Pool p;
      StringHelper::toString(stat, p, s);
      return s;
}

ThreadException::ThreadException(log4cxx_status_t stat)
     : Exception(formatMessage(stat)) {
}

ThreadException::ThreadException(const LogString& msg)
     : Exception(msg) {
}

ThreadException::ThreadException(const ThreadException &src)
      : Exception(src) {
}

ThreadException& ThreadException::operator=(const ThreadException& src) {
       Exception::operator=(src);
       return *this;
}

LogString ThreadException::formatMessage(log4cxx_status_t stat) {
       LogString s(LOG4CXX_STR("Thread exception: stat = "));
       Pool p;
       StringHelper::toString(stat, p, s);
       return s;
}

IllegalMonitorStateException::IllegalMonitorStateException(const LogString& msg1)
      : Exception(msg1) {
}

IllegalMonitorStateException::IllegalMonitorStateException(const IllegalMonitorStateException& src)
      : Exception(src) {
}

IllegalMonitorStateException& IllegalMonitorStateException::operator=(const IllegalMonitorStateException& src) {
       Exception::operator=(src);
       return *this;
}

InstantiationException::InstantiationException(const LogString& msg1)
      : Exception(msg1) {
}

InstantiationException::InstantiationException(const InstantiationException& src)
       : Exception(src) {
}

InstantiationException& InstantiationException::operator=(const InstantiationException& src) {
        Exception::operator=(src);
        return *this;
}

ClassNotFoundException::ClassNotFoundException(const LogString& className)
    : Exception(formatMessage(className)) {
}

ClassNotFoundException::ClassNotFoundException(const ClassNotFoundException& src)
     : Exception(src) {
}


ClassNotFoundException& ClassNotFoundException::operator=(const ClassNotFoundException& src) {
      Exception::operator=(src);
      return *this;
}

LogString ClassNotFoundException::formatMessage(const LogString& className) {
      LogString s(LOG4CXX_STR("Class not found: "));
      s.append(className);
      return s;
}


NoSuchElementException::NoSuchElementException()
     : Exception(LOG4CXX_STR("No such element")) {
}

NoSuchElementException::NoSuchElementException(const NoSuchElementException& src)
     : Exception(src) {
}

NoSuchElementException& NoSuchElementException::operator=(const NoSuchElementException& src) {
      Exception::operator=(src);
      return *this;
}


IllegalStateException::IllegalStateException()
     : Exception(LOG4CXX_STR("Illegal state")) {
}

IllegalStateException::IllegalStateException(const IllegalStateException& src)
     : Exception(src) {
}

IllegalStateException& IllegalStateException::operator=(const IllegalStateException& src) {
      Exception::operator=(src);
      return *this;
}

SocketException::SocketException(const LogString& msg) : IOException(msg) {
}

SocketException::SocketException(log4cxx_status_t status) : IOException(status) { 
}

SocketException::SocketException(const SocketException& src)
     : IOException(src) {
}

SocketException& SocketException::operator=(const SocketException& src) {
      IOException::operator=(src);
      return *this;
}

ConnectException::ConnectException(log4cxx_status_t status) : SocketException(status) { 
}

ConnectException::ConnectException(const ConnectException& src)
     : SocketException(src) {
}

ConnectException& ConnectException::operator=(const ConnectException& src) {
      SocketException::operator=(src);
      return *this;
}

ClosedChannelException::ClosedChannelException() : SocketException(LOG4CXX_STR("Attempt to write to closed socket")) { 
}

ClosedChannelException::ClosedChannelException(const ClosedChannelException& src)
     : SocketException(src) {
}

ClosedChannelException& ClosedChannelException::operator=(const ClosedChannelException& src) {
      SocketException::operator=(src);
      return *this;
}

BindException::BindException(log4cxx_status_t status) : SocketException(status) { 
}

BindException::BindException(const BindException& src)
     : SocketException(src) {
}

BindException& BindException::operator=(const BindException& src) {
      SocketException::operator=(src);
      return *this;
}

InterruptedIOException::InterruptedIOException(const LogString& msg) : IOException(msg) { 
}

InterruptedIOException::InterruptedIOException(const InterruptedIOException& src)
     : IOException(src) {
}

InterruptedIOException& InterruptedIOException::operator=(const InterruptedIOException& src) {
      IOException::operator=(src);
      return *this;
}

SocketTimeoutException::SocketTimeoutException() 
    : InterruptedIOException(LOG4CXX_STR("SocketTimeoutException")) { 
}

SocketTimeoutException::SocketTimeoutException(const SocketTimeoutException& src)
     : InterruptedIOException(src) {
}

SocketTimeoutException& SocketTimeoutException::operator=(const SocketTimeoutException& src) {
      InterruptedIOException::operator=(src);
      return *this;
}
