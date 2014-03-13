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

#ifndef _LOG4CXX_SPI_OPTION_HANDLER_H
#define _LOG4CXX_SPI_OPTION_HANDLER_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/object.h>
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
        namespace spi
        {
                class OptionHandler;
                typedef helpers::ObjectPtrT<OptionHandler> OptionHandlerPtr;

                /**
                A string based interface to configure package components.
                */
                class LOG4CXX_EXPORT OptionHandler : public virtual helpers::Object
                {
                public:
                        DECLARE_ABSTRACT_LOG4CXX_OBJECT(OptionHandler)
                        virtual ~OptionHandler() {}

                        /**
                        Activate the options that were previously set with calls to option
                        setters.

                        <p>This allows to defer activiation of the options until all
                        options have been set. This is required for components which have
                        related options that remain ambigous until all are set.

                        <p>For example, the FileAppender has the {@link
                        FileAppender#setFile File} and {@link
                        FileAppender#setAppend Append} options both of
                        which are ambigous until the other is also set.  */
                        virtual void activateOptions(log4cxx::helpers::Pool& p) = 0;


                        /**
                        Set <code>option</code> to <code>value</code>.

                        <p>The handling of each option depends on the OptionHandler
                        instance. Some options may become active immediately whereas
                        other may be activated only when #activateOptions is
                        called.
                        */
                        virtual void setOption(const LogString& option,
                            const LogString& value) = 0;

                }; // class OptionConverter
        }  // namespace spi
} // namespace log4cxx


#endif //_LOG4CXX_SPI_OPTION_HANDLER_H
