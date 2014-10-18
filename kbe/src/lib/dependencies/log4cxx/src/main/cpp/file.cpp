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
#include <log4cxx/file.h>
#include <apr_file_io.h>
#include <apr_file_info.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include <assert.h>
#include <log4cxx/helpers/exception.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

File::File() {
}

template<class S> 
static LogString decodeLS(const S* src) {
    LogString dst;
    if (src != 0) {
      Transcoder::decode(src, dst);
    }
    return dst;
}

template<class S> 
static LogString decodeLS(const std::basic_string<S>& src) {
    LogString dst;
    Transcoder::decode(src, dst);
    return dst;
}


File::File(const std::string& name1)
  : path(decodeLS(name1)) {
}

File::File(const char* name1)
  : path(decodeLS(name1)) {
}

#if LOG4CXX_WCHAR_T_API
File::File(const std::wstring& name1)
   : path(decodeLS(name1)) {
}

File::File(const wchar_t* name1)
   : path(decodeLS(name1)) {
}
#endif

#if LOG4CXX_UNICHAR_API
File::File(const std::basic_string<UniChar>& name1)
   : path(decodeLS(name1)) {
}

File::File(const UniChar* name1)
   : path(decodeLS(name1)) {
}
#endif

#if LOG4CXX_CFSTRING_API
File::File(const CFStringRef& name1)
   : path(decodeLS(name1)) {
}
#endif

File::File(const File& src)
  : path(src.path) {
}

File& File::operator=(const File& src) {
  if (this == &src) return *this;

  path.assign(src.path);

  return *this;
}


File::~File() {
}


LogString File::getPath() const {
    return path;
}

File& File::setPath(const LogString& newName) {
    path.assign(newName);
    return *this;
}

LogString File::getName() const {
    const logchar slashes[] = { 0x2F, 0x5C, 0 };
    size_t lastSlash = path.find_last_of(slashes);
    if (lastSlash != LogString::npos) {
        return path.substr(lastSlash+1);
    }
    return path;
}

char* File::getPath(Pool& p) const {
    int style = APR_FILEPATH_ENCODING_UNKNOWN;
    apr_filepath_encoding(&style, p.getAPRPool());
    char* retval = NULL;
    if (style == APR_FILEPATH_ENCODING_UTF8) {
        retval = Transcoder::encodeUTF8(path, p);
    } else {
        retval = Transcoder::encode(path, p);
    }
    return retval;
}

log4cxx_status_t File::open(apr_file_t** file, int flags,
      int perm, Pool& p) const {
    return apr_file_open(file, getPath(p), flags, perm, p.getAPRPool());
}



bool File::exists(Pool& p) const {
  apr_finfo_t finfo;
  apr_status_t rv = apr_stat(&finfo, getPath(p),
        0, p.getAPRPool());
  return rv == APR_SUCCESS;
}

char* File::convertBackSlashes(char* src) {
  for(char* c = src; *c != 0; c++) {
   if(*c == '\\') {
      *c = '/';
   }
  }
  return src;
}

bool File::deleteFile(Pool& p) const {
  apr_status_t rv = apr_file_remove(convertBackSlashes(getPath(p)),
        p.getAPRPool());
  return rv == APR_SUCCESS;
}

bool File::renameTo(const File& dest, Pool& p) const {
  apr_status_t rv = apr_file_rename(convertBackSlashes(getPath(p)),
        convertBackSlashes(dest.getPath(p)),
        p.getAPRPool());
  return rv == APR_SUCCESS;
}


size_t File::length(Pool& pool) const {
  apr_finfo_t finfo;
  apr_status_t rv = apr_stat(&finfo, getPath(pool),
        APR_FINFO_SIZE, pool.getAPRPool());
  if (rv == APR_SUCCESS) {
    return (size_t) finfo.size;
  }
  return 0;
}


log4cxx_time_t File::lastModified(Pool& pool) const {
  apr_finfo_t finfo;
  apr_status_t rv = apr_stat(&finfo, getPath(pool),
        APR_FINFO_MTIME, pool.getAPRPool());
  if (rv == APR_SUCCESS) {
    return finfo.mtime;
  }
  return 0;
}


std::vector<LogString> File::list(Pool& p) const {
    apr_dir_t *dir;
    apr_finfo_t entry;
    std::vector<LogString> filenames;

    apr_status_t stat = apr_dir_open(&dir, 
        convertBackSlashes(getPath(p)), 
        p.getAPRPool());
    if(stat == APR_SUCCESS) {
        int style = APR_FILEPATH_ENCODING_UNKNOWN;
        apr_filepath_encoding(&style, p.getAPRPool());
        stat = apr_dir_read(&entry, APR_FINFO_DIRENT, dir);
        while(stat == APR_SUCCESS) {
            if (entry.name != NULL) {
               LogString filename;
               if(style == APR_FILEPATH_ENCODING_UTF8) {
                 Transcoder::decodeUTF8(entry.name, filename);
               } else {
                   Transcoder::decode(entry.name, filename);
               }
               filenames.push_back(filename);
            }
            stat = apr_dir_read(&entry, APR_FINFO_DIRENT, dir);
        }
        stat = apr_dir_close(dir);
    }
    return filenames;
}

LogString File::getParent(Pool&) const {
     LogString::size_type slashPos = path.rfind(LOG4CXX_STR('/'));
     LogString::size_type backPos = path.rfind(LOG4CXX_STR('\\'));
     if (slashPos == LogString::npos) {
         slashPos = backPos;
     } else {
         if (backPos != LogString::npos && backPos > slashPos) {
             slashPos = backPos;
         }
     }
     LogString parent;
     if (slashPos != LogString::npos && slashPos > 0) {
          parent.assign(path, 0, slashPos);
     }
     return parent;
}

bool File::mkdirs(Pool& p) const {
     apr_status_t stat = apr_dir_make_recursive(convertBackSlashes(getPath(p)),
          APR_OS_DEFAULT, p.getAPRPool());
     return stat == APR_SUCCESS;
}
