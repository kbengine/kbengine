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

#ifndef _LOG4CXX_HTML_LAYOUT_H
#define _LOG4CXX_HTML_LAYOUT_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/layout.h>
#include <log4cxx/helpers/iso8601dateformat.h>



namespace log4cxx
{
        /**
        This layout outputs events in a HTML table.
        */
        class LOG4CXX_EXPORT HTMLLayout : public Layout
        {
        private:
                // Print no location info by default
                bool locationInfo; //= false

                LogString title;

                helpers::ISO8601DateFormat dateFormat;

        public:
                DECLARE_LOG4CXX_OBJECT(HTMLLayout)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(HTMLLayout)
                        LOG4CXX_CAST_ENTRY_CHAIN(Layout)
                END_LOG4CXX_CAST_MAP()

                HTMLLayout();

                /**
                The <b>LocationInfo</b> option takes a boolean value. By
                default, it is set to false which means there will be no location
                information output by this layout. If the the option is set to
                true, then the file name and line number of the statement
                at the origin of the log statement will be output.

                <p>If you are embedding this layout within an
                {@link net::SMTPAppender SMTPAppender} then make sure
                to set the <b>LocationInfo</b> option of that appender as well.
                */
                inline void setLocationInfo(bool locationInfoFlag)
                        { this->locationInfo = locationInfoFlag; }

                /**
                Returns the current value of the <b>LocationInfo</b> option.
                */
                inline bool getLocationInfo() const
                        { return locationInfo; }

                /**
                The <b>Title</b> option takes a String value. This option sets the
                document title of the generated HTML document.
                <p>Defaults to 'Log4cxx Log Messages'.
                */
                inline void setTitle(const LogString& title1)
                        { this->title.assign(title1); }

                /**
                Returns the current value of the <b>Title</b> option.
                */
                inline const LogString& getTitle() const
                        { return title; }

                /**
                Returns the content type output by this layout, i.e "text/html".
                */
                virtual LogString getContentType() const { return LOG4CXX_STR("text/html"); }

                /**
                No options to activate.
                */
                virtual void activateOptions(log4cxx::helpers::Pool& /* p */) {}

                /**
                Set options
                */
                virtual void setOption(const LogString& option, const LogString& value);

                virtual void format(LogString& output,
                     const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& pool) const;

                /**
                Append appropriate HTML headers.
                */
                virtual void appendHeader(LogString& output, log4cxx::helpers::Pool& pool);

                /**
                Append the appropriate HTML footers.
                */
                virtual void appendFooter(LogString& output, log4cxx::helpers::Pool& pool);

                /**
                The HTML layout handles the throwable contained in logging
                events. Hence, this method return <code>false</code>.  */
                virtual bool ignoresThrowable() const
                        { return false; }

        }; // class HtmlLayout
      LOG4CXX_PTR_DEF(HTMLLayout);
}  // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif // _LOG4CXX_HTML_LAYOUT_H
