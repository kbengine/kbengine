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

#ifndef _LOG4CXX_HELPERS_CHARSETDECODER_H
#define _LOG4CXX_HELPERS_CHARSETDECODER_H

#include <log4cxx/helpers/objectimpl.h>

namespace log4cxx
{
        namespace helpers {
          class CharsetDecoder;
          LOG4CXX_PTR_DEF(CharsetDecoder);
          class ByteBuffer;


          /**
          *   An abstract engine to transform a sequences of bytes in a specific charset
        *   into a LogString.
          */
          class LOG4CXX_EXPORT CharsetDecoder : public ObjectImpl
          {
          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(CharsetDecoder)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(CharsetDecoder)
                  END_LOG4CXX_CAST_MAP()
          protected:
               /**
               *  Protected constructor.
               */
                  CharsetDecoder();
          public:
               /**
               *  Destructor.
               */
                  virtual ~CharsetDecoder();

              /**
               *   Get decoder for default charset.
               */
                  static CharsetDecoderPtr getDefaultDecoder();
              /**
               *  Get decoder for specified character set.
               *  @param charset the following values should be recognized:
               *     "US-ASCII", "ISO-8859-1", "UTF-8",
               *     "UTF-16BE", "UTF-16LE".
               *  @return decoder
               *  @throws IllegalArgumentException if charset is not recognized.
               */
              static CharsetDecoderPtr getDecoder(const LogString& charset);

              /**
               *   Get decoder for UTF-8.
               */
                  static CharsetDecoderPtr getUTF8Decoder();
              /**
               *   Get decoder for ISO-8859-1.
               */
                  static CharsetDecoderPtr getISOLatinDecoder();
                  
                  

              /**
               *  Decodes as many bytes as possible from the given
               *   input buffer, writing the results to the given output string.
               *  @param in input buffer.
               *  @param out output string.
               *  @return APR_SUCCESS if not encoding errors were found.
               */
                  virtual log4cxx_status_t decode(ByteBuffer& in,
                        LogString& out) = 0;

              /**
               *  Determins if status value indicates an invalid byte sequence.
               */
                  inline static bool isError(log4cxx_status_t stat) {
                     return (stat != 0);
                  }

          private:
               /**
               *  Private copy constructor.
               */
                  CharsetDecoder(const CharsetDecoder&);
               /**
               *  Private assignment operator.
               */
                  CharsetDecoder& operator=(const CharsetDecoder&);
              /**
               *  Creates a new decoder for the default charset.
               */
                  static CharsetDecoder* createDefaultDecoder();
        };

        } // namespace helpers
}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_CHARSETENCODER_H
