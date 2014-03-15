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
#include <log4cxx/stream.h>
#include <log4cxx/helpers/transcoder.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>

using namespace log4cxx;

logstream_base::logstream_ios_base::logstream_ios_base(std::ios_base::fmtflags initval, 
                    int initsize) {
#if LOG4CXX_MEMSET_IOS_BASE
   //
   //    the destructor for std::ios_base in the MSVC STL
   //        releases a pointer that was not initialized in the constructor.
   //
    memset(this, 0, sizeof(*this));
#endif
    flags(initval);
    precision(initsize);
    width(initsize);
                    
}


logstream_base::logstream_base(const LoggerPtr& log,
     const LevelPtr& lvl) : initset((std::ios_base::fmtflags) -1, 1), 
     initclear((std::ios_base::fmtflags) 0, 0), fillchar(0), fillset(false), logger(log), level(lvl), location() {
     enabled = logger->isEnabledFor(level);
}

logstream_base::~logstream_base() {
}

void logstream_base::insert(std::ios_base& (*manip)(std::ios_base&)) {
    get_stream_state(initclear, initset, fillchar, fillset);
    (*manip)(initset);
    (*manip)(initclear);
    refresh_stream_state();
}

bool logstream_base::set_stream_state(std::ios_base& dest, int& dstchar) {
     std::ios_base::fmtflags setval = initset.flags();
     std::ios_base::fmtflags clrval = initclear.flags();
     std::ios_base::fmtflags mask = setval ^ (~clrval);
     dest.setf(clrval, mask);
     if (initset.precision() == initclear.precision()) {
         dest.precision(initset.precision());
     }
     if (initset.width() == initclear.width()) {
         dest.width(initset.width());
     }
     dstchar = fillchar;
     return fillset;
}

logstream_base& logstream_base::endmsg(logstream_base& stream) {
     stream.end_message();
     return stream;
}

logstream_base& logstream_base::nop(logstream_base& stream) {
     return stream;
}

void logstream_base::end_message() {
     if (isEnabled()) {
         log(logger, level, location);
     }
     erase();
}



int log4cxx::logstream_base::precision(int p) {
    get_stream_state(initclear, initset, fillchar, fillset);
    initset.precision(p);
    int oldVal = initclear.precision(p);
    refresh_stream_state();
    return oldVal;
}

int log4cxx::logstream_base::precision() {
    get_stream_state(initclear, initset, fillchar, fillset);
   return initclear.precision();
}

int log4cxx::logstream_base::width(int w) {
    get_stream_state(initclear, initset, fillchar, fillset);
    initset.width(w);
    int oldVal = initclear.width(w);
    refresh_stream_state();
    return oldVal;
}

int log4cxx::logstream_base::width()  {
    get_stream_state(initclear, initset, fillchar, fillset);
    return initclear.width();
}

int log4cxx::logstream_base::fill(int newfill) {
    get_stream_state(initclear, initset, fillchar, fillset);
    int oldfill = fillchar;
    fillchar = newfill;
    fillset = true;
    refresh_stream_state();
    return oldfill;
}

int logstream_base::fill()  {
    get_stream_state(initclear, initset, fillchar, fillset);
    return fillchar;
}

std::ios_base::fmtflags logstream_base::flags(std::ios_base::fmtflags newflags) {
    get_stream_state(initclear, initset, fillchar, fillset);
    initset.flags(newflags);
    std::ios_base::fmtflags oldVal = initclear.flags(newflags);
    refresh_stream_state();
    return oldVal;
}

std::ios_base::fmtflags logstream_base::setf(std::ios_base::fmtflags newflags, std::ios_base::fmtflags mask) {
    get_stream_state(initclear, initset, fillchar, fillset);
    initset.setf(newflags, mask);
    std::ios_base::fmtflags oldVal = initclear.setf(newflags, mask);
    refresh_stream_state();
    return oldVal;
}

