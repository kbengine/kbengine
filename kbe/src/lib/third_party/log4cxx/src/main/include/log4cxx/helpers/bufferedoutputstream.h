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

#ifndef _LOG4CXX_HELPERS_BUFFEREDOUTPUTSTREAM_H
#define _LOG4CXX_HELPERS_BUFFEREDOUTPUTSTREAM_H

#include <log4cxx/helpers/outputstream.h>

namespace log4cxx
{

        namespace helpers {

          /**
          *   Abstract class for writing to character streams.
          */
          class LOG4CXX_EXPORT BufferedOutputStream : public OutputStream
          {
          private:
                  size_t count;
                  LogString buf;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(BufferedOutputStream)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(BufferedOutputStream)
                          LOG4CXX_CAST_ENTRY_CHAIN(OutputStream)
                  END_LOG4CXX_CAST_MAP()

          protected:
                  BufferedOutputStream(OutputStreamPtr& out, size_t size = 4096);
                  ~BufferedOutputStream();

          public:
                  void close(Pool& p);
                  void flush(Pool& p);
                  void write(ByteBuffer& buf, Pool& p);

          private:
                  BufferedOutputStream(const BufferedOutputStream&);
                  BufferedOutputStream& operator=(const BufferedOutputStream&);
          };

          LOG4CXX_PTR_DEF(BufferedOutputStream);
        } // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_BUFFEREDOUTPUTSTREAM_H
