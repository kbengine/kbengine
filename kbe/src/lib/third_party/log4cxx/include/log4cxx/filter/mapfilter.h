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
#ifndef _LOG4CXX_FILTER_MAPFILTER_H
#define _LOG4CXX_FILTER_MAPFILTER_H

#include <log4cxx/spi/filter.h>

namespace log4cxx
{
    namespace filter
    {


        class LOG4CXX_EXPORT MapFilter:public log4cxx::spi::Filter
        {
          public:
            DECLARE_LOG4CXX_OBJECT(MapFilter)
            BEGIN_LOG4CXX_CAST_MAP()
            LOG4CXX_CAST_ENTRY(log4cxx::spi::Filter)
            END_LOG4CXX_CAST_MAP()

            MapFilter();


            FilterDecision decide(const spi::LoggingEventPtr & event) const;

        };
    }
}
#endif
