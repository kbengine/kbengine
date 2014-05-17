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
#include <log4cxx/helpers/fileinputstream.h>
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

IMPLEMENT_LOG4CXX_OBJECT(FileInputStream)

FileInputStream::FileInputStream(const LogString& filename) : fileptr(0) {
    open(filename);
}

FileInputStream::FileInputStream(const logchar* filename) : fileptr(0) {
    LogString fn(filename);
    open(fn);
}


void FileInputStream::open(const LogString& filename) {
    apr_fileperms_t perm = APR_OS_DEFAULT;
    apr_int32_t flags = APR_READ;
    apr_status_t stat = File().setPath(filename).open(&fileptr, flags, perm, pool);
    if (stat != APR_SUCCESS) {
      throw IOException(stat);
    }
}


FileInputStream::FileInputStream(const File& aFile) {
    apr_fileperms_t perm = APR_OS_DEFAULT;
    apr_int32_t flags = APR_READ;
    apr_status_t stat = aFile.open(&fileptr, flags, perm, pool);
    if (stat != APR_SUCCESS) {
      throw IOException(stat);
    }
}


FileInputStream::~FileInputStream() {
  if (fileptr != NULL && !APRInitializer::isDestructed) {
    apr_file_close(fileptr);
  }
}


void FileInputStream::close() {
  apr_status_t stat = apr_file_close(fileptr);
  if (stat == APR_SUCCESS) {
    fileptr = NULL;
  } else {
    throw IOException(stat);
  }
}


int FileInputStream::read(ByteBuffer& buf) {
  apr_size_t bytesRead = buf.remaining();
  apr_status_t stat = apr_file_read(fileptr, buf.current(), &bytesRead);
  int retval = -1; 
  if (!APR_STATUS_IS_EOF(stat)) {
    if (stat != APR_SUCCESS) {
      throw IOException(stat);
    }
    buf.position(buf.position() + bytesRead);
    retval = bytesRead;
  }

  return retval;
}
