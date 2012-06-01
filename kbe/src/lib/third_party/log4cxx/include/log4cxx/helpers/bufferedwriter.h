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

#ifndef _LOG4CXX_HELPERS_BUFFEREDWRITER_H
#define _LOG4CXX_HELPERS_BUFFEREDWRITER_H

#include <log4cxx/helpers/writer.h>

namespace log4cxx
{

        namespace helpers {

          /**
          *   Writes text to a character-output stream buffering
          *       requests to increase efficiency.
          */
          class LOG4CXX_EXPORT BufferedWriter : public Writer
          {
          private:
                  WriterPtr out;
                  size_t sz;
                  LogString buf;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(BufferedWriter)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(BufferedWriter)
                          LOG4CXX_CAST_ENTRY_CHAIN(Writer)
                  END_LOG4CXX_CAST_MAP()

                  BufferedWriter(WriterPtr& out);
                  BufferedWriter(WriterPtr& out, size_t sz);
                  virtual ~BufferedWriter();

                  virtual void close(Pool& p);
                  virtual void flush(Pool& p);
                  virtual void write(const LogString& str, Pool& p);

          private:
                  BufferedWriter(const BufferedWriter&);
                  BufferedWriter& operator=(const BufferedWriter&);
          };

        } // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_BUFFEREDWRITER_H
