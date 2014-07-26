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

#ifndef _LOG4CXX_PATTERN_CLASSNAME_PATTERN_CONVERTER_H
#define _LOG4CXX_PATTERN_CLASSNAME_PATTERN_CONVERTER_H


#include <log4cxx/pattern/namepatternconverter.h>

namespace log4cxx {
 namespace pattern {


/**
 * Formats the class name of the site of the logging request.
 *
 * 
 * 
 */
class LOG4CXX_EXPORT ClassNamePatternConverter : public NamePatternConverter {
  /**
   * Private constructor.
   * @param options options, may be null.
   * @param logger logger for diagnostic messages, may be null.
   */
  ClassNamePatternConverter(
    const std::vector<LogString>& options);

public:
   DECLARE_LOG4CXX_PATTERN(ClassNamePatternConverter)
   BEGIN_LOG4CXX_CAST_MAP()
        LOG4CXX_CAST_ENTRY(ClassNamePatternConverter)
        LOG4CXX_CAST_ENTRY_CHAIN(NamePatternConverter)
   END_LOG4CXX_CAST_MAP()

  /**
   * Gets an instance of ClassNamePatternConverter.
   * @param options options, may be null.
   * @return instance of pattern converter.
   */
  static PatternConverterPtr newInstance(
    const std::vector<LogString>& options);

  void format(const log4cxx::spi::LoggingEventPtr&event,
     LogString& toAppendTo,
     log4cxx::helpers::Pool& p) const;
};

 }
}
#endif
