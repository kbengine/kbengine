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
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <stdlib.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/charsetdecoder.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <vector>
#include <apr.h>
#include <apr_strings.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>

#if LOG4CXX_LOGCHAR_IS_UNICHAR || LOG4CXX_CFSTRING_API || LOG4CXX_UNICHAR_API
#include <CoreFoundation/CFString.h>
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;


void Transcoder::decodeUTF8(const std::string& src, LogString& dst) {
     std::string::const_iterator iter = src.begin();
     while(iter != src.end()) {
         unsigned int sv = decode(src, iter);
         if(sv != 0xFFFF) {
            encode(sv, dst);
         } else {
            dst.append(1, LOSSCHAR);
            iter++;
        }
     }
}

void Transcoder::encodeUTF8(const LogString& src, std::string& dst) {
#if LOG4CXX_LOGCHAR_IS_UTF8
     dst.append(src);
#else
     LogString::const_iterator iter = src.begin();
     while(iter != src.end()) {
         unsigned int sv = decode(src, iter);
         if(sv != 0xFFFF) {
            encode(sv, dst);
         } else {
            dst.append(1, LOSSCHAR);
            iter++;
        }
     }
#endif     
}

char* Transcoder::encodeUTF8(const LogString& src, Pool& p) {
#if LOG4CXX_LOGCHAR_IS_UTF8
     return p.pstrdup(src);
#else
     std::string tmp;
     encodeUTF8(src, tmp);
     return p.pstrdup(tmp);
#endif     
}


void Transcoder::encodeUTF8(unsigned int sv, ByteBuffer& dst) {
    size_t bytes = encodeUTF8(sv, dst.current());
    dst.position(dst.position() + bytes);
}


size_t Transcoder::encodeUTF8(unsigned int ch, char* dst) {
    if (ch < 0x80) {
        dst[0] = (char) ch;
        return 1;
    } else if (ch < 0x800) {
        dst[0] = (char) (0xC0 + (ch >> 6));
        dst[1] = (char) (0x80 + (ch & 0x3F));
        return 2;
    } else if (ch < 0x10000) {
        dst[0] = (char) (0xE0 + (ch >> 12));
        dst[1] = (char) (0x80 + ((ch >> 6) & 0x3F));
        dst[2] = (char) (0x80 + (ch & 0x3F));
        return 3;
    } else if (ch <= 0x10FFFF) {
        dst[0] = (char) (0xF0 + (ch >> 18));
        dst[1] = (char) (0x80 + ((ch >> 12) & 0x3F));
        dst[2] = (char) (0x80 + ((ch >> 6) & 0x3F));
        dst[3] = (char) (0x80 + (ch & 0x3F));
        return 4;
    } else {
        //
        //  output UTF-8 encoding of 0xFFFF
        //
        dst[0] = (char) 0xEF;
        dst[1] = (char) 0xBF;
        dst[2] = (char) 0xBF;
        return 3;
    }
}

void Transcoder::encodeUTF16BE(unsigned int sv, ByteBuffer& dst) {
    size_t bytes = encodeUTF16BE(sv, dst.current());
    dst.position(dst.position() + bytes);
}


size_t Transcoder::encodeUTF16BE(unsigned int ch, char* dst) {
    if (ch <= 0xFFFF) {
        dst[0] = (char) (ch >> 8);
        dst[1] = (char) (ch & 0xFF);
        return 2;
    }
    if (ch <= 0x10FFFF) {
        unsigned char w = (unsigned char) ((ch >> 16) - 1);
        dst[0] = (char) (0xD8 + (w >> 2));
        dst[1] = (char) (((w & 0x03) << 6) + ((ch >> 10) & 0x3F));
        dst[2] = (char) (0xDC + ((ch & 0x30) >> 4));
        dst[3] = (char) (ch & 0xFF);
        return 4;
    }
    dst[0] = dst[1] = (char) 0xFF;
    return 2;
}

void Transcoder::encodeUTF16LE(unsigned int sv, ByteBuffer& dst) {
    size_t bytes = encodeUTF16LE(sv, dst.current());
    dst.position(dst.position() + bytes);
}

