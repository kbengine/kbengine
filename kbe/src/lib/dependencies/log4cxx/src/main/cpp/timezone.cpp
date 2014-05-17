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
#if defined(_MSC_VER)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#define __STDC_CONSTANT_MACROS
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/timezone.h>
#include <stdlib.h>

#include <apr_time.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/logger.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT( TimeZone )

namespace log4cxx
{
  namespace helpers
  {
    namespace TimeZoneImpl
    {
      /** Time zone object that represents GMT. */
      class GMTTimeZone : public TimeZone
      {
      public:
        /** Class factory. */
        static const TimeZonePtr & getInstance()
        {
          static TimeZonePtr tz( new GMTTimeZone() );
          return tz;
        }

        /** Explode time to human readable form. */
        log4cxx_status_t explode( apr_time_exp_t * result, log4cxx_time_t input ) const
        {
           apr_status_t stat;
           //  APR 1.1 and early mishandles microseconds on dates
           //   before 1970, APR bug 32520
           if (LOG4CXX_UNLIKELY(input < 0 && apr_time_usec(input) < 0)) {
              apr_time_t floorTime = (apr_time_sec(input) -1) * APR_USEC_PER_SEC;
              stat = apr_time_exp_gmt(result, floorTime);
              result->tm_usec = (int) (input - floorTime);
           } else {
              stat = apr_time_exp_gmt( result, input );
           }
           return stat;
        }

      private:
        GMTTimeZone() : TimeZone( LOG4CXX_STR("GMT") )
        {
        }
      };



      /** Time zone object that represents GMT. */
      class LocalTimeZone : public TimeZone
      {
      public:
        /** Class factory. */
        static const TimeZonePtr & getInstance()
        {
          static TimeZonePtr tz( new LocalTimeZone() );
          return tz;
        }

        /** Explode time to human readable form. */
        log4cxx_status_t explode( apr_time_exp_t * result, log4cxx_time_t input ) const
        {
          apr_status_t stat;
          //  APR 1.1 and early mishandles microseconds on dates
          //   before 1970, APR bug 32520
          if (LOG4CXX_UNLIKELY(input < 0 && apr_time_usec(input) < 0)) {
             apr_time_t floorTime = (apr_time_sec(input) -1) * APR_USEC_PER_SEC;
             stat = apr_time_exp_lt(result, floorTime);
             result->tm_usec = (int) (input - floorTime);
          } else {
             stat = apr_time_exp_lt( result, input );
          }
          return stat;
        }


      private:
        LocalTimeZone() : TimeZone( getTimeZoneName() )
        {
        }

        static const LogString getTimeZoneName()
        {
          const int MAX_TZ_LENGTH = 255;
          char tzName[MAX_TZ_LENGTH];
          apr_size_t tzLength;
          apr_time_exp_t tm;
          apr_time_exp_lt(&tm, 0);
          apr_strftime(tzName, &tzLength, MAX_TZ_LENGTH, "%Z", &tm);
          if (tzLength == 0) {
            apr_strftime(tzName, &tzLength, MAX_TZ_LENGTH, "%z", &tm);
          }
          tzName[tzLength] = 0;
          LogString retval;
          log4cxx::helpers::Transcoder::decode(tzName, retval);
          return retval;
        }

      };



      /** Time zone object that represents a fixed offset from GMT. */
      class FixedTimeZone : public TimeZone
      {
      public:
        FixedTimeZone( const LogString & name, apr_int32_t offset1 ) : TimeZone( name ), offset( offset1 )
        {
        }

        /** Explode time to human readable form. */
        log4cxx_status_t explode( apr_time_exp_t * result, log4cxx_time_t input ) const
        {
          apr_status_t stat;
          //  APR 1.1 and early mishandles microseconds on dates
          //   before 1970, APR bug 32520
          if (LOG4CXX_UNLIKELY(input < 0 && apr_time_usec(input) < 0)) {
             apr_time_t floorTime = (apr_time_sec(input) -1) * APR_USEC_PER_SEC;
             stat = apr_time_exp_tz(result, floorTime, offset);
             result->tm_usec = (int) (input - floorTime);
          } else {
             stat = apr_time_exp_tz( result, input, offset );
          }
          return stat;
        }


      private:
        const apr_int32_t offset;
      };

    }
  }
}



TimeZone::TimeZone( const LogString & id1 ) : id( id1 )
{
}

TimeZone::~TimeZone()
{
}

const TimeZonePtr & TimeZone::getDefault()
{
  return log4cxx::helpers::TimeZoneImpl::LocalTimeZone::getInstance();
}

const TimeZonePtr & TimeZone::getGMT()
{
  return log4cxx::helpers::TimeZoneImpl::GMTTimeZone::getInstance();
}

const TimeZonePtr TimeZone::getTimeZone( const LogString & id )
{
  const logchar gmt[] = { 0x47, 0x4D, 0x54, 0 };
  if ( id == gmt )
  {
    return log4cxx::helpers::TimeZoneImpl::GMTTimeZone::getInstance();
  }
  if ( id.length() >= 5 && id.substr( 0, 3 ) == gmt )
  {
    int hours = 0;
    int minutes = 0;
    int sign = 1;
    if (id[3] == 0x2D /* '-' */) {
      sign = -1;
    }
    LogString off( id.substr( 4 ) );
    if ( id.length() >= 7 )
    {
      size_t colonPos = off.find( 0x3A /* ':' */);
      if ( colonPos == LogString::npos )
      {
        minutes = StringHelper::toInt(off.substr(off.length() - 2));
        hours = StringHelper::toInt(off.substr(0, off.length() - 2));
      }
      else
      {
        minutes = StringHelper::toInt(off.substr(colonPos + 1));
        hours = StringHelper::toInt(off.substr(0, colonPos));
      }
    } else {
      hours = StringHelper::toInt(off);
    }
    LogString s(gmt);
    Pool p;
    LogString hh;
    StringHelper::toString(hours, p, hh);
    if (sign > 0) {
      s.append(1, (logchar) 0x2B /* '+' */);
    } else {
      s.append(1, (logchar) 0x2D /* '-' */);
    }
    if (hh.length() == 1) {
      s.append(1, (logchar) 0x30 /* '0' */);
    }
    s.append(hh);
    s.append(1, (logchar) 0x3A /*' :' */);
    LogString mm;
    StringHelper::toString(minutes, p, mm);
    if (mm.length() == 1) {
      s.append(1, (logchar) 0x30 /* '0' */);
    }
    s.append(mm);
    apr_int32_t offset = sign * (hours * 3600 + minutes * 60);
    return new log4cxx::helpers::TimeZoneImpl::FixedTimeZone( s, offset );
  }
  const TimeZonePtr & ltz = getDefault();
  if ( ltz->getID() == id )
  {
    return ltz;
  }
  return getGMT();
}

