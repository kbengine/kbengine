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
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/exception.h>
#include <apr_xlate.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#include <apr_portable.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/synchronized.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(CharsetEncoder)

namespace log4cxx
{

        namespace helpers {

#if APR_HAS_XLATE
          /**
          * A character encoder implemented using apr_xlate.
          */
          class APRCharsetEncoder : public CharsetEncoder
          {
          public:
              APRCharsetEncoder(const LogString& topage) : pool(), mutex(pool) {
#if LOG4CXX_LOGCHAR_IS_WCHAR
                  const char* frompage = "WCHAR_T";
#endif
#if LOG4CXX_LOGCHAR_IS_UTF8
                  const char* frompage = "UTF-8";
#endif
#if LOG4CXX_LOGCHAR_IS_UNICHAR
                  const char* frompage = "UTF-16";
#endif
                  std::string tpage(Transcoder::encodeCharsetName(topage));
                  apr_status_t stat = apr_xlate_open(&convset,
                     tpage.c_str(),
                     frompage,
                     pool.getAPRPool());
                  if (stat != APR_SUCCESS) {
                     throw IllegalArgumentException(topage);
                  }
              }

              virtual ~APRCharsetEncoder() {
              }

              virtual log4cxx_status_t encode(const LogString& in,
                    LogString::const_iterator& iter,
                    ByteBuffer& out) {
                      apr_status_t stat;
                      size_t outbytes_left = out.remaining();
                      size_t initial_outbytes_left = outbytes_left;
                      size_t position = out.position();
                      if (iter == in.end()) {
                        synchronized sync(mutex);
                        stat = apr_xlate_conv_buffer(convset, NULL, NULL,
                           out.data() + position, &outbytes_left);
                      } else {
                        LogString::size_type inOffset = (iter - in.begin());
                        apr_size_t inbytes_left =
                            (in.size() - inOffset) * sizeof(LogString::value_type);
                        apr_size_t initial_inbytes_left = inbytes_left;
                        {
                             synchronized sync(mutex);
                             stat = apr_xlate_conv_buffer(convset,
                                (const char*) (in.data() + inOffset),
                                &inbytes_left,
                                out.data() + position,
                                &outbytes_left);
                        }
                        iter += ((initial_inbytes_left - inbytes_left) / sizeof(LogString::value_type));
                      }
                      out.position(out.position() + (initial_outbytes_left - outbytes_left));
                      return stat;
              }

          private:
                  APRCharsetEncoder(const APRCharsetEncoder&);
                  APRCharsetEncoder& operator=(const APRCharsetEncoder&);
                  Pool pool;
                  Mutex mutex;
                  apr_xlate_t *convset;
          };
#endif

#if LOG4CXX_LOGCHAR_IS_WCHAR && LOG4CXX_HAS_WCSTOMBS
          /**
           *  A character encoder implemented using wcstombs.
          */
          class WcstombsCharsetEncoder : public CharsetEncoder
          {
          public:
              WcstombsCharsetEncoder() {
              }

           /**
            *   Converts a wchar_t to the default external multibyte encoding.
            */
              log4cxx_status_t encode(const LogString& in,
                    LogString::const_iterator& iter,
                    ByteBuffer& out) {
                      log4cxx_status_t stat = APR_SUCCESS;

                      if (iter != in.end()) {
                         size_t outbytes_left = out.remaining();
                         size_t position = out.position();
                         std::wstring::size_type inOffset = (iter - in.begin());
                         enum { BUFSIZE = 256 };
                         wchar_t buf[BUFSIZE];
                         size_t chunkSize = BUFSIZE - 1;
                         if (chunkSize * MB_LEN_MAX > outbytes_left) {
                             chunkSize = outbytes_left / MB_LEN_MAX;
                         }
                         if (chunkSize > in.length() - inOffset) {
                             chunkSize = in.length() - inOffset;
                         }
                         memset(buf, 0, BUFSIZE * sizeof(wchar_t));
                         memcpy(buf,
                             in.data() + inOffset,
                             chunkSize * sizeof(wchar_t));
                         size_t converted = wcstombs(out.data() + position, buf, outbytes_left);

                         if (converted == (size_t) -1) {
                             stat = APR_BADARG;
                             //
                             //   if unconvertable character was encountered
                             //       repeatedly halve source to get fragment that
                             //       can be converted
                             for(chunkSize /= 2;
                                 chunkSize > 0;
                                 chunkSize /= 2) {
                                 buf[chunkSize] = 0;
                                 converted = wcstombs(out.data() + position, buf, outbytes_left);
                                 if (converted != (size_t) -1) {
                                    iter += chunkSize;
                                    out.position(out.position() + converted);
                           break;
                                 }
                             }
                         } else {
                            iter += chunkSize;
                            out.position(out.position() + converted);
                         }
                      }
                      return stat;
              }



