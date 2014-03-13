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
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/datelayout.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/helpers/dateformat.h>
#include <log4cxx/helpers/relativetimedateformat.h>
#include <log4cxx/helpers/absolutetimedateformat.h>
#include <log4cxx/helpers/datetimedateformat.h>
#include <log4cxx/helpers/iso8601dateformat.h>
#include <log4cxx/helpers/timezone.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

DateLayout::DateLayout(const LogString& dateFormatOption1) :
   timeZoneID(), dateFormatOption(dateFormatOption1), dateFormat(0)
{
}

DateLayout::~DateLayout()
{
}


void DateLayout::setOption(const LogString& option, const LogString& value)
{

        if (StringHelper::equalsIgnoreCase(option,
                 LOG4CXX_STR("DATEFORMAT"), LOG4CXX_STR("dateformat")))
        {
                dateFormatOption = value;
        }
        else if (StringHelper::equalsIgnoreCase(option,
                 LOG4CXX_STR("TIMEZONE"), LOG4CXX_STR("timezone")))
        {
                timeZoneID = value;
        }
}

void DateLayout::activateOptions(Pool&)
{
        if(!dateFormatOption.empty())
        {

          if(dateFormatOption.empty())
          {
                  dateFormat = 0;
          }
          else if(StringHelper::equalsIgnoreCase(dateFormatOption,
                  LOG4CXX_STR("NULL"), LOG4CXX_STR("null")))
          {
                  dateFormat = 0;
                  dateFormatOption = LOG4CXX_STR("NULL");
          }
          else if(StringHelper::equalsIgnoreCase(dateFormatOption,
                  LOG4CXX_STR("RELATIVE"), LOG4CXX_STR("relative")))
          {
                  dateFormat =  new RelativeTimeDateFormat();
                  dateFormatOption = LOG4CXX_STR("RELATIVE");
          }
          else if(StringHelper::equalsIgnoreCase(dateFormatOption,
                  LOG4CXX_STR("ABSOLUTE"),  LOG4CXX_STR("absolute")))
          {
                  dateFormat =  new AbsoluteTimeDateFormat();
                  dateFormatOption = LOG4CXX_STR("ABSOLUTE");
          }
          else if(StringHelper::equalsIgnoreCase(dateFormatOption,
                  LOG4CXX_STR("DATE"), LOG4CXX_STR("date")))
          {
                  dateFormat =  new DateTimeDateFormat();
                  dateFormatOption = LOG4CXX_STR("DATE");
          }
          else if(StringHelper::equalsIgnoreCase(dateFormatOption,
                  LOG4CXX_STR("ISO8601"), LOG4CXX_STR("iso8601")))
          {
                  dateFormat =  new ISO8601DateFormat();
                  dateFormatOption = LOG4CXX_STR("iso8601");
          }
          else
          {
                  dateFormat = new SimpleDateFormat(dateFormatOption);
          }
        }
        if (dateFormat != NULL) {
           if (timeZoneID.empty()) {
              dateFormat->setTimeZone(TimeZone::getDefault());
           } else {
              dateFormat->setTimeZone(TimeZone::getTimeZone(timeZoneID));
           }
        }
}


void DateLayout::formatDate(LogString &s,
                            const spi::LoggingEventPtr& event,
                            Pool& p) const {

        if(dateFormat != 0)
        {
                dateFormat->format(s, event->getTimeStamp(), p);
                s.append(1, (logchar) 0x20 /* ' ' */);
        }
}