std::ios_base::fmtflags logstream_base::setf(std::ios_base::fmtflags newflags) {
    get_stream_state(initclear, initset, fillchar, fillset);
    initset.setf(newflags);
    std::ios_base::fmtflags oldVal = initclear.setf(newflags);
    refresh_stream_state();
    return oldVal;
}
    


void logstream_base::setLevel(const ::log4cxx::LevelPtr& newlevel) {
    level = newlevel;
    bool oldLevel = enabled;
    enabled = logger->isEnabledFor(level);
    if (oldLevel != enabled) {
        erase();
    }
}

bool logstream_base::isEnabledFor(const ::log4cxx::LevelPtr& level) const {
    return logger->isEnabledFor(level);
}


void logstream_base::setLocation(const log4cxx::spi::LocationInfo& newlocation) {
    if (LOG4CXX_UNLIKELY(enabled)) {
        location = newlocation;
    }
}


logstream::logstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level) : logstream_base(logger, level), stream(0) {
}
             
logstream::logstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level) 
            : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}


logstream::logstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level) : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}
             
logstream::~logstream() {
    delete stream;
}

logstream& logstream::operator<<(logstream_base& (*manip)(logstream_base&)) {
    (*manip)(*this);
    return *this;
}

logstream& logstream::operator<<(const LevelPtr& level) {
    setLevel(level);
    return *this;
}

logstream& logstream::operator<<(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}

logstream& logstream::operator>>(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}
             
logstream& logstream::operator<<(std::ios_base& (*manip)(std::ios_base&)) {
      logstream_base::insert(manip);
      return *this;
}
            
logstream::operator std::basic_ostream<char>&() {
      if (stream == 0) {
          stream = new std::basic_stringstream<Ch>();
          refresh_stream_state();
      }
      return *stream;
}

void logstream::log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location) {
    if (stream != 0) {
        std::basic_string<Ch> msg = stream->str();
        if (!msg.empty()) {
            logger->log(level, msg, location);
        }
    }
}
              

void logstream::erase() {
  if (stream != 0) {
      std::basic_string<Ch> emptyStr;
      stream->str(emptyStr);
  }
}
              

void logstream::get_stream_state(std::ios_base& base,
                            std::ios_base& mask,
                            int& fill,
                            bool& fillSet) const {
  if (stream != 0) {
      std::ios_base::fmtflags flags = stream->flags();
      base.flags(flags);
      mask.flags(flags);
      int width = stream->width();
      base.width(width);
      mask.width(width);
      int precision = stream->precision();
      base.precision(precision);
      mask.precision(precision);
      fill = stream->fill();
      fillSet = true;
  }
}

void logstream::refresh_stream_state() {
   if (stream != 0) {
      int fillchar;
      if(logstream_base::set_stream_state(*stream, fillchar)) {
         stream->fill(fillchar);
      }
   }
}
              

#if LOG4CXX_WCHAR_T_API

wlogstream::wlogstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level) : logstream_base(logger, level), stream(0) {
}
             
wlogstream::wlogstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level) 
            : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}


wlogstream::wlogstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level) : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}
             
wlogstream::~wlogstream() {
    delete stream;
}

wlogstream& wlogstream::operator<<(logstream_base& (*manip)(logstream_base&)) {
    (*manip)(*this);
    return *this;
}

wlogstream& wlogstream::operator<<(const LevelPtr& level) {
    setLevel(level);
    return *this;
}

wlogstream& wlogstream::operator<<(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}

wlogstream& wlogstream::operator>>(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}



             
wlogstream& wlogstream::operator<<(std::ios_base& (*manip)(std::ios_base&)) {
      logstream_base::insert(manip);
      return *this;
}
            
wlogstream::operator std::basic_ostream<wchar_t>&() {
      if (stream == 0) {
          stream = new std::basic_stringstream<Ch>();
          refresh_stream_state();
      }
      return *stream;
}

void wlogstream::log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location) {
    if (stream != 0) {
        std::basic_string<Ch> msg = stream->str();
        if (!msg.empty()) {
            logger->log(level, msg, location);
        }
    }
}
              