          private:
                  WcstombsCharsetEncoder(const WcstombsCharsetEncoder&);
                  WcstombsCharsetEncoder& operator=(const WcstombsCharsetEncoder&);
          };
#endif


          /**
          *   Encodes a LogString to US-ASCII.
          */
          class USASCIICharsetEncoder : public CharsetEncoder
          {
          public:
              USASCIICharsetEncoder() {
              }

              virtual log4cxx_status_t encode(const LogString& in,
                    LogString::const_iterator& iter,
                    ByteBuffer& out) {
                  log4cxx_status_t stat = APR_SUCCESS;
                  if (iter != in.end()) {
                      while(out.remaining() > 0 && iter != in.end()) {
                          LogString::const_iterator prev(iter);
                          unsigned int sv = Transcoder::decode(in, iter);
                          if (sv <= 0x7F) {
                              out.put((char) sv);
                          } else {
                              iter = prev;
                              stat = APR_BADARG;
                              break;
                          }
                      }
                  }
                  return stat;
              }

          private:
                  USASCIICharsetEncoder(const USASCIICharsetEncoder&);
                  USASCIICharsetEncoder& operator=(const USASCIICharsetEncoder&);
          };

          /**
          *   Converts a LogString to ISO-8859-1.
          */
          class ISOLatinCharsetEncoder : public CharsetEncoder
          {
          public:
              ISOLatinCharsetEncoder() {
              }

              virtual log4cxx_status_t encode(const LogString& in,
                    LogString::const_iterator& iter,
                    ByteBuffer& out) {
                  log4cxx_status_t stat = APR_SUCCESS;
                  if (iter != in.end()) {
                      while(out.remaining() > 0 && iter != in.end()) {
                          LogString::const_iterator prev(iter);
                          unsigned int sv = Transcoder::decode(in, iter);
                          if (sv <= 0xFF) {
                              out.put((char) sv);
                          } else {
                              iter = prev;
                              stat = APR_BADARG;
                              break;
                          }
                      }
                  }
                  return stat;
              }

          private:
                  ISOLatinCharsetEncoder(const ISOLatinCharsetEncoder&);
                  ISOLatinCharsetEncoder& operator=(const ISOLatinCharsetEncoder&);
          };

          /**
          *   Encodes a LogString to a byte array when the encodings are identical.
          */
          class TrivialCharsetEncoder : public CharsetEncoder
          {
          public:
              TrivialCharsetEncoder() {
              }


              virtual log4cxx_status_t encode(const LogString& in,
                    LogString::const_iterator& iter,
                    ByteBuffer& out) {
                  if(iter != in.end()) {
                 size_t requested = in.length() - (iter - in.begin());
                 if (requested > out.remaining()/sizeof(logchar)) {
                    requested = out.remaining()/sizeof(logchar);
                 }
                 memcpy(out.current(),
                       (const char*) in.data() + (iter - in.begin()),
                      requested * sizeof(logchar));
                 iter += requested;
                 out.position(out.position() + requested * sizeof(logchar));
              }
                  return APR_SUCCESS;
              }

