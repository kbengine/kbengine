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


#ifndef _LOG4CXX_FILTER_PROPERTYFILTER_H
#define _LOG4CXX_FILTER_PROPERTYFILTER_H

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/spi/filter.h>
#include <map>

namespace log4cxx
{
    namespace filter
    {

/**
 * NOTE: This filter modifies logging events by adding properties to the event.
 *
 * The 'properties' param is converted to event properties, which are
 * set on every event processed by the filter.
 *
 * Individual properties are only set if they do not already exist on the
 * logging event (will not override existing properties).
 *
 * This class relies on the convention that property name/value pairs are
 * equals-symbol delimited, and each name/value pair is comma-delimited
 *
 * Example properties param:
 * somename=somevalue,anothername=anothervalue,thirdname=third value
 *
 * 
 */
        class LOG4CXX_EXPORT PropertyFilter : public log4cxx::spi::Filter
        {
            typedef std::map < LogString, LogString > PropertyMap;
            PropertyMap* properties;
            PropertyFilter(const PropertyFilter &);
                  PropertyFilter & operator=(const PropertyFilter &);

          public:
                  DECLARE_LOG4CXX_OBJECT(PropertyFilter)
                  BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(log4cxx::spi::Filter)
                  END_LOG4CXX_CAST_MAP()

                  PropertyFilter();
                  ~PropertyFilter();
            void setProperties(const LogString & props);

            FilterDecision decide(const spi::LoggingEventPtr & event) const;

        };

    }
}

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif
