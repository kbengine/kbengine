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

#ifndef _LOG4CXX_PATTERN_NAME_ABBREVIATOR
#define _LOG4CXX_PATTERN_NAME_ABBREVIATOR

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/helpers/objectimpl.h>

namespace log4cxx {
  namespace pattern {

    class NameAbbreviator;
    LOG4CXX_PTR_DEF(NameAbbreviator);

/**
 * NameAbbreviator generates abbreviated logger and class names.
 *
 * 
 * 
 */
class LOG4CXX_EXPORT NameAbbreviator : public log4cxx::helpers::ObjectImpl {
public:
   DECLARE_ABSTRACT_LOG4CXX_OBJECT(NameAbbreviator)
   BEGIN_LOG4CXX_CAST_MAP()
        LOG4CXX_CAST_ENTRY(NameAbbreviator)
   END_LOG4CXX_CAST_MAP()

protected:
   NameAbbreviator();

public:
   virtual ~NameAbbreviator();

  /**
   * Gets an abbreviator.
   *
   * For example, "%logger{2}" will output only 2 elements of the logger name,
   * "%logger{1.}" will output only the first character of the non-final elements in the name,
   * "%logger(1~.2~} will output the first character of the first element, two characters of
   * the second and subsequent elements and will use a tilde to indicate abbreviated characters.
   *
   * @param pattern abbreviation pattern.
   * @return abbreviator, will not be null.
   */
  static NameAbbreviatorPtr getAbbreviator(const LogString& pattern);

  /**
   * Gets default abbreviator.
   *
   * @return default abbreviator.
   */
  static NameAbbreviatorPtr getDefaultAbbreviator();

  /**
   * Abbreviates a name in a StringBuffer.
   *
   * @param nameStart starting position of name in buf.
   * @param buf buffer, may not be null.
   */
  virtual void abbreviate(LogString::size_type nameStart, LogString& buf) const = 0;

};
  }
}
#endif
