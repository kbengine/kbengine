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

#include <log4cxx/logstring.h>
#include <log4cxx/pattern/patternparser.h>
#include <log4cxx/pattern/literalpatternconverter.h>
#include <log4cxx/helpers/loglog.h>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::helpers;

const logchar PatternParser::ESCAPE_CHAR = 0x25; // '%'


/**
 * Private constructor.
 */
PatternParser::PatternParser() {
}

bool PatternParser::isUnicodeIdentifierStart(logchar ch) {
  //
  //   greatly simplified version checks if
  //     character is USACII alpha or number
  //
  return (ch >= 0x41 /* 'A' */ && ch <= 0x5A /* 'Z' */) ||
         (ch >= 0x61 /* 'a' */ && ch <= 0x7A /* 'z' */) ||
         (ch >= 0x30 /* '0' */ && ch <= 0x39 /* '9' */);
}

bool PatternParser::isUnicodeIdentifierPart(logchar ch) {
  //
  //   greatly simplified version checks if
  //     character is USACII alpha or number
  //
  return isUnicodeIdentifierStart(ch)
         || (ch == 0x5F /* '_' */);
}

int PatternParser::extractConverter(
  logchar lastChar, const LogString& pattern,
  LogString::size_type i, LogString& convBuf,
  LogString& currentLiteral) {
  if (!convBuf.empty()) {
    convBuf.erase(convBuf.begin(), convBuf.end());
  }

  // When this method is called, lastChar points to the first character of the
  // conversion word. For example:
  // For "%hello"     lastChar = 'h'
  // For "%-5hello"   lastChar = 'h'
  //System.out.println("lastchar is "+lastChar);
  if (!isUnicodeIdentifierStart(lastChar)) {
    return i;
  }

  convBuf.append(1, lastChar);

  while (
    (i < pattern.length())
      && isUnicodeIdentifierPart(pattern[i])) {
    convBuf.append(1, pattern[i]);
    currentLiteral.append(1, pattern[i]);

    //System.out.println("conv buffer is now ["+convBuf+"].");
    i++;
  }

  return i;
}


int PatternParser::extractOptions(const LogString& pattern, LogString::size_type i,
   std::vector<LogString>& options) {
  while ((i < pattern.length()) && (pattern[i] == 0x7B /* '{' */)) {
    int end = pattern.find(0x7D /* '}' */, i);

    if (end == -1) {
      break;
    }

    LogString r(pattern.substr(i + 1, end - i - 1));
    options.push_back(r);
    i = end + 1;
  }

  return i;
}

void PatternParser::parse(
  const LogString& pattern,
  std::vector<PatternConverterPtr>& patternConverters,
  std::vector<FormattingInfoPtr>& formattingInfos,
  const PatternMap& rules) {

  LogString currentLiteral;

  int patternLength = pattern.length();
  int state = LITERAL_STATE;
  logchar c;
  int i = 0;
  FormattingInfoPtr formattingInfo(FormattingInfo::getDefault());

  while (i < patternLength) {
    c = pattern[i++];

    switch (state) {
    case LITERAL_STATE:

      // In literal state, the last char is always a literal.
      if (i == patternLength) {
        currentLiteral.append(1, c);

        continue;
      }

      if (c == ESCAPE_CHAR) {
        // peek at the next char.
        if(pattern[i] == ESCAPE_CHAR) {
          currentLiteral.append(1, c);
          i++; // move pointer
        } else {
          if (!currentLiteral.empty()) {
            patternConverters.push_back(
              LiteralPatternConverter::newInstance(currentLiteral));
            formattingInfos.push_back(FormattingInfo::getDefault());
            currentLiteral.erase(currentLiteral.begin(), currentLiteral.end());
          }

          currentLiteral.append(1, c); // append %
          state = CONVERTER_STATE;
          formattingInfo = FormattingInfo::getDefault();
        }
      } else {
        currentLiteral.append(1, c);
      }

      break;

    case CONVERTER_STATE:
      currentLiteral.append(1, c);

      switch (c) {
      case 0x2D: // '-'
        formattingInfo =
          new FormattingInfo(
            true, formattingInfo->getMinLength(),
            formattingInfo->getMaxLength());

        break;

      case 0x2E: // '.'
        state = DOT_STATE;

        break;

      default:

        if ((c >= 0x30 /* '0' */) && (c <= 0x39 /* '9' */)) {
          formattingInfo =
            new FormattingInfo(
              formattingInfo->isLeftAligned(), c - 0x30 /* '0' */,
              formattingInfo->getMaxLength());
          state = MIN_STATE;
        } else {
          i = finalizeConverter(
              c, pattern, i, currentLiteral, formattingInfo,
              rules, patternConverters, formattingInfos);

          // Next pattern is assumed to be a literal.
          state = LITERAL_STATE;
          formattingInfo = FormattingInfo::getDefault();
          if (!currentLiteral.empty()) {
            currentLiteral.erase(currentLiteral.begin(), currentLiteral.end());
          }
        }
      } // switch

      break;

    case MIN_STATE:
      currentLiteral.append(1, c);

      if ((c >= 0x30 /* '0' */) && (c <= 0x39 /* '9' */)) {
        formattingInfo =
          new FormattingInfo(
            formattingInfo->isLeftAligned(),
            (formattingInfo->getMinLength() * 10) + (c - 0x30 /* '0' */),
            formattingInfo->getMaxLength());
      } else if (c == 0x2E /* '.' */) {
        state = DOT_STATE;
      } else {
        i = finalizeConverter(
            c, pattern, i, currentLiteral, formattingInfo,
            rules, patternConverters, formattingInfos);
        state = LITERAL_STATE;
        formattingInfo = FormattingInfo::getDefault();
        if (!currentLiteral.empty()) {
            currentLiteral.erase(currentLiteral.begin(), currentLiteral.end());
        }
      }

      break;

    case DOT_STATE:
      currentLiteral.append(1, c);

      if ((c >= 0x30 /* '0' */) && (c <= 0x39 /* '9' */)) {
        formattingInfo =
          new FormattingInfo(
            formattingInfo->isLeftAligned(), formattingInfo->getMinLength(),
            c - 0x30 /* '0' */);
        state = MAX_STATE;
      } else {
          LogLog::error(LOG4CXX_STR("Error in pattern, was expecting digit."));

        state = LITERAL_STATE;
      }

      break;

    case MAX_STATE:
      currentLiteral.append(1, c);

      if ((c >= 0x30 /* '0' */) && (c <= 0x39 /* '9' */)) {
        formattingInfo =
          new FormattingInfo(
            formattingInfo->isLeftAligned(), formattingInfo->getMinLength(),
            (formattingInfo->getMaxLength() * 10) + (c - 0x30 /* '0' */));
      } else {
        i = finalizeConverter(
            c, pattern, i, currentLiteral, formattingInfo,
            rules, patternConverters, formattingInfos);
        state = LITERAL_STATE;
        formattingInfo = FormattingInfo::getDefault();
        if (!currentLiteral.empty()) {
            currentLiteral.erase(currentLiteral.begin(), currentLiteral.end());
        }
      }

      break;
    } // switch
  }

  // while
  if (currentLiteral.length() != 0) {
    patternConverters.push_back(
      LiteralPatternConverter::newInstance(currentLiteral));
    formattingInfos.push_back(FormattingInfo::getDefault());
  }
}


PatternConverterPtr PatternParser::createConverter(
  const LogString& converterId,
  LogString& currentLiteral,
  const PatternMap& rules,
  std::vector<LogString>& options) {

  LogString converterName(converterId);

  for (int i = converterId.length(); i > 0; i--) {
    converterName = converterName.substr(0, i);
    PatternMap::const_iterator iter = rules.find(converterName);
    if (iter != rules.end()) {
      currentLiteral.erase(currentLiteral.begin(),
          currentLiteral.end() - (converterId.length() - i));
      return (iter->second)(options);
    }
  }

  LogLog::error(LogString(LOG4CXX_STR("Unrecognized format specifier ")) + converterId);
  ObjectPtr converterObj;

  return converterObj;
}

int PatternParser::finalizeConverter(
  logchar c, const LogString& pattern, int i,
  LogString& currentLiteral, const FormattingInfoPtr& formattingInfo,
  const PatternMap&  rules,
  std::vector<PatternConverterPtr>& patternConverters,
  std::vector<FormattingInfoPtr>&  formattingInfos) {
  LogString convBuf;
  i = extractConverter(c, pattern, i, convBuf, currentLiteral);
  if (convBuf.empty()) {
     LogLog::error(LOG4CXX_STR("Empty conversion specifier"));
     patternConverters.push_back(
         LiteralPatternConverter::newInstance(currentLiteral));
     formattingInfos.push_back(FormattingInfo::getDefault());
  } else {
     LogString converterId(convBuf);

     std::vector<LogString> options;
     i = extractOptions(pattern, i, options);

     PatternConverterPtr pc(
        createConverter(
            converterId, currentLiteral, rules, options));

     if (pc == NULL) {
        LogString msg(LOG4CXX_STR("Unrecognized conversion specifier ["));
        msg.append(converterId);
        msg.append(LOG4CXX_STR("] in conversion pattern."));
        LogLog::error(msg);
        patternConverters.push_back(
           LiteralPatternConverter::newInstance(currentLiteral));
        formattingInfos.push_back(FormattingInfo::getDefault());
     } else {
        patternConverters.push_back(pc);
        formattingInfos.push_back(formattingInfo);

        if (currentLiteral.length() > 0) {
           patternConverters.push_back(
              LiteralPatternConverter::newInstance(currentLiteral));
           formattingInfos.push_back(FormattingInfo::getDefault());
        }
     }
  }

  if (!currentLiteral.empty()) {
    currentLiteral.erase(currentLiteral.begin(), currentLiteral.end());
  }

  return i;
}
