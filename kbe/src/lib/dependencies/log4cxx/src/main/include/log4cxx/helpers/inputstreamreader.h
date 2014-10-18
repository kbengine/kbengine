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

#ifndef _LOG4CXX_HELPERS_INPUTSTREAMREADER_H
#define _LOG4CXX_HELPERS_INPUTSTREAMREADER_H

#include <log4cxx/helpers/reader.h>
#include <log4cxx/helpers/inputstream.h>
#include <log4cxx/helpers/charsetdecoder.h>

namespace log4cxx
{

        namespace helpers {

          /**
           * Class for reading from character streams.
           * Decorates a byte based InputStream and provides appropriate 
           * conversion to characters.
           */
          class LOG4CXX_EXPORT InputStreamReader : public Reader
          {
          private:
                  InputStreamPtr in;
                  CharsetDecoderPtr dec;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(InputStreamReader)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(InputStreamReader)
                          LOG4CXX_CAST_ENTRY_CHAIN(Reader)
                  END_LOG4CXX_CAST_MAP()

                  /**
                   * Creates an InputStreamReader that uses the default charset.
                   *
                   * @param in The input stream to decorate.
                   */
                  InputStreamReader(const InputStreamPtr& in);

                  /**
                   * Creates an InputStreamReader that uses the given charset decoder.
                   *
                   * @param in The input stream to decorate.
                   * @param enc The charset decoder to use for the conversion.
                   */
                  InputStreamReader(const InputStreamPtr& in, const CharsetDecoderPtr &enc);

                  ~InputStreamReader();

                  /**
                   * Closes the stream.
                   *
                   * @param p The memory pool associated with the reader.
                   */
                  virtual void close(Pool& p);

                  /**
                   * @return The complete stream contents as a LogString.
                   * @param p The memory pool associated with the reader.
                   */
                  virtual LogString read(Pool& p);

                  /**
                   * @return The name of the character encoding being used by this stream.
                   */
                  LogString getEncoding() const;

          private:
                  InputStreamReader(const InputStreamReader&);

                  InputStreamReader& operator=(const InputStreamReader&);
          };

          LOG4CXX_PTR_DEF(InputStreamReader);
        } // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_INPUTSTREAMREADER_H
