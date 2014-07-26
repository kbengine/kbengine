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

#ifndef _LOG4CXX_PATTERN_LOGGING_EVENT_PATTERN_CONVERTER_H
#define _LOG4CXX_PATTERN_LOGGING_EVENT_PATTERN_CONVERTER_H

#include <log4cxx/pattern/patternconverter.h>
#include <log4cxx/spi/loggingevent.h>

namespace log4cxx {

  namespace pattern {
    /**
    * LoggingEventPatternConverter is a base class for pattern converters
    * that can format information from instances of LoggingEvent.
    *
    * 
    *
    * 
    */
class LOG4CXX_EXPORT LoggingEventPatternConverter : public PatternConverter {
protected:
  /**
   * Constructs an instance of LoggingEventPatternConverter.
   * @param name name of converter.
   * @param style CSS style for output.
   */
  LoggingEventPatternConverter(
    const LogString& name, const LogString& style);

public:
DECLARE_LOG4CXX_PATTERN(LoggingEventPatternConverter)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(LoggingEventPatternConverter)
     LOG4CXX_CAST_ENTRY_CHAIN(PatternConverter)
END_LOG4CXX_CAST_MAP()

  /**
   * Formats an event into a string buffer.
   * @param event event to format, may not be null.
   * @param toAppendTo string buffer to which the formatted event will be appended.  May not be null.
   * @param p pool for memory allocations needing during format.
   */
  virtual void format(
    const log4cxx::spi::LoggingEventPtr& event,
    LogString& toAppendTo,
    log4cxx::helpers::Pool& p) const = 0;

  void format(const log4cxx::helpers::ObjectPtr& obj,
     LogString& toAppendTo,
     log4cxx::helpers::Pool& p) const;

  /**
   * Normally pattern converters are not meant to handle Exceptions although
   * few pattern converters might.
   *
   * By examining the return values for this method, the containing layout will
   * determine whether it handles throwables or not.

   * @return true if this PatternConverter handles throwables
   */
  virtual bool handlesThrowable() const;
};

LOG4CXX_PTR_DEF(LoggingEventPatternConverter);

  }
}

#endif
