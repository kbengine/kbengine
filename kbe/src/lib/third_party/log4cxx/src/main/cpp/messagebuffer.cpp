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

#include <log4cxx/helpers/messagebuffer.h>
#include <log4cxx/helpers/transcoder.h>

using namespace log4cxx::helpers;

CharMessageBuffer::CharMessageBuffer() : stream(0) {}

CharMessageBuffer::~CharMessageBuffer() {
   delete stream;
}

CharMessageBuffer& CharMessageBuffer::operator<<(const std::basic_string<char>& msg) {
   if (stream == 0) {
      buf.append(msg);
   } else {
      *stream << msg;
   }
   return *this;
}

CharMessageBuffer& CharMessageBuffer::operator<<(const char* msg) {
   const char* actualMsg = msg;
   if (actualMsg == 0) {
      actualMsg = "null";
   }
   if (stream == 0) {
      buf.append(actualMsg);
   } else {
      *stream << actualMsg;
   }
   return *this;
}
CharMessageBuffer& CharMessageBuffer::operator<<(char* msg) {
   return operator<<((const char*) msg);
}

CharMessageBuffer& CharMessageBuffer::operator<<(const char msg) {
   if (stream == 0) {
      buf.append(1, msg);
   } else {
      buf.assign(1, msg);
      *stream << buf;
   }
   return *this;
}

CharMessageBuffer::operator std::basic_ostream<char>&() {
   if (stream == 0) {
     stream = new std::basic_ostringstream<char>();
     if (!buf.empty()) {
        *stream << buf;
     }
   }
   return *stream;
}

const std::basic_string<char>& CharMessageBuffer::str(std::basic_ostream<char>&) {
   buf = stream->str();
   return buf;
}

const std::basic_string<char>& CharMessageBuffer::str(CharMessageBuffer&) {
   return buf;
}

bool CharMessageBuffer::hasStream() const {
    return (stream != 0);
}

std::ostream& CharMessageBuffer::operator<<(ios_base_manip manip) {
   std::ostream& s = *this;
   (*manip)(s);
   return s;
}

std::ostream& CharMessageBuffer::operator<<(bool val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(short val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(int val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(unsigned int val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(long val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(unsigned long val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(float val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(double val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(long double val) { return ((std::ostream&) *this).operator<<(val); }
std::ostream& CharMessageBuffer::operator<<(void* val) { return ((std::ostream&) *this).operator<<(val); }


#if LOG4CXX_WCHAR_T_API
WideMessageBuffer::WideMessageBuffer() : stream(0) {}

WideMessageBuffer::~WideMessageBuffer() {
   delete stream;
}

WideMessageBuffer& WideMessageBuffer::operator<<(const std::basic_string<wchar_t>& msg) {
   if (stream == 0) {
      buf.append(msg);
   } else {
      *stream << msg;
   }
   return *this;
}

WideMessageBuffer& WideMessageBuffer::operator<<(const wchar_t* msg) {
   const wchar_t* actualMsg = msg;
   if (actualMsg == 0) {
      actualMsg = L"null";
   }
   if (stream == 0) {
      buf.append(actualMsg);
   } else {
      *stream << actualMsg;
   }
   return *this;
}

WideMessageBuffer& WideMessageBuffer::operator<<(wchar_t* msg) {
   return operator<<((const wchar_t*) msg);
}

WideMessageBuffer& WideMessageBuffer::operator<<(const wchar_t msg) {
   if (stream == 0) {
      buf.append(1, msg);
   } else {
      buf.assign(1, msg);
      *stream << buf;
   }
   return *this;
}

WideMessageBuffer::operator std::basic_ostream<wchar_t>&() {
   if (stream == 0) {
     stream = new std::basic_ostringstream<wchar_t>();
     if (!buf.empty()) {
        *stream << buf;
     }
   }
   return *stream;
}

const std::basic_string<wchar_t>& WideMessageBuffer::str(std::basic_ostream<wchar_t>&) {
   buf = stream->str();
   return buf;
}

const std::basic_string<wchar_t>& WideMessageBuffer::str(WideMessageBuffer&) {
   return buf;
}

bool WideMessageBuffer::hasStream() const {
    return (stream != 0);
}

std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(ios_base_manip manip) {
   std::basic_ostream<wchar_t>& s = *this;
   (*manip)(s);
   return s;
}

std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(bool val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(short val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(int val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(unsigned int val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(long val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(unsigned long val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(float val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(double val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(long double val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }
std::basic_ostream<wchar_t>& WideMessageBuffer::operator<<(void* val) { return ((std::basic_ostream<wchar_t>&) *this).operator<<(val); }


MessageBuffer::MessageBuffer()  : wbuf(0)
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
   , ubuf(0)
#endif   
{
}

MessageBuffer::~MessageBuffer() {
    delete wbuf;
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
    delete ubuf;
#endif   
}

bool MessageBuffer::hasStream() const {
    bool retval = cbuf.hasStream() || (wbuf != 0 && wbuf->hasStream());
#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
    retval = retval || (ubuf != 0 && ubuf->hasStream());    
#endif   
    return retval;
}

std::ostream& MessageBuffer::operator<<(ios_base_manip manip) {
   std::ostream& s = *this;
   (*manip)(s);
   return s;
}

MessageBuffer::operator std::ostream&() {
   return (std::ostream&) cbuf;
}

CharMessageBuffer& MessageBuffer::operator<<(const std::string& msg) {
   return cbuf.operator<<(msg);
}

CharMessageBuffer& MessageBuffer::operator<<(const char* msg) {
   return cbuf.operator<<(msg);
}
CharMessageBuffer& MessageBuffer::operator<<(char* msg) {
   return cbuf.operator<<((const char*) msg);
}

CharMessageBuffer& MessageBuffer::operator<<(const char msg) {
   return cbuf.operator<<(msg);
}

const std::string& MessageBuffer::str(CharMessageBuffer& buf) {
   return cbuf.str(buf);
}

const std::string& MessageBuffer::str(std::ostream& os) {
   return cbuf.str(os);
}

WideMessageBuffer& MessageBuffer::operator<<(const std::wstring& msg) {
   wbuf = new WideMessageBuffer();
   return (*wbuf) << msg;
}

WideMessageBuffer& MessageBuffer::operator<<(const wchar_t* msg) {
   wbuf = new WideMessageBuffer();
   return (*wbuf) << msg;
}
WideMessageBuffer& MessageBuffer::operator<<(wchar_t* msg) {
   wbuf = new WideMessageBuffer();
   return (*wbuf) << (const wchar_t*) msg;
}

WideMessageBuffer& MessageBuffer::operator<<(const wchar_t msg) {
   wbuf = new WideMessageBuffer();
   return (*wbuf) << msg;
}

const std::wstring& MessageBuffer::str(WideMessageBuffer& buf) {
   return wbuf->str(buf);
}

const std::wstring& MessageBuffer::str(std::basic_ostream<wchar_t>& os) {
   return wbuf->str(os);
}

std::ostream& MessageBuffer::operator<<(bool val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(short val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(int val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(unsigned int val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(long val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(unsigned long val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(float val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(double val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(long double val) { return cbuf.operator<<(val); }
std::ostream& MessageBuffer::operator<<(void* val) { return cbuf.operator<<(val); }


#endif


#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
UniCharMessageBuffer& MessageBuffer::operator<<(const std::basic_string<log4cxx::UniChar>& msg) {
   ubuf = new UniCharMessageBuffer();
   return (*ubuf) << msg;
}

UniCharMessageBuffer& MessageBuffer::operator<<(const log4cxx::UniChar* msg) {
   ubuf = new UniCharMessageBuffer();
   return (*ubuf) << msg;
}
UniCharMessageBuffer& MessageBuffer::operator<<(log4cxx::UniChar* msg) {
   ubuf = new UniCharMessageBuffer();
   return (*ubuf) << (const log4cxx::UniChar*) msg;
}

UniCharMessageBuffer& MessageBuffer::operator<<(const log4cxx::UniChar msg) {
   ubuf = new UniCharMessageBuffer();
   return (*ubuf) << msg;
}

const std::basic_string<log4cxx::UniChar>& MessageBuffer::str(UniCharMessageBuffer& buf) {
   return ubuf->str(buf);
}

const std::basic_string<log4cxx::UniChar>& MessageBuffer::str(std::basic_ostream<log4cxx::UniChar>& os) {
   return ubuf->str(os);
}


UniCharMessageBuffer::UniCharMessageBuffer() : stream(0) {}

UniCharMessageBuffer::~UniCharMessageBuffer() {
   delete stream;
}


UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const std::basic_string<log4cxx::UniChar>& msg) {
   if (stream == 0) {
        buf.append(msg);
   } else {
      *stream << buf;
   }
   return *this;
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const log4cxx::UniChar* msg) {
   const log4cxx::UniChar* actualMsg = msg;
    static log4cxx::UniChar nullLiteral[] = { 0x6E, 0x75, 0x6C, 0x6C, 0};
   if (actualMsg == 0) {
      actualMsg = nullLiteral;
   }
   if (stream == 0) {
        buf.append(actualMsg);
   } else {
      *stream << actualMsg;
   }
   return *this;
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(log4cxx::UniChar* msg) {
   return operator<<((const log4cxx::UniChar*) msg);
}

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const log4cxx::UniChar msg) {
   if (stream == 0) {
        buf.append(1, msg);
   } else {
      *stream << msg;
   }
   return *this;
}

UniCharMessageBuffer::operator UniCharMessageBuffer::uostream&() {
   if (stream == 0) {
     stream = new std::basic_ostringstream<UniChar>();
     if (!buf.empty()) {
        *stream << buf;
     }
   }
   return *stream;
}

const std::basic_string<log4cxx::UniChar>& UniCharMessageBuffer::str(UniCharMessageBuffer::uostream&) {
    buf = stream->str();
   return buf;
}

const std::basic_string<log4cxx::UniChar>& UniCharMessageBuffer::str(UniCharMessageBuffer&) {
   return buf;
}

bool UniCharMessageBuffer::hasStream() const {
    return (stream != 0);
}

UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(ios_base_manip manip) {
   UniCharMessageBuffer::uostream& s = *this;
   (*manip)(s);
   return s;
}

UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(bool val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(short val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(int val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(unsigned int val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(long val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(unsigned long val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(float val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(double val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(long double val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }
UniCharMessageBuffer::uostream& UniCharMessageBuffer::operator<<(void* val) { return ((UniCharMessageBuffer::uostream&) *this).operator<<(val); }



#endif

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#include <vector>

UniCharMessageBuffer& UniCharMessageBuffer::operator<<(const CFStringRef& msg) {
    const log4cxx::UniChar* chars = CFStringGetCharactersPtr(msg);
    if (chars != 0) {
         return operator<<(chars);
    } else {
         size_t length = CFStringGetLength(msg);
         std::vector<log4cxx::UniChar> tmp(length);
         CFStringGetCharacters(msg, CFRangeMake(0, length), &tmp[0]);
         if (stream) {
            std::basic_string<UniChar> s(&tmp[0], tmp.size());
            *stream << s;
         } else {
            buf.append(&tmp[0], tmp.size());
        }
    }
   return *this;
}


UniCharMessageBuffer& MessageBuffer::operator<<(const CFStringRef& msg) {
   ubuf = new UniCharMessageBuffer();
   return (*ubuf) << msg;
}
#endif

