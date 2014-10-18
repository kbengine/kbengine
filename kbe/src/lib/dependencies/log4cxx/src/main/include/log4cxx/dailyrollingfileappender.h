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

#ifndef _LOG4CXX_DAILYROLLINGFILEAPPENDER_H
#define _LOG4CXX_DAILYROLLINGFILEAPPENDER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/appender.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/spi/optionhandler.h>
#include <log4cxx/rolling/rollingfileappenderskeleton.h>

namespace log4cxx {
  namespace helpers {
    class Pool;
  }

  namespace spi {
    class ErrorHandler;
    typedef log4cxx::helpers::ObjectPtrT<ErrorHandler> ErrorHandlerPtr;
  }


/**
   DailyRollingFileAppender extends {@link log4cxx::FileAppender FileAppender} so that the
   underlying file is rolled over at a user chosen frequency.

   <p>The rolling schedule is specified by the <b>DatePattern</b>
   option. This pattern should follow the 
   {@link log4cxx::helpers::SimpleDateFormat SimpleDateFormat}
   conventions. In particular, you <em>must</em> escape literal text
   within a pair of single quotes. A formatted version of the date
   pattern is used as the suffix for the rolled file name.

   <p>For example, if the <b>File</b> option is set to
   <code>/foo/bar.log</code> and the <b>DatePattern</b> set to
   <code>'.'yyyy-MM-dd</code>, on 2001-02-16 at midnight, the logging
   file <code>/foo/bar.log</code> will be copied to
   <code>/foo/bar.log.2001-02-16</code> and logging for 2001-02-17
   will continue in <code>/foo/bar.log</code> until it rolls over
   the next day.

   <p>Is is possible to specify monthly, weekly, half-daily, daily,
   hourly, or minutely rollover schedules.

   <p><table border="1" cellpadding="2">
   <tr>
   <th>DatePattern</th>
   <th>Rollover schedule</th>
   <th>Example</th>

   <tr>
   <td><code>'.'yyyy-MM</code>
   <td>Rollover at the beginning of each month</td>

   <td>At midnight of May 31st, 2002 <code>/foo/bar.log</code> will be
   copied to <code>/foo/bar.log.2002-05</code>. Logging for the month
   of June will be output to <code>/foo/bar.log</code> until it is
   also rolled over the next month.

   <tr>
   <td><code>'.'yyyy-ww</code>

   <td>Rollover at the first day of each week. The first day of the
   week depends on the locale.</td>

   <td>Assuming the first day of the week is Sunday, on Saturday
   midnight, June 9th 2002, the file <i>/foo/bar.log</i> will be
   copied to <i>/foo/bar.log.2002-23</i>.  Logging for the 24th week
   of 2002 will be output to <code>/foo/bar.log</code> until it is
   rolled over the next week.

   <tr>
   <td><code>'.'yyyy-MM-dd</code>

   <td>Rollover at midnight each day.</td>

   <td>At midnight, on March 8th, 2002, <code>/foo/bar.log</code> will
   be copied to <code>/foo/bar.log.2002-03-08</code>. Logging for the
   9th day of March will be output to <code>/foo/bar.log</code> until
   it is rolled over the next day.

   <tr>
   <td><code>'.'yyyy-MM-dd-a</code>

   <td>Rollover at midnight and midday of each day.</td>

   <td>At noon, on March 9th, 2002, <code>/foo/bar.log</code> will be
   copied to <code>/foo/bar.log.2002-03-09-AM</code>. Logging for the
   afternoon of the 9th will be output to <code>/foo/bar.log</code>
   until it is rolled over at midnight.

   <tr>
   <td><code>'.'yyyy-MM-dd-HH</code>

   <td>Rollover at the top of every hour.</td>

   <td>At approximately 11:00.000 o'clock on March 9th, 2002,
   <code>/foo/bar.log</code> will be copied to
   <code>/foo/bar.log.2002-03-09-10</code>. Logging for the 11th hour
   of the 9th of March will be output to <code>/foo/bar.log</code>
   until it is rolled over at the beginning of the next hour.


   <tr>
   <td><code>'.'yyyy-MM-dd-HH-mm</code>

   <td>Rollover at the beginning of every minute.</td>

   <td>At approximately 11:23,000, on March 9th, 2001,
   <code>/foo/bar.log</code> will be copied to
   <code>/foo/bar.log.2001-03-09-10-22</code>. Logging for the minute
   of 11:23 (9th of March) will be output to
   <code>/foo/bar.log</code> until it is rolled over the next minute.

   </table>

   <p>Do not use the colon ":" character in anywhere in the
   <b>DatePattern</b> option. The text before the colon is interpeted
   as the protocol specificaion of a URL which is probably not what
   you want.
*/

  class LOG4CXX_EXPORT DailyRollingFileAppender : public log4cxx::rolling::RollingFileAppenderSkeleton {
  DECLARE_LOG4CXX_OBJECT(DailyRollingFileAppender)
  BEGIN_LOG4CXX_CAST_MAP()
          LOG4CXX_CAST_ENTRY(DailyRollingFileAppender)
          LOG4CXX_CAST_ENTRY_CHAIN(FileAppender)
  END_LOG4CXX_CAST_MAP()

  /**
     The date pattern used to initiate rollover.
  */
  LogString datePattern;


public:
  /**
     The default constructor simply calls its {@link
     FileAppender#FileAppender parents constructor}.  */
  DailyRollingFileAppender();

  /**
    Instantiate a DailyRollingFileAppender and open the file designated by
    <code>filename</code>. The opened filename will become the ouput
    destination for this appender.

  */
  DailyRollingFileAppender(
    const LayoutPtr& layout,
    const LogString& filename,
    const LogString& datePattern);


  /**
     The <b>DatePattern</b> takes a string in the same format as
     expected by {@link log4cxx::helpers::SimpleDateFormat SimpleDateFormat}. This options determines the
     rollover schedule.
   */
  void setDatePattern(const LogString& pattern);

  /** Returns the value of the <b>DatePattern</b> option. */
  LogString getDatePattern();

  void setOption(const LogString& option,
   const LogString& value);

  /**
   * Prepares DailyRollingFileAppender for use.
   */
  void activateOptions(log4cxx::helpers::Pool&);

};

LOG4CXX_PTR_DEF(DailyRollingFileAppender);

}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif
