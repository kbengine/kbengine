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

#ifndef _LOG4CXX_HELPERS_TRANSCODER_H
#define _LOG4CXX_HELPERS_TRANSCODER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/logstring.h>


namespace log4cxx {
   namespace helpers {
     class ByteBuffer;
     class Pool;
     /**
     *    Simple transcoder for converting between
     *      external char and wchar_t strings and
     *      internal strings.
     *
     */
      class LOG4CXX_EXPORT Transcoder {
      public:

      
      /**
       *   Appends this specified string of UTF-8 characters to LogString.
       */
      static void decodeUTF8(const std::string& src, LogString& dst);
      /**
       *    Converts the LogString to a UTF-8 string.
       */
      static void encodeUTF8(const LogString& src, std::string& dst);
      /**
       *    Converts the LogString to a UTF-8 string.
       */
      static char* encodeUTF8(const LogString& src, log4cxx::helpers::Pool& p);
      /**
       *    Append UCS-4 code point to a byte buffer as UTF-8.
       */
      static void encodeUTF8(unsigned int sv, ByteBuffer& dst);
      /**
       *    Append UCS-4 code point to a byte buffer as UTF-16LE.
       */
      static void encodeUTF16LE(unsigned int sv, ByteBuffer& dst);
      /**
       *    Append UCS-4 code point to a byte buffer as UTF-16BE.
       */
      static void encodeUTF16BE(unsigned int sv, ByteBuffer& dst);
      

      /**
       *   Decodes next character from a UTF-8 string.
       *   @param in string from which the character is extracted.
       *   @param iter iterator addressing start of character, will be
       *   advanced to next character if successful.
       *   @return scalar value (UCS-4) or 0xFFFF if invalid sequence.
       */
      static unsigned int decode(const std::string& in,
           std::string::const_iterator& iter);

      /**
        *   Appends UCS-4 value to a UTF-8 string.
        *   @param ch UCS-4 value.
        *   @param dst destination.
        */
      static void encode(unsigned int ch, std::string& dst);

      /**
       *    Appends string in the current code-page
       *       to a LogString.
       */
      static void decode(const std::string& src, LogString& dst);
      /**
       *     Appends a LogString to a string in the current
       *        code-page.  Unrepresentable characters may be 
       *        replaced with loss characters.
      */
      static void encode(const LogString& src, std::string& dst);

      /**
        *     Encodes the specified LogString to the current
        *       character set. 
        *      @param src string to encode.
        *      @param p pool from which to allocate return value.
        *      @return pool allocated string.
        */
      static char* encode(const LogString& src, log4cxx::helpers::Pool& p);



#if LOG4CXX_WCHAR_T_API || LOG4CXX_LOGCHAR_IS_WCHAR_T || defined(WIN32) || defined(_WIN32)
      static void decode(const std::wstring& src, LogString& dst);
      static void encode(const LogString& src, std::wstring& dst);
      static wchar_t* wencode(const LogString& src, log4cxx::helpers::Pool& p);

      /**
       *   Decodes next character from a wstring.
       *   @param in string from which the character is extracted.
       *   @param iter iterator addressing start of character, will be
       *   advanced to next character if successful.
       *   @return scalar value (UCS-4) or 0xFFFF if invalid sequence.
       */
      static unsigned int decode(const std::wstring& in,
           std::wstring::const_iterator& iter);

      /**
        *   Appends UCS-4 value to a UTF-8 string.
        *   @param ch UCS-4 value.
        *   @param dst destination.
        */
      static void encode(unsigned int ch, std::wstring& dst);

#endif


#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API || LOG4CXX_LOGCHAR_IS_UNICHAR
      static void decode(const std::basic_string<UniChar>& src, LogString& dst);
      static void encode(const LogString& src, std::basic_string<UniChar>& dst);
      
      /**
       *   Decodes next character from a UniChar string.
       *   @param in string from which the character is extracted.
       *   @param iter iterator addressing start of character, will be
       *   advanced to next character if successful.
       *   @return scalar value (UCS-4) or 0xFFFF if invalid sequence.
       */
      static unsigned int decode(const std::basic_string<UniChar>& in,
           std::basic_string<UniChar>::const_iterator& iter);

      /**
        *   Appends UCS-4 value to a UTF-8 string.
        *   @param ch UCS-4 value.
        *   @param dst destination.
        */
      static void encode(unsigned int ch, std::basic_string<UniChar>& dst);

#endif

#if LOG4CXX_CFSTRING_API
      static void decode(const CFStringRef& src, LogString& dst);
      static CFStringRef encode(const LogString& src);
#endif

      enum { LOSSCHAR = 0x3F };
      
      /**
       *   Returns a logchar value given a character literal in the ASCII charset.
       *   Used to implement the LOG4CXX_STR macro for EBCDIC and UNICHAR.
       */
      static logchar decode(char v);
      /**
       *   Returns a LogString given a string literal in the ASCII charset.
       *   Used to implement the LOG4CXX_STR macro for EBCDIC and UNICHAR.
       */
      static LogString decode(const char* v);

      /**
       *   Encodes a charset name in the default encoding 
       *      without using a CharsetEncoder (which could trigger recursion).
       */
      static std::string encodeCharsetName(const LogString& charsetName);

      private:

      private:
      Transcoder();
      Transcoder(const Transcoder&);
      Transcoder& operator=(const Transcoder&);
      enum { BUFSIZE = 256 };
      static size_t encodeUTF8(unsigned int ch, char* dst);
      static size_t encodeUTF16BE(unsigned int ch, char* dst);
      static size_t encodeUTF16LE(unsigned int ch, char* dst);
      
      };
   }
}

#define LOG4CXX_ENCODE_CHAR(var, src) \
std::string var;                      \
log4cxx::helpers::Transcoder::encode(src, var)

#define LOG4CXX_DECODE_CHAR(var, src) \
log4cxx::LogString var;                      \
log4cxx::helpers::Transcoder::decode(src, var)

#define LOG4CXX_DECODE_CFSTRING(var, src) \
log4cxx::LogString var;                      \
log4cxx::helpers::Transcoder::decode(src, var)

#define LOG4CXX_ENCODE_CFSTRING(var, src) \
CFStringRef var = log4cxx::helpers::Transcoder::encode(src)


#if LOG4CXX_LOGCHAR_IS_WCHAR

#define LOG4CXX_ENCODE_WCHAR(var, src) \
const std::wstring& var = src

#define LOG4CXX_DECODE_WCHAR(var, src) \
const log4cxx::LogString& var = src

#else

#define LOG4CXX_ENCODE_WCHAR(var, src) \
std::wstring var;                      \
log4cxx::helpers::Transcoder::encode(src, var)

#define LOG4CXX_DECODE_WCHAR(var, src) \
log4cxx::LogString var;                      \
log4cxx::helpers::Transcoder::decode(src, var)

#endif

#if LOG4CXX_LOGCHAR_IS_UNICHAR

#define LOG4CXX_ENCODE_UNICHAR(var, src) \
const std::basic_string<UniChar>& var = src

#define LOG4CXX_DECODE_UNICHAR(var, src) \
const log4cxx::LogString& var = src

#else

#define LOG4CXX_ENCODE_UNICHAR(var, src) \
std::basic_string<UniChar> var;          \
log4cxx::helpers::Transcoder::encode(src, var)

#define LOG4CXX_DECODE_UNICHAR(var, src) \
log4cxx::LogString var;                      \
log4cxx::helpers::Transcoder::decode(src, var)

#endif

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif //_LOG4CXX_HELPERS_TRANSCODER_H