size_t Transcoder::encodeUTF16LE(unsigned int ch, char* dst) {
    if (ch <= 0xFFFF) {
        dst[1] = (char) (ch >> 8);
        dst[0] = (char) (ch & 0xFF);
        return 2;
    }
    if (ch <= 0x10FFFF) {
        unsigned char w = (unsigned char) ((ch >> 16) - 1);
        dst[1] = (char) (0xD8 + (w >> 2));
        dst[0] = (char) (((w & 0x03) << 6) + ((ch >> 10) & 0x3F));
        dst[3] = (char) (0xDC + ((ch & 0x30) >> 4));
        dst[2] = (char) (ch & 0xFF);
        return 4;
    }
    dst[0] = dst[1] = (char) 0xFF;
    return 2;
}


unsigned int Transcoder::decode(const std::string& src,
                                       std::string::const_iterator& iter) {
  std::string::const_iterator start(iter);
  unsigned char ch1 = *(iter++);
  if (ch1 <= 0x7F) {
      return ch1;
  }
  //
  //   should not have continuation character here
  //
  if ((ch1 & 0xC0) != 0x80 && iter != src.end()) {
      unsigned char ch2 = *(iter++);
      //
      //   should be continuation
      if ((ch2 & 0xC0) != 0x80) {
          iter = start;
          return 0xFFFF;
      }
      if((ch1 & 0xE0) == 0xC0) {
          if ((ch2 & 0xC0) == 0x80) {
              unsigned int rv = ((ch1 & 0x1F) << 6) + (ch2 & 0x3F);
              if (rv >= 0x80) {
                  return rv;
              }
          }
          iter = start;
          return 0xFFFF;
      }
      if (iter != src.end()) {
          unsigned char ch3 = *(iter++);
          //
          //   should be continuation
          //
          if ((ch3 & 0xC0) != 0x80) {
              iter = start;
              return 0xFFFF;
          }
          if ((ch1 & 0xF0) == 0xE0) {
              unsigned rv = ((ch1 & 0x0F) << 12)
              + ((ch2 & 0x3F) << 6)
              + (ch3 & 0x3F);
              if (rv <= 0x800) {
                  iter = start;
                  return 0xFFFF;
              }
              return rv;
          }
          if (iter != src.end()) {
              unsigned char ch4 = *(iter++);
              if ((ch4 & 0xC0) != 0x80) {
                  iter = start;
                  return 0xFFFF;
              }
              unsigned int rv = ((ch1 & 0x07) << 18)
                     + ((ch2 & 0x3F) << 12)
                     + ((ch3 & 0x3F) << 6)
                     + (ch4 & 0x3F);
              if (rv > 0xFFFF) {
                  return rv;
              }

          }
      }
  }
  iter = start;
  return 0xFFFF;
}


void Transcoder::encode(unsigned int sv, std::string& dst) {
    char tmp[8];
    size_t bytes = encodeUTF8(sv, tmp);
    dst.append(tmp, bytes);
}


void Transcoder::decode(const std::string& src, LogString& dst) {
#if LOG4CXX_CHARSET_UTF8 && LOG4CXX_LOGCHAR_IS_UTF8
   dst.append(src);
#else
   static CharsetDecoderPtr decoder(CharsetDecoder::getDefaultDecoder());
   dst.reserve(dst.size() + src.size());
   std::string::const_iterator iter = src.begin();
#if !LOG4CXX_CHARSET_EBCDIC
   for(;
       iter != src.end() && ((unsigned char) *iter) < 0x80;
       iter++) {
       dst.append(1, *iter);
   }
#endif
  if (iter != src.end()) {   
    size_t offset = iter - src.begin();
    ByteBuffer buf(const_cast<char*>(src.data() + offset), src.size() - offset);
    while(buf.remaining() > 0) {
      log4cxx_status_t stat = decoder->decode(buf, dst);
      if(CharsetDecoder::isError(stat)) {
        dst.append(1, LOSSCHAR);
        buf.position(buf.position() + 1);
      }
    }
    decoder->decode(buf, dst);
  }
#endif  
}

char* Transcoder::encode(const LogString& src, Pool& p) {
#if LOG4CXX_CHARSET_UTF8 && LOG4CXX_LOGCHAR_IS_UTF8
   return p.pstrdup(src);
#else
   std::string tmp;
   encode(src, tmp);
   return p.pstrdup(tmp);
#endif
}



