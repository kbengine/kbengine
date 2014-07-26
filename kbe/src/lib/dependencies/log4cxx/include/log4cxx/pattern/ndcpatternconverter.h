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

#ifndef _LOG4CXX_PATTERN_NDC_PATTERN_CONVERTER
#define _LOG4CXX_PATTERN_NDC_PATTERN_CONVERTER

#include <log4cxx/pattern/loggingeventpatternconverter.h>

namespace log4cxx { namespace pattern {


/**
 * Return the event's NDC in a StringBuffer.
 *
 * 
 * 
 */
class LOG4CXX_EXPORT NDCPatternConverter : public LoggingEventPatternConverter {

  /**
   * Private constructor.
   */
  NDCPatternConverter();

public:
DECLARE_LOG4CXX_PATTERN(NDCPatternConverter)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(NDCPatternConverter)
     LOG4CXX_CAST_ENTRY_CHAIN(LoggingEventPatternConverter)
END_LOG4CXX_CAST_MAP()

  /**
   * Obtains an instance of NDCPatternConverter.
   * @param options options, may be null.
   * @return instance of NDCPatternConverter.
   */
  static PatternConverterPtr newInstance(
    const std::vector<LogString>& options);

  void format(const log4cxx::spi::LoggingEventPtr& event,
     LogString& toAppendTo,
     log4cxx::helpers::Pool& p) const;
};
}
}
#endif
