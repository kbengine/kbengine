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


#ifndef _LOG4CXX_HELPER_PATTERN_CONVERTER_H
#define _LOG4CXX_HELPER_PATTERN_CONVERTER_H

#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif



#include <map>
#include <vector>
#include <log4cxx/helpers/class.h>
#include <log4cxx/pattern/patternconverter.h>
#include <log4cxx/pattern/formattinginfo.h>

namespace log4cxx {
  namespace pattern {

typedef PatternConverterPtr (*PatternConstructor)(const std::vector<LogString>& options);
typedef std::map<LogString, PatternConstructor> PatternMap;


// Contributors:   Nelson Minar <(nelson@monkey.org>
//                 Igor E. Poteryaev <jah@mail.ru>
//                 Reinhard Deschler <reinhard.deschler@web.de>

/**
 * Most of the work of the {@link log4cxx::PatternLayout PatternLayout} class
 * is delegated to the PatternParser class.
 * <p>It is this class that parses conversion patterns and creates
 * a chained list of {@link PatternConverter PatternConverters}.
 *
 * 
*/
class LOG4CXX_EXPORT PatternParser {
  /**
   * Escape character for format specifier.
   */
  static const logchar ESCAPE_CHAR;

  enum {
    LITERAL_STATE = 0,
    CONVERTER_STATE = 1,
    DOT_STATE = 3,
    MIN_STATE = 4,
    MAX_STATE = 5
  };

  /**
   * Private constructor.
   */
  PatternParser();

private:
  /** Extract the converter identifier found at position i.
   *
   * After this function returns, the variable i will point to the
   * first char after the end of the converter identifier.
   *
   * If i points to a char which is not a character acceptable at the
   * start of a unicode identifier, the value null is returned.
   *
   * @param lastChar last processed character.
   * @param pattern format string.
   * @param i current index into pattern format.
   * @param convBuf buffer to receive conversion specifier.
   * @param currentLiteral literal to be output in case format specifier in unrecognized.
   * @return position in pattern after converter.
   */
  static int extractConverter(
    logchar lastChar, const LogString& pattern,
    LogString::size_type i, LogString& convBuf,
    LogString& currentLiteral);

  /**
   * Extract options.
   * @param pattern conversion pattern.
   * @param i start of options.
   * @param options array to receive extracted options
   * @return position in pattern after options.
   */
  static int extractOptions(const LogString& pattern, LogString::size_type i,
     std::vector<LogString>& options);

public:
  /**
   * Parse a format specifier.
   * @param pattern pattern to parse.
   * @param patternConverters list to receive pattern converters.
   * @param formattingInfos list to receive field specifiers corresponding to pattern converters.
   * @param rules map of stock pattern converters keyed by format specifier.
   */
  static void parse(
    const LogString& pattern,
    std::vector<PatternConverterPtr>& patternConverters,
    std::vector<FormattingInfoPtr>& formattingInfos,
    const PatternMap& rules);

private:
  /**
   * Creates a new PatternConverter.
   *
   *
   * @param converterId converterId.
   * @param currentLiteral literal to be used if converter is unrecognized or following converter
   *    if converterId contains extra characters.
   * @param rules map of stock pattern converters keyed by format specifier.
   * @param options converter options.
   * @return  converter or null.
   */
  static PatternConverterPtr createConverter(
    const LogString& converterId,
    LogString& currentLiteral,
    const PatternMap& rules,
    std::vector<LogString>& options);

  /**
   * Processes a format specifier sequence.
   *
   * @param c initial character of format specifier.
   * @param pattern conversion pattern
   * @param i current position in conversion pattern.
   * @param currentLiteral current literal.
   * @param formattingInfo current field specifier.
   * @param rules map of stock pattern converters keyed by format specifier.
   * @param patternConverters list to receive parsed pattern converter.
   * @param formattingInfos list to receive corresponding field specifier.
   * @return position after format specifier sequence.
   */
  static int finalizeConverter(
    logchar c, const LogString& pattern, int i,
    LogString& currentLiteral, const FormattingInfoPtr& formattingInfo,
    const PatternMap&  rules,
    std::vector<PatternConverterPtr>& patternConverters,
    std::vector<FormattingInfoPtr>&  formattingInfos);

    static bool isUnicodeIdentifierStart(logchar c);
    static bool isUnicodeIdentifierPart(logchar c);


};

}
}


#if defined(_MSC_VER)
#pragma warning (pop)
#endif


#endif
