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
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <apr_file_io.h>
#include <log4cxx/helpers/transcoder.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(FileOutputStream)

FileOutputStream::FileOutputStream(const LogString& filename,
    bool append) : pool(), fileptr(open(filename, append, pool)) {
}

FileOutputStream::FileOutputStream(const logchar* filename,
    bool append) : pool(), fileptr(open(filename, append, pool)) {
}

apr_file_t* FileOutputStream::open(const LogString& filename,
    bool append, Pool& pool) {
    apr_fileperms_t perm = APR_OS_DEFAULT;
    apr_int32_t flags = APR_WRITE | APR_CREATE;
    if (append) {
        flags |= APR_APPEND;
    } else {
        flags |= APR_TRUNCATE;
    }
    File fn;
    fn.setPath(filename);
    apr_file_t* fileptr = 0;
    apr_status_t stat = fn.open(&fileptr, flags, perm, pool);
    if (stat != APR_SUCCESS) {
      throw IOException(stat);
    }
    return fileptr;
}

FileOutputStream::~FileOutputStream() {
  if (fileptr != NULL && !APRInitializer::isDestructed) {
    apr_file_close(fileptr);
  }
}

void FileOutputStream::close(Pool& /* p */) {
  if (fileptr != NULL) {
    apr_status_t stat = apr_file_close(fileptr);
    if (stat != APR_SUCCESS) {
        throw IOException(stat);
    }
    fileptr = NULL;
  }
}

void FileOutputStream::flush(Pool& /* p */) {
}

void FileOutputStream::write(ByteBuffer& buf, Pool& /* p */ ) {
  if (fileptr == NULL) {
     throw IOException(-1);
  }
  size_t nbytes = buf.remaining();
  size_t pos = buf.position();
  const char* data = buf.data();
  while(nbytes > 0) {
    apr_status_t stat = apr_file_write(
      fileptr, data + pos, &nbytes);
    if (stat != APR_SUCCESS) {
      throw IOException(stat);
    }
    pos += nbytes;
    buf.position(pos);
    nbytes = buf.remaining();
  }
}

