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

#ifndef _LOG4CXX_TTCC_LAYOUT_H
#define _LOG4CXX_TTCC_LAYOUT_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/helpers/datelayout.h>

namespace log4cxx
{

    /**
    TTCC layout format consists of time, thread, logger name and nested
    diagnostic context information, hence the name.

    <p>Each of the four fields can be individually enabled or
    disabled. The time format depends on the <code>DateFormat</code>
    used.

    <p>Here is an example TTCCLayout output with the
    {@link helpers::RelativeTimeDateFormat RelativeTimeDateFormat}.

    <pre>
    176 [main] INFO  examples.Sort - Populating an array of 2 elements in reverse order.
    225 [main] INFO  examples.SortAlgo - Entered the sort method.
    262 [main] DEBUG examples.SortAlgo.OUTER i=1 - Outer loop.
    276 [main] DEBUG examples.SortAlgo.SWAP i=1 j=0 - Swapping intArray[0] = 1 and intArray[1] = 0
    290 [main] DEBUG examples.SortAlgo.OUTER i=0 - Outer loop.
    304 [main] INFO  examples.SortAlgo.DUMP - Dump of interger array:
    317 [main] INFO  examples.SortAlgo.DUMP - Element [0] = 0
    331 [main] INFO  examples.SortAlgo.DUMP - Element [1] = 1
    343 [main] INFO  examples.Sort - The next log statement should be an error message.
    346 [main] ERROR examples.SortAlgo.DUMP - Tried to dump an uninitialized array.
    467 [main] INFO  examples.Sort - Exiting main method.
    </pre>

    <p>The first field is the number of milliseconds elapsed since the
    start of the program. The second field is the thread outputting the
    log statement. The third field is the level, the fourth field is
    the logger to which the statement belongs.

    <p>The fifth field (just before the '-') is the nested diagnostic
    context.  Note the nested diagnostic context may be empty as in the
    first two statements. The text after the '-' is the message of the
    statement.

    <p><b>WARNING</b> Do not use the same TTCCLayout instance from
    within different appenders. The TTCCLayout is not thread safe when
    used in his way. However, it is perfectly safe to use a TTCCLayout
    instance from just one appender.

    <p>PatternLayout offers a much more flexible alternative.
    */
        class LOG4CXX_EXPORT TTCCLayout : public helpers::DateLayout
        {
        private:
                  // Internal representation of options
                  bool threadPrinting;
                  bool categoryPrefixing;
                  bool contextPrinting;
                  bool filePrinting;

        public:
                DECLARE_LOG4CXX_OBJECT(TTCCLayout)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(TTCCLayout)
                        LOG4CXX_CAST_ENTRY_CHAIN(Layout)
                END_LOG4CXX_CAST_MAP()

        /**
        Instantiate a TTCCLayout object with {@link
        helpers::RelativeTimeDateFormat RelativeTimeDateFormat} as the date
        formatter in the local time zone.
        */
                TTCCLayout();

        /**
        Instantiate a TTCCLayout object using the local time zone. The
        DateFormat used will depend on the <code>dateFormatType</code>.
        <p>This constructor just calls the {@link
        helpers::DateLayout#setDateFormat DateLayout::setDateFormat} method.
        */
                TTCCLayout(const LogString& dateFormatType);

        /**
        The <b>ThreadPrinting</b> option specifies whether the name of the
        current thread is part of log output or not. This is true by default.
        */
                inline void setThreadPrinting(bool threadPrinting1)
                        { this->threadPrinting = threadPrinting1; }

        /**
        Returns value of the <b>ThreadPrinting</b> option.
        */
                inline bool getThreadPrinting() const
                        { return threadPrinting; }

        /**
        The <b>CategoryPrefixing</b> option specifies whether Logger
        name is part of log output or not. This is true by default.
        */
                inline void setCategoryPrefixing(bool categoryPrefixing1)
                        { this->categoryPrefixing = categoryPrefixing1; }

        /**
        Returns value of the <b>CategoryPrefixing</b> option.
        */
                inline bool getCategoryPrefixing() const
                        { return categoryPrefixing; }

        /**
        The <b>ContextPrinting</b> option specifies log output will include
        the nested context information belonging to the current thread.
        This is true by default.
        */
                inline void setContextPrinting(bool contextPrinting1)
                        { this->contextPrinting = contextPrinting1; }

        /**
        Returns value of the <b>ContextPrinting</b> option.
        */
                inline bool getContextPrinting() const
                        { return contextPrinting; }

        /**
        The <b>FilePrinting</b> option specifies log output will include
        the file and the line where the log statement was written.
        */
                inline void setFilePrinting(bool filePrinting1)
                        { this->filePrinting = filePrinting1; }

        /**
        Returns value of the <b>ContextPrinting</b> option.
        */
                inline bool getFilePrinting() const
                        { return filePrinting; }

        /**
        In addition to the level of the statement and message, this function
        writes to the ouput stream time, thread, logger and NDC
        information.

        <p>Time, thread, logger and diagnostic context are printed
        depending on options.

        @param output destination to receive formatted output.
        @param event event to format.
        @param pool pool used to allocate memory needed during formatting.
        */
        virtual void format(LogString& output,
            const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& pool) const;

        /**
        The TTCCLayout does not handle the throwable contained within
        {@link spi::LoggingEvent LoggingEvents}. Thus, it returns
        <code>true</code>.
        */
        virtual bool ignoresThrowable() const { return true; }
        };
      LOG4CXX_PTR_DEF(TTCCLayout);
}


#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif
