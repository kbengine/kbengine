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

#ifndef _LOG4CXX_FILE_H
#define _LOG4CXX_FILE_H

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>

extern "C" {
struct apr_file_t;
struct apr_finfo_t;
}

namespace log4cxx
{
                namespace helpers {
                  class Transcoder;
                  class Pool;
                }

                /**
                * An abstract representation of file and directory path names.
                */
                class LOG4CXX_EXPORT File
                {
                public:
                    /**
                    *   Construct a new instance.
                    */
                    File();
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path in local encoding.
                    */
                    File(const char* path);
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path in current encoding.
                    */
                    File(const std::string& path);
#if LOG4CXX_WCHAR_T_API
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path.
                    */
                    File(const wchar_t* path);
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path.
                    */
                    File(const std::wstring& path);
#endif
#if LOG4CXX_UNICHAR_API
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path.
                    */
                    File(const UniChar* path);
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path.
                    */
                    File(const std::basic_string<UniChar>& path);
#endif
#if LOG4CXX_CFSTRING_API
                    /**
                    *   Construct a new instance.  Use setPath to specify path using a LogString.
                    * @param path file path.
                    */
                    File(const CFStringRef& path);
#endif
                    /**
                     *  Copy constructor.
                     */
                    File(const File& src);
                    /**
                     *  Assignment operator.
                     */ 
                    File& operator=(const File& src);
                    /**
                     *  Destructor.
                     */
                    ~File();
                    
                    /**
                     *  Determines if file exists.
                     *  @param p pool.
                     *  @return true if file exists.
                     */
                    bool exists(log4cxx::helpers::Pool& p) const;
                    /**
                     *  Determines length of file.  May not be accurate if file is current open.
                     *  @param p pool.
                     *  @return length of file.
                     */
                    size_t length(log4cxx::helpers::Pool& p) const;
                    /**
                     *  Determines last modification date.
                     *  @param p pool.
                     *  @return length of file.
                     */
                    log4cxx_time_t lastModified(log4cxx::helpers::Pool& p) const;
                    /**
                     *  Get final portion of file path.
                     *  @return file name.
                     */
                    LogString getName() const;
                    /**
                     *  Get file path.
                     *  @return file path.
                     */
                    LogString getPath() const;
                    /**
                     *  Set file path
                     */
                    File& setPath(const LogString&);

                    /**
                     *  Open file.  See apr_file_open for details.
                     *  @param file APR file handle.
                     *  @param flags flags.
                     *  @param perm permissions.
                     *  @param p pool.
                     *  @return APR_SUCCESS if successful.
                     */
                    log4cxx_status_t open(apr_file_t** file, int flags,
                          int perm, log4cxx::helpers::Pool& p) const;

                    /**
                     *   List files if current file is a directory.
                     *   @param p pool.
                     *   @return list of files in this directory, operation of non-directory returns empty list.
                     */
                    std::vector<LogString> list(log4cxx::helpers::Pool& p) const;

                    /**
                     *   Delete file.
                     *   @param p pool.
                     *   @return true if file successfully deleted.
                     */
                    bool deleteFile(log4cxx::helpers::Pool& p) const;
                    /**
                     *   Rename file.
                     *   @param dest new path for file.
                     *   @param p pool.
                     *   @return true if file successfully renamed.
                     */
                    bool renameTo(const File& dest, log4cxx::helpers::Pool& p) const;
                    
                    /**
                     *   Get path of parent directory.
                     *   @param p pool.
                     *   @return path of parent directory.
                     */
                    LogString getParent(log4cxx::helpers::Pool& p) const;
                    /**
                     *  Make directories recursively.
                     *  @param p pool.
                     *  @return true if all requested directories existed or have been created.
                     */
                    bool mkdirs(log4cxx::helpers::Pool& p) const;

                private:
                    LogString path;
                    static char* convertBackSlashes(char*);
                    char* getPath(log4cxx::helpers::Pool& p) const;
                };
} // namespace log4cxx


#define LOG4CXX_FILE(name) log4cxx::File(name)

#endif // _LOG4CXX_FILE_H
