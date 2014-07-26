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

#ifndef _LOG4CXX_WRITER_APPENDER_H
#define _LOG4CXX_WRITER_APPENDER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/outputstreamwriter.h>

namespace log4cxx
{

        namespace helpers {
          class Transcoder;
        }

        /**
        WriterAppender appends log events to a standard output stream
        */
        class LOG4CXX_EXPORT WriterAppender : public AppenderSkeleton
        {
        private:
                /**
                Immediate flush means that the underlying writer or output stream
                will be flushed at the end of each append operation. Immediate
                flush is slower but ensures that each append request is actually
                written. If <code>immediateFlush</code> is set to
                <code>false</code>, then there is a good chance that the last few
                logs events are not actually written to persistent media if and
                when the application crashes.

                <p>The <code>immediateFlush</code> variable is set to
                <code>true</code> by default.

                */
                bool immediateFlush;

                /**
                The encoding to use when opening an input stream.
                <p>The <code>encoding</code> variable is set to <code>""</code> by
                default which results in the utilization of the system's default
                encoding.  */
                LogString encoding;

                /**
                *  This is the {@link Writer Writer} where we will write to.
                */
                log4cxx::helpers::WriterPtr writer;


        public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(WriterAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(WriterAppender)
                        LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                END_LOG4CXX_CAST_MAP()

                /**
                This default constructor does nothing.*/
                WriterAppender();
        protected:
                WriterAppender(const LayoutPtr& layout,
                              log4cxx::helpers::WriterPtr& writer);
                WriterAppender(const LayoutPtr& layout);

        public:
                ~WriterAppender();

                /**
                Derived appenders should override this method if option structure
                requires it.
                */
                virtual void activateOptions(log4cxx::helpers::Pool& pool);

                /**
                If the <b>ImmediateFlush</b> option is set to
                <code>true</code>, the appender will flush at the end of each
                write. This is the default behavior. If the option is set to
                <code>false</code>, then the underlying stream can defer writing
                to physical medium to a later time.

                <p>Avoiding the flush operation at the end of each append results in
                a performance gain of 10 to 20 percent. However, there is safety
                tradeoff involved in skipping flushing. Indeed, when flushing is
                skipped, then it is likely that the last few log events will not
                be recorded on disk when the application exits. This is a high
                price to pay even for a 20% performance gain.
                */
                void setImmediateFlush(bool value);
                /**
                Returns value of the <b>ImmediateFlush</b> option.
                */
                bool getImmediateFlush() const { return immediateFlush; }

                /**
                This method is called by the AppenderSkeleton#doAppend
                method.

                <p>If the output stream exists and is writable then write a log
                statement to the output stream. Otherwise, write a single warning
                message to <code>stderr</code>.

                <p>The format of the output will depend on this appender's
                layout.

                */
                virtual void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);


        protected:
                /**
                This method determines if there is a sense in attempting to append.

                <p>It checks whether there is a set output target and also if
                there is a set layout. If these checks fail, then the boolean
                value <code>false</code> is returned. */
                virtual bool checkEntryConditions() const;


        public:
                /**
                Close this appender instance. The underlying stream or writer is
                also closed.

                <p>Closed appenders cannot be reused.
                */
                virtual void close();

        protected:
                /**
                 * Close the underlying {@link log4cxx::helpers::Writer}.
                 * */
                void closeWriter();

                /**
                    Returns an OutputStreamWriter when passed an OutputStream.  The
                    encoding used will depend on the value of the
                    <code>encoding</code> property.  If the encoding value is
                    specified incorrectly the writer will be opened using the default
                    system encoding (an error message will be printed to the loglog.  */
                virtual log4cxx::helpers::WriterPtr createWriter(
                    log4cxx::helpers::OutputStreamPtr& os);

       public:
                LogString getEncoding() const;
                void setEncoding(const LogString& value);
                void setOption(const LogString& option,
                    const LogString& value);

                /**
                  <p>Sets the Writer where the log output will go. The
                  specified Writer must be opened by the user and be
                  writable.

                  <p>The <code>java.io.Writer</code> will be closed when the
                  appender instance is closed.


                  <p><b>WARNING:</b> Logging to an unopened Writer will fail.
                  <p>
                  @param writer An already opened Writer.  */
                void setWriter(const log4cxx::helpers::WriterPtr& writer);

                virtual bool requiresLayout() const;

        protected:
               /**
                Actual writing occurs here.
               */
               virtual void subAppend(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);


                /**
                Write a footer as produced by the embedded layout's
                Layout#appendFooter method.  */
                virtual void writeFooter(log4cxx::helpers::Pool& p);

                /**
                Write a header as produced by the embedded layout's
                Layout#appendHeader method.  */
                virtual void writeHeader(log4cxx::helpers::Pool& p);

        private:
                //
                //  prevent copy and assignment
                WriterAppender(const WriterAppender&);
                WriterAppender& operator=(const WriterAppender&);
        };

        LOG4CXX_PTR_DEF(WriterAppender);

}  //namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif //_LOG4CXX_WRITER_APPENDER_H