void Transcoder::encode(const LogString& src, std::string& dst) {
#if LOG4CXX_CHARSET_UTF8 && LOG4CXX_LOGCHAR_IS_UTF8
   dst.append(src);
#else
   static CharsetEncoderPtr encoder(CharsetEncoder::getDefaultEncoder());
   dst.reserve(dst.size() + src.size());
   LogString::const_iterator iter = src.begin();
#if !LOG4CXX_CHARSET_EBCDIC
   for(;
       iter != src.end() && ((unsigned int) *iter) < 0x80;
       iter++) {
       dst.append(1, *iter);
   }
#endif
  if (iter != src.end()) {
    char buf[BUFSIZE];
    ByteBuffer out(buf, BUFSIZE);
    while(iter != src.end()) {
      log4cxx_status_t stat = encoder->encode(src, iter, out);
      out.flip();
      dst.append(out.data(), out.limit());
      out.clear();
      if (CharsetEncoder::isError(stat)) {
        dst.append(1, LOSSCHAR);
        iter++;
      }
    }
    encoder->encode(src, iter, out);
  }
#endif  
}


template<class String, class Iterator>
static unsigned int decodeUTF16(const String& in, Iterator& iter) {
    unsigned int ch1 = *iter;
    //
    //   if not surrogate pair
    //
    if (ch1 < 0xD800 || ch1 > 0xDFFF) {
        //
        //  then advance iterator and return wchar_t value
        //
        if(ch1 != 0xFFFF) iter++;
        return ch1;
    } else if (ch1 < 0xDC00) {
        //
        //  started with high-surrogate value
        //     if there is an additional wchar_t
        Iterator iter2 = iter + 1;
        if (iter2 != in.end()) {
           unsigned int ch2 = *iter2;
           //
           //    if it is a matching low surrogate then
           //       advance the iterator and return the scalar value
           if (ch2 >= 0xDC00 && ch2 <= 0xDFFF) {
              iter += 2;
              return (ch1 - 0xD800) * 0x400 + (ch2 - 0xDC00) + 0x10000;
           }
        }
    }
    //
    //    unrecognized value, do not advance iterator
    //
    return 0xFFFF;
}

template<class String>
static void encodeUTF16(unsigned int sv, String& dst) {
    if (sv < 0x10000) {
        dst.append(1, sv);
    } else {
        unsigned char u = (unsigned char) (sv >> 16);
        unsigned char w = (unsigned char) (u - 1);
        unsigned short hs = (0xD800 + ((w & 0xF) << 6) + ((sv & 0xFFFF) >> 10));
        unsigned short ls = (0xDC00 + (sv && 0x3FF));
        dst.append(1, hs);
        dst.append(1, ls);
    }
}



#if LOG4CXX_WCHAR_T_API || LOG4CXX_LOGCHAR_IS_WCHAR_T || defined(WIN32) || defined(_WIN32)
void Transcoder::decode(const std::wstring& src, LogString& dst) {
#if LOG4CXX_LOGCHAR_IS_WCHAR_T
  dst.append(src, len);
#else
  std::wstring::const_iterator i = src.begin();
  while(i != src.end()) {
      unsigned int cp = decode(src, i);
      if (cp != 0xFFFF) {
        encode(cp, dst);
      } else {
        dst.append(1, LOSSCHAR);
        i++;
      }
  }
#endif  
}

void Transcoder::encode(const LogString& src, std::wstring& dst) {
#if LOG4CXX_LOGCHAR_IS_WCHAR_T
  dst.append(src);
#else
  for(LogString::const_iterator i = src.begin();
      i != src.end();) {
      unsigned int cp = Transcoder::decode(src, i);
      encode(cp, dst);
  }
#endif
}

wchar_t* Transcoder::wencode(const LogString& src, Pool& p) {
#if LOG4CXX_LOGCHAR_IS_WCHAR_T
    std::wstring& tmp = src;
#else
    std::wstring tmp;
    encode(src, tmp);
#endif
    wchar_t* dst = (wchar_t*) p.palloc((tmp.length() + 1) * sizeof(wchar_t));
    dst[tmp.length()] = 0;
    memcpy(dst, tmp.data(), tmp.length() * sizeof(wchar_t));
    return dst;
}    


