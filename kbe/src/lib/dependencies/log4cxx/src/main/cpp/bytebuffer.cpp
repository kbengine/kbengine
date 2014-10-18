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
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/exception.h>
#include <apr_pools.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

ByteBuffer::ByteBuffer(char* data1, size_t capacity)
   : base(data1), pos(0), lim(capacity), cap(capacity) {
}

ByteBuffer::~ByteBuffer() {
}

void ByteBuffer::clear() {
  lim = cap;
  pos = 0;
}

void ByteBuffer::flip() {
  lim = pos;
  pos = 0;
}

void ByteBuffer::position(size_t newPosition) {
  if (newPosition < lim) {
    pos = newPosition;
  } else {
    pos = lim;
  }
}

void ByteBuffer::limit(size_t newLimit) {
  if (newLimit > cap) {
    throw IllegalArgumentException(LOG4CXX_STR("newLimit"));
  }
  lim = newLimit;
}


bool ByteBuffer::put(char byte) {
  if (pos < lim) {
    base[pos++] = byte;
    return true;
  }
  return false;
}
