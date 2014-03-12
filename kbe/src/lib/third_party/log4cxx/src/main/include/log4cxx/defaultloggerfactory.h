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

#ifndef _LOG4CXX_DEFAULT_LOGGER_FACTORY_H
#define _LOG4CXX_DEFAULT_LOGGER_FACTORY_H

#include <log4cxx/spi/loggerfactory.h>
#include <log4cxx/helpers/objectimpl.h>

namespace log4cxx
{
        class Logger;
        typedef helpers::ObjectPtrT<Logger> LoggerPtr;

        class LOG4CXX_EXPORT DefaultLoggerFactory :
                public virtual spi::LoggerFactory,
                public virtual helpers::ObjectImpl
        {
        public:
                DECLARE_ABSTRACT_LOG4CXX_OBJECT(DefaultLoggerFactory)
                BEGIN_LOG4CXX_CAST_MAP()
                        LOG4CXX_CAST_ENTRY(spi::LoggerFactory)
                END_LOG4CXX_CAST_MAP()

                virtual LoggerPtr makeNewLoggerInstance(
                    log4cxx::helpers::Pool& pool, 
                    const LogString& name) const;
        };
}  // namespace log4cxx

#endif //_LOG4CXX_DEFAULT_LOGGER_FACTORY_H