void wlogstream::erase() {
  if (stream != 0) {
      std::basic_string<Ch> emptyStr;
      stream->str(emptyStr);
  }
}
              

void wlogstream::get_stream_state(std::ios_base& base,
                            std::ios_base& mask,
                            int& fill,
                            bool& fillSet) const {
  if (stream != 0) {
      std::ios_base::fmtflags flags = stream->flags();
      base.flags(flags);
      mask.flags(flags);
      int width = stream->width();
      base.width(width);
      mask.width(width);
      int precision = stream->precision();
      base.precision(precision);
      mask.precision(precision);
      fill = stream->fill();
      fillSet = true;
  }
}

void wlogstream::refresh_stream_state() {
   if (stream != 0) {
      int fillchar;
      if(logstream_base::set_stream_state(*stream, fillchar)) {
         stream->fill(fillchar);
      }
   }
}
#endif

#if LOG4CXX_UNICHAR_API
ulogstream::ulogstream(const Ch* loggerName, 
                const log4cxx::LevelPtr& level) 
            : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}


ulogstream::ulogstream(const std::basic_string<Ch>& loggerName, 
                const log4cxx::LevelPtr& level) : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}
#endif

#if LOG4CXX_CFSTRING_API
ulogstream::ulogstream(const CFStringRef& loggerName, 
                const log4cxx::LevelPtr& level) 
            : logstream_base(log4cxx::Logger::getLogger(loggerName), level), stream(0) {
}

#endif
              

#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API

ulogstream::ulogstream(const log4cxx::LoggerPtr& logger,
                 const log4cxx::LevelPtr& level) : logstream_base(logger, level), stream(0) {
}
             
             
             
ulogstream::~ulogstream() {
    delete stream;
}

ulogstream& ulogstream::operator<<(logstream_base& (*manip)(logstream_base&)) {
    (*manip)(*this);
    return *this;
}

ulogstream& ulogstream::operator<<(const LevelPtr& level) {
    setLevel(level);
    return *this;
}

ulogstream& ulogstream::operator<<(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}

ulogstream& ulogstream::operator>>(const log4cxx::spi::LocationInfo& newlocation) {
   setLocation(newlocation);
   return *this;
}



             
ulogstream& ulogstream::operator<<(std::ios_base& (*manip)(std::ios_base&)) {
      logstream_base::insert(manip);
      return *this;
}
            
ulogstream::operator std::basic_ostream<UniChar>&() {
      if (stream == 0) {
          stream = new std::basic_stringstream<Ch>();
          refresh_stream_state();
      }
      return *stream;
}

void ulogstream::log(LoggerPtr& logger,
                               const LevelPtr& level,
                               const log4cxx::spi::LocationInfo& location) {
    if (stream != 0) {
        std::basic_string<Ch> msg = stream->str();
        if (!msg.empty() && logger->isEnabledFor(level)) {
            LOG4CXX_DECODE_UNICHAR(lsmsg, msg);
            logger->forcedLogLS(level, lsmsg, location);
        }
    }
}
              

void ulogstream::erase() {
  if (stream != 0) {
      std::basic_string<Ch> emptyStr;
      stream->str(emptyStr);
  }
}
              

void ulogstream::get_stream_state(std::ios_base& base,
                            std::ios_base& mask,
                            int& fill,
                            bool& fillSet) const {
  if (stream != 0) {
      std::ios_base::fmtflags flags = stream->flags();
      base.flags(flags);
      mask.flags(flags);
      int width = stream->width();
      base.width(width);
      mask.width(width);
      int precision = stream->precision();
      base.precision(precision);
      mask.precision(precision);
      fill = stream->fill();
      fillSet = true;
  }
}

void ulogstream::refresh_stream_state() {
   if (stream != 0) {
      int fillchar;
      if(logstream_base::set_stream_state(*stream, fillchar)) {
         stream->fill(fillchar);
      }
   }
}
#endif
              

