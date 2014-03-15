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

#ifndef _LOG4CXX_PATTERN_NAMED_PATTERN_CONVERTER_H
#define _LOG4CXX_PATTERN_NAMED_PATTERN_CONVERTER_H

#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <log4cxx/pattern/nameabbreviator.h>

#include <vector>

namespace log4cxx {
  namespace pattern {

/**
 *
 * Base class for other pattern converters which can return only parts of their name.
 *
 */
class LOG4CXX_EXPORT NamePatternConverter : public LoggingEventPatternConverter {
  /**
   * Abbreviator.
   */
  const NameAbbreviatorPtr abbreviator;

public:
DECLARE_LOG4CXX_PATTERN(NamePatternConverter)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(NamePatternConverter)
     LOG4CXX_CAST_ENTRY_CHAIN(LoggingEventPatternConverter)
END_LOG4CXX_CAST_MAP()


protected:
  /**
   * Constructor.
   * @param name name of converter.
   * @param style style name for associated output.
   * @param options options, may be null, first element will be interpreted as an abbreviation pattern.
   */
  NamePatternConverter(
    const LogString& name,
    const LogString& style,
    const std::vector<LogString>& options);

  /**
   * Abbreviate name in string buffer.
   * @param nameStart starting position of name to abbreviate.
   * @param buf string buffer containing name.
   */
  void abbreviate(int nameStart, LogString& buf) const;

private:
   NameAbbreviatorPtr getAbbreviator(const std::vector<LogString>& options);
};

  }
}

#endif