unsigned int Transcoder::decode(const std::wstring& in,
    std::wstring::const_iterator& iter) {
#if defined(__STDC_ISO_10646__)
    return *(iter++);
#else
    return decodeUTF16(in, iter);
#endif
}


void Transcoder::encode(unsigned int sv, std::wstring& dst) {
#if defined(__STDC_ISO_10646__)
    dst.append(1, sv);
#else
    if (sizeof(wchar_t) == 4) {
        dst.append(1, sv);
    } else {
        encodeUTF16(sv, dst);
    }
#endif
}

#endif



#if LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
void Transcoder::decode(const std::basic_string<UniChar>& src, LogString& dst) {
#if LOG4CXX_LOGCHAR_IS_UNICHAR
     dst.append(src);
#else
     for(std::basic_string<UniChar>::const_iterator i = src.begin();
         i != src.end();) {
         unsigned int cp = decode(src, i);
         encode(cp, dst);
     }
#endif     
}

void Transcoder::encode(const LogString& src, std::basic_string<UniChar>& dst) {
#if LOG4CXX_LOGCHAR_IS_UNICHAR
     dst.append(src);
#else
  for(LogString::const_iterator i = src.begin();
      i != src.end();) {
      unsigned int cp = decode(src, i);
      encode(cp, dst);
  }
#endif
}

unsigned int Transcoder::decode(const std::basic_string<UniChar>& in,
    std::basic_string<UniChar>::const_iterator& iter) {
    return decodeUTF16(in, iter);
}

void Transcoder::encode(unsigned int sv, std::basic_string<UniChar>& dst) {
    encodeUTF16(sv, dst);
}

#endif

#if LOG4CXX_CFSTRING_API
void Transcoder::decode(const CFStringRef& src, LogString& dst) {
     const UniChar* chars = CFStringGetCharactersPtr(src);
     if (chars) {
          decode(chars, dst);
     } else {
          size_t length = CFStringGetLength(src);
          if (length > 0) {
              std::vector<UniChar> tmp(length);
              CFStringGetCharacters(src, CFRangeMake(0, length), &tmp[0]);
#if LOG4CXX_LOGCHAR_IS_UNICHAR
              dst.append(&tmp[0], tmp.size());
#else
              decode(std::basic_string<UniChar>(&tmp[0], tmp.size()), dst);
#endif              
          }
    }
}

CFStringRef Transcoder::encode(const LogString& src) {
    LOG4CXX_ENCODE_UNICHAR(tmp, src);
    return CFStringCreateWithCharacters(kCFAllocatorDefault, tmp.data(), tmp.size());
}
#endif


logchar Transcoder::decode(char val) {
#if LOG4CXX_CHARSET_EBCDIC
   LogString dst;
   Transcoder::decode(std::string(1, val), dst);
   return dst[0];
#else
   return val;
#endif
}

LogString Transcoder::decode(const char* val) {
#if LOG4CXX_LOGCHAR_IS_UTF8 && !LOG4CXX_CHARSET_EBCDIC
    return val;
#else
    LogString dst;
    Transcoder::decode(val, dst);
    return dst;
#endif
}


std::string Transcoder::encodeCharsetName(const LogString& val) {
     char asciiTable[] = { ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',', '-', '.', '/',
                           '0', '1', '2', '3', '4', '5', '6' , '7', '8', '9', ':', ';', '<', '=', '>', '?',
                           '@', 'A', 'B', 'C', 'D', 'E', 'F',  'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                           'P', 'Q', 'R', 'S', 'T', 'U', 'V',  'W', 'X', 'Y', 'Z', '[', '\\', ']', '^', '_',
                           '`', 'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
                           'p', 'q', 'r', 's', 't', 'u', 'v',  'w', 'x', 'y', 'z', '{', '|', '}', '~', ' ' };
    std::string out;
    for(LogString::const_iterator iter = val.begin();
        iter != val.end();
        iter++) {
        if (*iter >= 0x30 && *iter < 0x7F) {
            out.append(1, asciiTable[*iter - 0x30]);
        } else {
            out.append(1, LOSSCHAR);
        }
    }
    return out;
}
