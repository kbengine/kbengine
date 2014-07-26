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
#include <log4cxx/fileappender.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/bufferedwriter.h>
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/synchronized.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

IMPLEMENT_LOG4CXX_OBJECT(FileAppender)


FileAppender::FileAppender() {
    synchronized sync(mutex);
    fileAppend = true;
    bufferedIO = false;
    bufferSize = 8 * 1024;
}

FileAppender::FileAppender(const LayoutPtr& layout1, const LogString& fileName1,
        bool append1, bool bufferedIO1, int bufferSize1) 
           : WriterAppender(layout1) {
        {  
            synchronized sync(mutex);
            fileAppend = append1;
            fileName = fileName1;
            bufferedIO = bufferedIO1;
            bufferSize = bufferSize1;
         }
        Pool p;
        activateOptions(p);
}

FileAppender::FileAppender(const LayoutPtr& layout1, const LogString& fileName1,
        bool append1)
: WriterAppender(layout1) {
        {
            synchronized sync(mutex);
            fileAppend = append1;
            fileName = fileName1;
            bufferedIO = false;
            bufferSize = 8 * 1024;
         }
        Pool p;
        activateOptions(p);
}

FileAppender::FileAppender(const LayoutPtr& layout1, const LogString& fileName1)
: WriterAppender(layout1) {
        {
            synchronized sync(mutex);
            fileAppend = true;
            fileName = fileName1;
            bufferedIO = false;
            bufferSize = 8 * 1024;
        }
        Pool p;
        activateOptions(p);
}

FileAppender::~FileAppender()
{
    finalize();
}

void FileAppender::setAppend(bool fileAppend1) { 
    synchronized sync(mutex);
    this->fileAppend = fileAppend1; 
}

void FileAppender::setFile(const LogString& file)
{
        synchronized sync(mutex);
        fileName = file;
}



void FileAppender::setBufferedIO(bool bufferedIO1)
{
        synchronized sync(mutex);
        this->bufferedIO = bufferedIO1;
        if(bufferedIO1)
        {
                setImmediateFlush(false);
        }
}

void FileAppender::setOption(const LogString& option,
        const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FILE"), LOG4CXX_STR("file"))
                || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("FILENAME"), LOG4CXX_STR("filename")))
        {
                synchronized sync(mutex);
                fileName = stripDuplicateBackslashes(value);
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("APPEND"), LOG4CXX_STR("append")))
        {
                synchronized sync(mutex);
                fileAppend = OptionConverter::toBoolean(value, true);
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFEREDIO"), LOG4CXX_STR("bufferedio")))
        {
                synchronized sync(mutex);
                bufferedIO = OptionConverter::toBoolean(value, true);
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("IMMEDIATEFLUSH"), LOG4CXX_STR("immediateflush")))
        {
                synchronized sync(mutex);
                bufferedIO = !OptionConverter::toBoolean(value, false);
        }
        else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize")))
        {
                synchronized sync(mutex);
                bufferSize = OptionConverter::toFileSize(value, 8*1024);
        }
        else
        {
                WriterAppender::setOption(option, value);
        }
}

void FileAppender::activateOptions(Pool& p)
{
  synchronized sync(mutex);
  int errors = 0;
  if (!fileName.empty()) {
    try {
      setFile(fileName, fileAppend, bufferedIO, bufferSize, p);
    } catch (IOException& e) {
      errors++;
      LogString msg(LOG4CXX_STR("setFile("));
      msg.append(fileName);
      msg.append(1, (logchar) 0x2C /* ',' */);
      StringHelper::toString(fileAppend, msg);
      msg.append(LOG4CXX_STR(") call failed."));
      LogLog::error(msg, e);
    }
  } else {
    errors++;
    LogLog::error(LogString(LOG4CXX_STR("File option not set for appender ["))
       +  name + LOG4CXX_STR("]."));
    LogLog::warn(LOG4CXX_STR("Are you using FileAppender instead of ConsoleAppender?"));
  }
  if(errors == 0) {
    WriterAppender::activateOptions(p);
  }
}


/**
 * Replaces double backslashes (except the leading doubles of UNC's)
 * with single backslashes for compatibility with existing path
 * specifications that were working around use of
 * OptionConverter::convertSpecialChars in XML configuration files.
 *
 * @param src source string
 * @return modified string
 * 
 *
 */
LogString FileAppender::stripDuplicateBackslashes(const LogString& src) {
    logchar backslash = 0x5C; // '\\'
    LogString::size_type i = src.find_last_of(backslash);
    if (i != LogString::npos) {
        LogString tmp(src);
        for(;
            i != LogString::npos && i > 0;
            i = tmp.find_last_of(backslash, i - 1)) {
            //
            //   if the preceding character is a slash then
            //      remove the preceding character
            //      and continue processing
            if (tmp[i - 1] == backslash) {
                tmp.erase(i, 1);
                i--;
                if (i == 0) break;
            } else {
                //
                //  if there an odd number of slashes
                //     the string wasn't trying to work around
                //     OptionConverter::convertSpecialChars
                return src;
            }
       }
       return tmp;
    }
    return src;
}

/**
  <p>Sets and <i>opens</i> the file where the log output will
  go. The specified file must be writable.

  <p>If there was already an opened file, then the previous file
  is closed first.

  <p><b>Do not use this method directly. To configure a FileAppender
  or one of its subclasses, set its properties one by one and then
  call activateOptions.</b>

  @param filename The path to the log file.
  @param append   If true will append to fileName. Otherwise will
      truncate fileName.
  @param bufferedIO
  @param bufferSize

  @throws IOException

 */
void FileAppender::setFile(
  const LogString& filename,
      bool append1,
      bool bufferedIO1,
      size_t bufferSize1,
      Pool& p) {
  synchronized sync(mutex);

  // It does not make sense to have immediate flush and bufferedIO.
  if (bufferedIO1) {
    setImmediateFlush(false);
  }

  closeWriter();

  bool writeBOM = false;
  if(StringHelper::equalsIgnoreCase(getEncoding(),
      LOG4CXX_STR("utf-16"), LOG4CXX_STR("UTF-16"))) {
      //
      //    don't want to write a byte order mark if the file exists
      //
      if (append1) {
        File outFile;
        outFile.setPath(filename);
        writeBOM = !outFile.exists(p);
      } else {
        writeBOM = true;
      }
  }

  OutputStreamPtr outStream;
  try {
      outStream = new FileOutputStream(filename, append1);
  } catch(IOException& ex) {
      LogString parentName = File().setPath(filename).getParent(p);
      if (!parentName.empty()) {
          File parentDir;
          parentDir.setPath(parentName);
          if(!parentDir.exists(p) && parentDir.mkdirs(p)) {
             outStream = new FileOutputStream(filename, append1);
          } else {
             throw ex;
          }
      } else {
        throw ex;
      }
  }
  
  
  //
  //   if a new file and UTF-16, then write a BOM
  //
  if (writeBOM) {
      char bom[] = { (char) 0xFE, (char) 0xFF };
      ByteBuffer buf(bom, 2);
      outStream->write(buf, p);
  }

  WriterPtr newWriter(createWriter(outStream));

  if (bufferedIO1) {
    newWriter = new BufferedWriter(newWriter, bufferSize1);
  }
  setWriter(newWriter);

  this->fileAppend = append1;
  this->bufferedIO = bufferedIO1;
  this->fileName = filename;
  this->bufferSize = bufferSize1;
  writeHeader(p);

}

