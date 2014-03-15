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
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/charsetencoder.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/stringhelper.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(OutputStreamWriter)

OutputStreamWriter::OutputStreamWriter(OutputStreamPtr& out1)
   : out(out1), enc(CharsetEncoder::getDefaultEncoder()) {
   if (out1 == 0) {
      throw NullPointerException(LOG4CXX_STR("out parameter may not be null."));
   }
}

OutputStreamWriter::OutputStreamWriter(OutputStreamPtr& out1,
     CharsetEncoderPtr &enc1)
    : out(out1), enc(enc1) {
    if (out1 == 0) {
       throw NullPointerException(LOG4CXX_STR("out parameter may not be null."));
    }
    if (enc1 == 0) {
       throw NullPointerException(LOG4CXX_STR("enc parameter may not be null."));
    }
}

OutputStreamWriter::~OutputStreamWriter() {
}

void OutputStreamWriter::close(Pool& p) {
  out->close(p);
}

void OutputStreamWriter::flush(Pool& p) {
  out->flush(p);
}

void OutputStreamWriter::write(const LogString& str, Pool& p) {
  if (str.length() > 0) {
    enum { BUFSIZE = 1024 };
    char rawbuf[BUFSIZE];
    ByteBuffer buf(rawbuf, (size_t) BUFSIZE);
    enc->reset();
    LogString::const_iterator iter = str.begin();
    while(iter != str.end()) {
      CharsetEncoder::encode(enc, str, iter, buf);
      buf.flip();
      out->write(buf, p);
      buf.clear();
    }
    CharsetEncoder::encode(enc, str, iter, buf);
    enc->flush(buf);
    buf.flip();
    out->write(buf, p);
  }
}

