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

#ifndef _LOG4CXX_HELPERS_INPUTSTREAM_H
#define _LOG4CXX_HELPERS_INPUTSTREAM_H

#include <log4cxx/helpers/objectimpl.h>

namespace log4cxx
{

        namespace helpers {
          class ByteBuffer;

          /**
           * Abstract class for reading from character streams.
           * 
           */
          class LOG4CXX_EXPORT InputStream : public ObjectImpl
          {
          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(InputStream)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(InputStream)
                  END_LOG4CXX_CAST_MAP()

          protected:
                  InputStream();

                  virtual ~InputStream();

          public:
                  /**
                   * Reads a sequence of bytes into the given buffer.
                   *
                   * @param dst The buffer into which bytes are to be transferred.
                   * @return the total number of bytes read into the buffer, or -1 if there
                   *         is no more data because the end of the stream has been reached.
                   */
                  virtual int read(ByteBuffer& dst) = 0;

                  /**
                   * Closes this input stream and releases any system 
                   * resources associated with the stream.
                   */
                  virtual void close() = 0;

          private:
                  InputStream(const InputStream&);
                  InputStream& operator=(const InputStream&);
          };

          LOG4CXX_PTR_DEF(InputStream);
        } // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_INPUTSTREAM_H
