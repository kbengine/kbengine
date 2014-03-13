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

#ifndef _LOG4CXX_CONSOLE_APPENDER_H
#define _LOG4CXX_CONSOLE_APPENDER_H

#include <log4cxx/writerappender.h>

namespace log4cxx
{
 
        /**
        * ConsoleAppender appends log events to <code>stdout</code> or
        * <code>stderr</code> using a layout specified by the user. The
        * default target is <code>stdout</code>.
        */
        class LOG4CXX_EXPORT ConsoleAppender : public WriterAppender
        {
        private:
                LogString target;

        public:
                DECLARE_LOG4CXX_OBJECT(ConsoleAppender)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(ConsoleAppender)
                        LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                END_LOG4CXX_CAST_MAP()

                ConsoleAppender();
                ConsoleAppender(const LayoutPtr& layout);
                ConsoleAppender(const LayoutPtr& layout, const LogString& target);
                ~ConsoleAppender();


                /**
                *  Sets the value of the <b>target</b> property. Recognized values
                *  are "System.out" and "System.err". Any other value will be
                *  ignored.
                * */
                void setTarget(const LogString& value);

                /**
                * Returns the current value of the <b>target</b> property. The
                * default value of the option is "System.out".
                *
                * See also #setTarget.
                * */
                LogString getTarget() const;

                void activateOptions(log4cxx::helpers::Pool& p);
                void setOption(const LogString& option, const LogString& value);
                static const LogString& getSystemOut();
                static const LogString& getSystemErr();


        private:
                void targetWarn(const LogString& val);
                static log4cxx::helpers::WriterPtr createWriter(const LogString& target);

        };
        LOG4CXX_PTR_DEF(ConsoleAppender);
}  //namespace log4cxx

#endif //_LOG4CXX_CONSOLE_APPENDER_H

