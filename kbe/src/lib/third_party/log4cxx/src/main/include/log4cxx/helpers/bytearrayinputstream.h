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

#ifndef _LOG4CXX_HELPERS_BYTEARRAYINPUTSTREAM_H
#define _LOG4CXX_HELPERS_BYTEARRAYINPUTSTREAM_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <vector>
#include <log4cxx/helpers/inputstream.h>


namespace log4cxx
{

        namespace helpers {

          /**
           * InputStream implemented on top of a byte array.
           */
          class LOG4CXX_EXPORT ByteArrayInputStream : public InputStream
          {
          private:
              LOG4CXX_LIST_DEF(ByteList, unsigned char);
              ByteList buf;
              size_t pos;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(ByteArrayInputStream)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(ByteArrayInputStream)
                          LOG4CXX_CAST_ENTRY_CHAIN(InputStream)
                  END_LOG4CXX_CAST_MAP()

                  /**
                   * Creates a ByteArrayInputStream.
                   *
                   * @param bytes array of bytes to copy into stream.
                   */
                   ByteArrayInputStream(const ByteList& bytes);

                   virtual ~ByteArrayInputStream();

                  /**
                   * Closes this file input stream and releases any system 
                   * resources associated with the stream.
                   */
                  virtual void close();

                  /**
                   * Reads a sequence of bytes into the given buffer.
                   *
                   * @param buf The buffer into which bytes are to be transferred.
                   * @return the total number of bytes read into the buffer, or -1 if there
                   *         is no more data because the end of the stream has been reached.
                   */
                  virtual int read(ByteBuffer& buf);

          private:

                  ByteArrayInputStream(const ByteArrayInputStream&);

                  ByteArrayInputStream& operator=(const ByteArrayInputStream&);

          };

          LOG4CXX_PTR_DEF(ByteArrayInputStream);
        } // namespace helpers

}  //namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif //_LOG4CXX_HELPERS_BYTEARRAYINPUTSTREAM_H
