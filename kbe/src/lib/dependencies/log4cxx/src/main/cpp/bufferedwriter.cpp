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
#include <log4cxx/helpers/bufferedwriter.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(BufferedWriter)

BufferedWriter::BufferedWriter(WriterPtr& out1)
    : out(out1), sz(1024) {
}

BufferedWriter::BufferedWriter(WriterPtr& out1, size_t sz1)
    : out(out1), sz(sz1) {
}

BufferedWriter::~BufferedWriter() {
}

void BufferedWriter::close(Pool& p) {
   flush(p);
   out->close(p);
}

void BufferedWriter::flush(Pool& p) {
  if (buf.length() > 0) {
     out->write(buf, p);
     buf.erase(buf.begin(), buf.end());
  }
}

void BufferedWriter::write(const LogString& str, Pool& p) {
  if (buf.length() + str.length() > sz) {
    out->write(buf, p);
    buf.erase(buf.begin(), buf.end());
  }
  if (str.length() > sz) {
    out->write(str, p);
  } else {
    buf.append(str);
  }
}

