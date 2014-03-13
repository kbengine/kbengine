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

#ifndef _LOG4CXX_PATTERN_DATE_PATTERN_CONVERTER
#define _LOG4CXX_PATTERN_DATE_PATTERN_CONVERTER

#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <log4cxx/helpers/cacheddateformat.h>
#include <log4cxx/helpers/date.h>
#include <vector>

namespace log4cxx {
  namespace pattern {


/**
 * Convert and format the event's date in a StringBuffer.
 *
 * 
 * 
 */
class LOG4CXX_EXPORT DatePatternConverter : public LoggingEventPatternConverter {
  /**
   * Date format.
   */
  log4cxx::helpers::DateFormatPtr df;

  /**
   * Private constructor.
   * @param options options, may be null.
   * @param logger logger for diagnostic messages, may be null.
   */
  DatePatternConverter(const OptionsList& options);

  /**
   * Obtains an instance of pattern converter.
   * @param options options, may be null.
   * @return instance of pattern converter.
   */
  static log4cxx::helpers::DateFormatPtr getDateFormat(const OptionsList& options);
  public:
  DECLARE_LOG4CXX_PATTERN(DatePatternConverter)
  BEGIN_LOG4CXX_CAST_MAP()
       LOG4CXX_CAST_ENTRY(DatePatternConverter)
       LOG4CXX_CAST_ENTRY_CHAIN(LoggingEventPatternConverter)
  END_LOG4CXX_CAST_MAP()


 static PatternConverterPtr newInstance(
    const std::vector<LogString>& options);
  void format(const log4cxx::spi::LoggingEventPtr& event,
     LogString& output,
     log4cxx::helpers::Pool& p) const;
  void format(const log4cxx::helpers::ObjectPtr& obj,
     LogString& output,
     log4cxx::helpers::Pool& p) const;

  void format(const log4cxx::helpers::DatePtr& date,
     LogString& toAppendTo,
     log4cxx::helpers::Pool& p) const;
};

LOG4CXX_PTR_DEF(DatePatternConverter);

  }
}
#endif

