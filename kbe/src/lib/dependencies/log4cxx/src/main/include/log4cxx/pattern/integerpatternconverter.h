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

#ifndef _LOG4CXX_PATTERN_INTEGER_PATTERN_CONVERTER
#define _LOG4CXX_PATTERN_INTEGER_PATTERN_CONVERTER

#include <log4cxx/pattern/patternconverter.h>

namespace log4cxx { namespace pattern {


/**
 * Formats an integer.
 *
 * 
 * 
 */
class LOG4CXX_EXPORT IntegerPatternConverter : public PatternConverter {

  /**
   * Private constructor.
   */
  IntegerPatternConverter();

public:
  DECLARE_LOG4CXX_PATTERN(IntegerPatternConverter)
  BEGIN_LOG4CXX_CAST_MAP()
       LOG4CXX_CAST_ENTRY(IntegerPatternConverter)
       LOG4CXX_CAST_ENTRY_CHAIN(PatternConverter)
  END_LOG4CXX_CAST_MAP()

  /**
   * Obtains an instance of pattern converter.
   * @param options options, may be null.
   * @return instance of pattern converter.
   */
  static PatternConverterPtr newInstance(
    const std::vector<LogString>& options);

  void format(const log4cxx::helpers::ObjectPtr& obj,
      LogString& toAppendTo,
      log4cxx::helpers::Pool& p) const;
};

LOG4CXX_PTR_DEF(IntegerPatternConverter);

}
}
#endif

