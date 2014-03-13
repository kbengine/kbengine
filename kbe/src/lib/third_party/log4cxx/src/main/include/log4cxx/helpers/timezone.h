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

#ifndef _LOG4CXX_HELPERS_TIMEZONE_H
#define _LOG4CXX_HELPERS_TIMEZONE_H

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/objectimpl.h>
#include <log4cxx/helpers/objectptr.h>

struct apr_time_exp_t;

namespace log4cxx
{
   namespace helpers
   {
      class TimeZone;
      LOG4CXX_PTR_DEF(TimeZone);

      class LOG4CXX_EXPORT TimeZone : public helpers::ObjectImpl
      {
      public:
         DECLARE_ABSTRACT_LOG4CXX_OBJECT(TimeZone)
         BEGIN_LOG4CXX_CAST_MAP()
            LOG4CXX_CAST_ENTRY(TimeZone)
         END_LOG4CXX_CAST_MAP()

         static const TimeZonePtr& getDefault();
            static const TimeZonePtr& getGMT();
         static const TimeZonePtr getTimeZone(const LogString& ID);

            const LogString getID() const {
                   return id;
            }


            /**
             *   Expand an APR time into the human readable
             *      components for this timezone.
             */
            virtual log4cxx_status_t explode(apr_time_exp_t* result,
                    log4cxx_time_t input) const = 0;


      protected:
            TimeZone(const LogString& ID);
            virtual ~TimeZone();

            const LogString id;
      };


   }
}

#endif //_LOG4CXX_HELPERS_TIMEZONE_H