          private:
                  TrivialCharsetEncoder(const TrivialCharsetEncoder&);
                  TrivialCharsetEncoder& operator=(const TrivialCharsetEncoder&);
          };

#if LOG4CXX_LOGCHAR_IS_UTF8
typedef TrivialCharsetEncoder UTF8CharsetEncoder;
#else
/**
 *  Converts a LogString to UTF-8.
 */
class UTF8CharsetEncoder : public CharsetEncoder {
public:
    UTF8CharsetEncoder() {
    }

    virtual log4cxx_status_t encode(const LogString& in,
         LogString::const_iterator& iter,
         ByteBuffer& out) {
         while(iter != in.end() && out.remaining() >= 8) {
              unsigned int sv = Transcoder::decode(in, iter);
              if (sv == 0xFFFF) {
                   return APR_BADARG;
              }
              Transcoder::encodeUTF8(sv, out);
         }
         return APR_SUCCESS;
     }

private:
     UTF8CharsetEncoder(const UTF8CharsetEncoder&);
     UTF8CharsetEncoder& operator=(const UTF8CharsetEncoder&);
};
#endif

/**
 *   Encodes a LogString to UTF16-BE.
 */
class UTF16BECharsetEncoder : public CharsetEncoder {
public:
     UTF16BECharsetEncoder() {
     }

     virtual log4cxx_status_t encode(const LogString& in,
             LogString::const_iterator& iter,
             ByteBuffer& out) {
             while(iter != in.end() && out.remaining() >= 4) {
                  unsigned int sv = Transcoder::decode(in, iter);
                  if (sv == 0xFFFF) {
                      return APR_BADARG;
                  }
                  Transcoder::encodeUTF16BE(sv, out);
             }
             return APR_SUCCESS;
     }

private:
     UTF16BECharsetEncoder(const UTF16BECharsetEncoder&);
     UTF16BECharsetEncoder& operator=(const UTF16BECharsetEncoder&);
};

/**
 *   Encodes a LogString to UTF16-LE.
 */
class UTF16LECharsetEncoder : public CharsetEncoder {
public:
     UTF16LECharsetEncoder() {
     }


     virtual log4cxx_status_t encode(const LogString& in,
             LogString::const_iterator& iter,
             ByteBuffer& out) {
             while(iter != in.end() && out.remaining() >= 4) {
                  unsigned int sv = Transcoder::decode(in, iter);
                  if (sv == 0xFFFF) {
                      return APR_BADARG;
                  }
                  Transcoder::encodeUTF16LE(sv, out);
             }
             return APR_SUCCESS;
     }
private:
     UTF16LECharsetEncoder(const UTF16LECharsetEncoder&);
     UTF16LECharsetEncoder& operator=(const UTF16LECharsetEncoder&);
};

/**
 *    Charset encoder that uses an embedded CharsetEncoder consistent
 *     with current locale settings.
 */
class LocaleCharsetEncoder : public CharsetEncoder {
public:
      LocaleCharsetEncoder() : pool(), mutex(pool), encoder(), encoding() {
      }
      virtual ~LocaleCharsetEncoder() {
      }
      virtual log4cxx_status_t encode(const LogString& in,
            LogString::const_iterator& iter,
            ByteBuffer& out) {
#if !LOG4CXX_CHARSET_EBCDIC
            char* current = out.current();
            size_t remain = out.remaining();
            for(;
                iter != in.end() && ((unsigned int) *iter) < 0x80 && remain > 0; 
                iter++, remain--, current++) {
                *current = *iter;
            }
            out.position(current - out.data());
#endif
            if (iter != in.end() && out.remaining() > 0) {  
                  Pool subpool;
                  const char* enc = apr_os_locale_encoding(subpool.getAPRPool());
                  {
                       synchronized sync(mutex);
                       if (enc == 0) {
                            if (encoder == 0) {
                                encoding = "C";
                                encoder = new USASCIICharsetEncoder();
                            }
                        } else if (encoding != enc) {
                            encoding = enc;
                            LogString ename;
                            Transcoder::decode(encoding, ename);
                            try {
                                encoder = CharsetEncoder::getEncoder(ename);
                            } catch(IllegalArgumentException ex) {
                                encoder = new USASCIICharsetEncoder();
                            }
                        }
                  }
                  return encoder->encode(in, iter, out);
            }
            return APR_SUCCESS;
      }

private:
      LocaleCharsetEncoder(const LocaleCharsetEncoder&);
      LocaleCharsetEncoder& operator=(const LocaleCharsetEncoder&);
      Pool pool;
      Mutex mutex;
      CharsetEncoderPtr encoder;
      std::string encoding;
};


        } // namespace helpers

}  //namespace log4cxx



CharsetEncoder::CharsetEncoder() {
}

CharsetEncoder::~CharsetEncoder() {
}

CharsetEncoderPtr CharsetEncoder::getDefaultEncoder() {
  static CharsetEncoderPtr encoder(createDefaultEncoder());
  //
  //  if invoked after static variable destruction
  //     (if logging is called in the destructor of a static object)
  //     then create a new decoder.
  // 
  if (encoder == 0) {
       return createDefaultEncoder();
  }
  return encoder;
}

CharsetEncoder* CharsetEncoder::createDefaultEncoder() {
#if LOG4CXX_CHARSET_UTF8
   return new UTF8CharsetEncoder();
#elif LOG4CXX_CHARSET_ISO88591
   return new ISOLatinCharsetEncoder();
#elif LOG4CXX_CHARSET_USASCII
   return new USASCIICharsetEncoder();
#elif LOG4CXX_LOGCHAR_IS_WCHAR && LOG4CXX_HAS_WCSTOMBS
  return new WcstombsCharsetEncoder();
#else
  return new LocaleCharsetEncoder();
#endif
}


CharsetEncoderPtr CharsetEncoder::getUTF8Encoder() {
    return new UTF8CharsetEncoder();
}



CharsetEncoderPtr CharsetEncoder::getEncoder(const LogString& charset) {
    if (StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("UTF-8"), LOG4CXX_STR("utf-8"))) {
        return new UTF8CharsetEncoder();
    } else if (StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("C"), LOG4CXX_STR("c")) ||
        charset == LOG4CXX_STR("646") ||
        StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("US-ASCII"), LOG4CXX_STR("us-ascii")) ||
        StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("ISO646-US"), LOG4CXX_STR("iso646-US")) ||
        StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("ANSI_X3.4-1968"), LOG4CXX_STR("ansi_x3.4-1968"))) {
        return new USASCIICharsetEncoder();
    } else if (StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("ISO-8859-1"), LOG4CXX_STR("iso-8859-1")) ||
        StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("ISO-LATIN-1"), LOG4CXX_STR("iso-latin-1"))) {
        return new ISOLatinCharsetEncoder();
    } else if (StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("UTF-16BE"), LOG4CXX_STR("utf-16be"))
        || StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("UTF-16"), LOG4CXX_STR("utf-16"))) {
        return new UTF16BECharsetEncoder();
    } else if (StringHelper::equalsIgnoreCase(charset, LOG4CXX_STR("UTF-16LE"), LOG4CXX_STR("utf-16le"))) {
        return new UTF16LECharsetEncoder();
    }
#if APR_HAS_XLATE || !defined(_WIN32)
    return new APRCharsetEncoder(charset);
#else    
    throw IllegalArgumentException(charset);
#endif
}


void CharsetEncoder::reset() {
}

void CharsetEncoder::flush(ByteBuffer& /* out */ ) {
}


void CharsetEncoder::encode(CharsetEncoderPtr& enc,
    const LogString& src,
    LogString::const_iterator& iter,
    ByteBuffer& dst) {
    log4cxx_status_t stat = enc->encode(src, iter, dst);
    if (stat != APR_SUCCESS && iter != src.end()) {
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_LOGCHAR_IS_UNICHAR
      iter++;
#elif LOG4CXX_LOGCHAR_IS_UTF8
      //  advance past this character and all continuation characters
     while((*(++iter) & 0xC0) == 0x80);
#else
#error logchar is unrecognized
#endif
      dst.put(Transcoder::LOSSCHAR);
    }
}
