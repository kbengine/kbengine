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

#ifndef _LOG4CXX_FILE_APPENDER_H
#define _LOG4CXX_FILE_APPENDER_H

#include <log4cxx/logger.h>
#include <log4cxx/logstring.h>
#include <log4cxx/writerappender.h>
#include <log4cxx/file.h>
#include <log4cxx/helpers/pool.h>

namespace log4cxx
{
        namespace helpers {
            class Pool;
        }

        /**
        *  FileAppender appends log events to a file.
        *
        *  <p>Support for <code>java.io.Writer</code> and console appending
        *  has been deprecated and then removed. See the replacement
        *  solutions: WriterAppender and ConsoleAppender.
        */
        class LOG4CXX_EXPORT FileAppender : public WriterAppender
        {
        protected:
                /** Append to or truncate the file? The default value for this
                variable is <code>true</code>, meaning that by default a
                <code>FileAppender</code> will append to an existing file and
                not truncate it.
                <p>This option is meaningful only if the FileAppender opens the
                file.
                */
                bool fileAppend;

                /**
                The name of the log file. */
                LogString fileName;

                /**
                Do we do bufferedIO? */
                bool bufferedIO;

                /**
                How big should the IO buffer be? Default is 8K. */
                int bufferSize;

        public:
                DECLARE_LOG4CXX_OBJECT(FileAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(FileAppender)
                        LOG4CXX_CAST_ENTRY_CHAIN(WriterAppender)
                END_LOG4CXX_CAST_MAP()

                /**
                The default constructor does not do anything.
                */
                FileAppender();

                /**
                Instantiate a <code>FileAppender</code> and open the file
                designated by <code>filename</code>. The opened filename will
                become the output destination for this appender.

                <p>If the <code>append</code> parameter is true, the file will be
                appended to. Otherwise, the file designated by
                <code>filename</code> will be truncated before being opened.

                <p>If the <code>bufferedIO</code> parameter is <code>true</code>,
                then buffered IO will be used to write to the output file.

                */
                FileAppender(const LayoutPtr& layout, const LogString& filename, bool append,
                        bool bufferedIO, int bufferSize);

                /**
                Instantiate a FileAppender and open the file designated by
                <code>filename</code>. The opened filename will become the output
                destination for this appender.

                <p>If the <code>append</code> parameter is true, the file will be
                appended to. Otherwise, the file designated by
                <code>filename</code> will be truncated before being opened.
                */
                FileAppender(const LayoutPtr& layout, const LogString& filename, bool append);

                /**
                Instantiate a FileAppender and open the file designated by
                <code>filename</code>. The opened filename will become the output
                destination for this appender.

                <p>The file will be appended to.  */
                FileAppender(const LayoutPtr& layout, const LogString& filename);

                ~FileAppender();

                /**
                The <b>File</b> property takes a string value which should be the
                name of the file to append to.

                <p><b>Note that the special values
                "System.out" or "System.err" are no longer honored.</b>

                <p>Note: Actual opening of the file is made when
                #activateOptions is called, not when the options are set.  */
                virtual void setFile(const LogString& file);

                /**
                Sets and <i>opens</i> the file where the log output will
                go. The specified file must be writable.

                <p>If there was already an opened file, then the previous file
                is closed first.

                <p><b>Do not use this method directly. To configure a FileAppender
                or one of its subclasses, set its properties one by one and then
                call activateOptions.</b>

                @param file The path to the log file.
                @param append If true will append to fileName. Otherwise will
                truncate fileName.
                @param bufferedIO Do we do bufferedIO?
                @param bufferSize How big should the IO buffer be?
                @param p memory pool for operation.
                */
                virtual void setFile(const LogString& file, bool append,
                        bool bufferedIO, size_t bufferSize,
                        log4cxx::helpers::Pool& p);

                /**
                Returns the value of the <b>Append</b> option.
                */
                inline bool getAppend() const { return fileAppend; }

                /** Returns the value of the <b>File</b> option. */
                inline LogString getFile() const { return fileName; }

                /**
                <p>Sets and <i>opens</i> the file where the log output will
                go. The specified file must be writable.

                <p>If there was already an opened file, then the previous file
                is closed first.*/
                void activateOptions(log4cxx::helpers::Pool& p);
                void setOption(const LogString& option,
                        const LogString& value);

                /**
                Get the value of the <b>BufferedIO</b> option.

                <p>BufferedIO will significatnly increase performance on heavily
                loaded systems.

                */
                inline bool getBufferedIO() const { return bufferedIO; }

                /**
                Get the size of the IO buffer.
                */
               inline  int getBufferSize() const { return bufferSize; }

                /**
                The <b>Append</b> option takes a boolean value. It is set to
                <code>true</code> by default. If true, then <code>File</code>
                will be opened in append mode by #setFile (see
                above). Otherwise, setFile will open
                <code>File</code> in truncate mode.

                <p>Note: Actual opening of the file is made when
                #activateOptions is called, not when the options are set.
                */
                void setAppend(bool fileAppend1);

                /**
                The <b>BufferedIO</b> option takes a boolean value. It is set to
                <code>false</code> by default. If true, then <code>File</code>
                will be opened in buffered mode.

                BufferedIO will significantly increase performance on heavily
                loaded systems.

                */
                void setBufferedIO(bool bufferedIO);

                /**
                Set the size of the IO buffer.
                */
                void setBufferSize(int bufferSize1) { this->bufferSize = bufferSize1; }

                /**
                 *   Replaces double backslashes with single backslashes
                 *   for compatibility with paths from earlier XML configurations files.
                 *   @param name file name
                 *   @return corrected file name
                 */
                static LogString stripDuplicateBackslashes(const LogString& name);

                private:
                FileAppender(const FileAppender&);
                FileAppender& operator=(const FileAppender&);

        }; // class FileAppender
        LOG4CXX_PTR_DEF(FileAppender);

}  // namespace log4cxx

#endif
