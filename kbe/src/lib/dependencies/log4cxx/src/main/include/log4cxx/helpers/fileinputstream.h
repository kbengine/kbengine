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

#ifndef _LOG4CXX_HELPERS_FILEINPUTSTREAM_H
#define _LOG4CXX_HELPERS_FILEINPUTSTREAM_H

#include <log4cxx/helpers/inputstream.h>
#include <log4cxx/file.h>
#include <log4cxx/helpers/pool.h>


namespace log4cxx
{

        namespace helpers {

          /**
           * InputStream implemented on top of APR file IO.
           * 
           */
          class LOG4CXX_EXPORT FileInputStream : public InputStream
          {
          private:
                  Pool pool;
                  apr_file_t* fileptr;

          public:
                  DECLARE_ABSTRACT_LOG4CXX_OBJECT(FileInputStream)
                  BEGIN_LOG4CXX_CAST_MAP()
                          LOG4CXX_CAST_ENTRY(FileInputStream)
                          LOG4CXX_CAST_ENTRY_CHAIN(InputStream)
                  END_LOG4CXX_CAST_MAP()

                  /**
                   * Creates a FileInputStream by opening a connection to an actual
                   * file, the file named by the path name name in the file system.
                   *
                   * @param filename The system-dependent file name.
                   */
                  FileInputStream(const LogString& filename);
                  FileInputStream(const logchar* filename);

                  /**
                   * Creates a FileInputStream by opening a connection to an actual 
                   * file, the file named by the File object file in the file system.
                   *
                   * @param aFile The file to be opened for reading.
                   */
                  FileInputStream(const File& aFile);

                  virtual ~FileInputStream();

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

                  FileInputStream(const FileInputStream&);

                  FileInputStream& operator=(const FileInputStream&);
                  void open(const LogString&);

          };

          LOG4CXX_PTR_DEF(FileInputStream);
        } // namespace helpers

}  //namespace log4cxx

#endif //_LOG4CXX_HELPERS_FILEINPUTSTREAM_H
