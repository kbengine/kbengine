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

#include <log4cxx/logstring.h>
#include <log4cxx/pattern/classnamepatternconverter.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>

 using namespace log4cxx;
 using namespace log4cxx::pattern;
 using namespace log4cxx::spi;
 using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(ClassNamePatternConverter)

ClassNamePatternConverter::ClassNamePatternConverter(
    const std::vector<LogString>& options) :
    NamePatternConverter(LOG4CXX_STR("Class Name"),
       LOG4CXX_STR("class name"), options) {
}

PatternConverterPtr ClassNamePatternConverter::newInstance(
    const std::vector<LogString>& options) {
    if (options.size() == 0) {
      static PatternConverterPtr def(new ClassNamePatternConverter(options));
      return def;
    }
    return new ClassNamePatternConverter(options);
}

void ClassNamePatternConverter::format(
   const LoggingEventPtr& event,
   LogString& toAppendTo,
   Pool& /* p */) const {
    int initialLength = toAppendTo.length();
    append(toAppendTo, event->getLocationInformation().getClassName());
    abbreviate(initialLength, toAppendTo);
  }
