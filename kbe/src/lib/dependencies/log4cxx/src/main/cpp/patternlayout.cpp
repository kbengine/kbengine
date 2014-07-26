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
#include <log4cxx/patternlayout.h>
#include <log4cxx/pattern/patternparser.h>
#include <log4cxx/pattern/loggingeventpatternconverter.h>
#include <log4cxx/pattern/formattinginfo.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/optionconverter.h>

#include <log4cxx/pattern/loggerpatternconverter.h>
#include <log4cxx/pattern/literalpatternconverter.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/pattern/classnamepatternconverter.h>
#include <log4cxx/pattern/datepatternconverter.h>
#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/pattern/filelocationpatternconverter.h>
#include <log4cxx/pattern/fulllocationpatternconverter.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/pattern/linelocationpatternconverter.h>
#include <log4cxx/pattern/messagepatternconverter.h>
#include <log4cxx/pattern/lineseparatorpatternconverter.h>
#include <log4cxx/pattern/methodlocationpatternconverter.h>
#include <log4cxx/pattern/levelpatternconverter.h>
#include <log4cxx/pattern/relativetimepatternconverter.h>
#include <log4cxx/pattern/threadpatternconverter.h>
#include <log4cxx/pattern/ndcpatternconverter.h>
#include <log4cxx/pattern/propertiespatternconverter.h>
#include <log4cxx/pattern/throwableinformationpatternconverter.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;
using namespace log4cxx::pattern;

IMPLEMENT_LOG4CXX_OBJECT(PatternLayout)


PatternLayout::PatternLayout()
{
}

/**
Constructs a PatternLayout using the supplied conversion pattern.
*/
PatternLayout::PatternLayout(const LogString& pattern)
  : conversionPattern(pattern) {
  Pool pool;
  activateOptions(pool);
}

void PatternLayout::setConversionPattern(const LogString& pattern)
{
    conversionPattern = pattern;
    Pool pool;
    activateOptions(pool);
}

void PatternLayout::format(LogString& output,
      const spi::LoggingEventPtr& event,
      Pool& pool) const
{
  std::vector<FormattingInfoPtr>::const_iterator formatterIter =
     patternFields.begin();
  for(std::vector<LoggingEventPatternConverterPtr>::const_iterator
           converterIter = patternConverters.begin();
      converterIter != patternConverters.end();
      converterIter++, formatterIter++) {
      int startField = output.length();
      (*converterIter)->format(event, output, pool);
      (*formatterIter)->format(startField, output);
  }

}

void PatternLayout::setOption(const LogString& option, const LogString& value)
{
        if (StringHelper::equalsIgnoreCase(option,
               LOG4CXX_STR("CONVERSIONPATTERN"),
               LOG4CXX_STR("conversionpattern")))
        {
                conversionPattern = OptionConverter::convertSpecialChars(value);
        }
}

void PatternLayout::activateOptions(Pool&)
{
        LogString pat(conversionPattern);
        if (pat.empty()) {
            pat = LOG4CXX_STR("%m%n");
        }
        patternConverters.erase(patternConverters.begin(), patternConverters.end());
        patternFields.erase(patternFields.begin(), patternFields.end());
        std::vector<PatternConverterPtr> converters;
        PatternParser::parse(pat,
                converters,
                patternFields,
                getFormatSpecifiers());

       //
       //   strip out any pattern converters that don't handle LoggingEvents
       //
       //
       for(std::vector<PatternConverterPtr>::const_iterator converterIter = converters.begin();
           converterIter != converters.end();
           converterIter++) {
           LoggingEventPatternConverterPtr eventConverter(*converterIter);
           if (eventConverter != NULL) {
             patternConverters.push_back(eventConverter);
           }
       }
}

#define RULES_PUT(spec, cls) \
specs.insert(PatternMap::value_type(LogString(LOG4CXX_STR(spec)), (PatternConstructor) cls ::newInstance))


log4cxx::pattern::PatternMap PatternLayout::getFormatSpecifiers() {
  PatternMap specs;
  RULES_PUT("c", LoggerPatternConverter);
  RULES_PUT("logger", LoggerPatternConverter);

  RULES_PUT("C", ClassNamePatternConverter);
  RULES_PUT("class", ClassNamePatternConverter);

  RULES_PUT("d", DatePatternConverter);
  RULES_PUT("date", DatePatternConverter);

  RULES_PUT("F", FileLocationPatternConverter);
  RULES_PUT("file", FileLocationPatternConverter);

  RULES_PUT("l", FullLocationPatternConverter);

  RULES_PUT("L", LineLocationPatternConverter);
  RULES_PUT("line", LineLocationPatternConverter);

  RULES_PUT("m", MessagePatternConverter);
  RULES_PUT("message", MessagePatternConverter);

  RULES_PUT("n", LineSeparatorPatternConverter);

  RULES_PUT("M", MethodLocationPatternConverter);
  RULES_PUT("method", MethodLocationPatternConverter);

  RULES_PUT("p", LevelPatternConverter);
  RULES_PUT("level", LevelPatternConverter);

  RULES_PUT("r", RelativeTimePatternConverter);
  RULES_PUT("relative", RelativeTimePatternConverter);

  RULES_PUT("t", ThreadPatternConverter);
  RULES_PUT("thread", ThreadPatternConverter);

  RULES_PUT("x", NDCPatternConverter);
  RULES_PUT("ndc", NDCPatternConverter);

  RULES_PUT("X", PropertiesPatternConverter);
  RULES_PUT("properties", PropertiesPatternConverter);

  RULES_PUT("throwable", ThrowableInformationPatternConverter);
   return specs;
}





