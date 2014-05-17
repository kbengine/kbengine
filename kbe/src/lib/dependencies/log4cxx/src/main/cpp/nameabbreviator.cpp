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

#include <log4cxx/pattern/nameabbreviator.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/stringhelper.h>
#include <vector>
#include <limits.h>

using namespace log4cxx;
using namespace log4cxx::pattern;
using namespace log4cxx::helpers;

IMPLEMENT_LOG4CXX_OBJECT(NameAbbreviator)

NameAbbreviator::NameAbbreviator() {
}

NameAbbreviator::~NameAbbreviator() {
}

namespace log4cxx {
  namespace pattern {
  /**
   * Abbreviator that simply appends full name to buffer.
   */
class NOPAbbreviator : public NameAbbreviator {
public:
DECLARE_ABSTRACT_LOG4CXX_OBJECT(NOPAbbreviator)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(NOPAbbreviator)
     LOG4CXX_CAST_ENTRY_CHAIN(NameAbbreviator)
END_LOG4CXX_CAST_MAP()

    /**
     * Constructor.
     */
    NOPAbbreviator() {
    }

    /**
     * {@inheritDoc}
     */
    void abbreviate(LogString::size_type /* nameStart */, LogString& /* buf */) const {
    }
};


  /**
   * Abbreviator that drops starting path elements.
   */
  class MaxElementAbbreviator : public NameAbbreviator {
    /**
     * Maximum number of path elements to output.
     */
    const int count;

public:
DECLARE_ABSTRACT_LOG4CXX_OBJECT(MaxElementAbbreviator)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(MaxElementAbbreviator)
     LOG4CXX_CAST_ENTRY_CHAIN(NameAbbreviator)
END_LOG4CXX_CAST_MAP()
    /**
     * Create new instance.
     * @param count maximum number of path elements to output.
     */
    MaxElementAbbreviator(const int count1) : count(count1) {
    }

    /**
     * Abbreviate name.
     * @param buf buffer to append abbreviation.
     * @param nameStart start of name to abbreviate.
     */
    void abbreviate(LogString::size_type nameStart, LogString& buf) const {
      // We substract 1 from 'len' when assigning to 'end' to avoid out of
      // bounds exception in return r.substring(end+1, len). This can happen if
      // precision is 1 and the logger name ends with a dot.
      LogString::size_type end = buf.length() - 1;

      for (LogString::size_type i = count; i > 0; i--) {
        end = buf.rfind(0x2E /* '.' */, end - 1);

        if ((end == LogString::npos) || (end < nameStart)) {
          return;
        }
      }

      buf.erase(buf.begin() + nameStart, buf.begin() + (end + 1));
    }
  };

  /**
   * Fragment of an pattern abbreviator.
   *
   */
  class PatternAbbreviatorFragment {
    /**
     * Count of initial characters of element to output.
     */
    LogString::size_type charCount;

    /**
     *  Character used to represent dropped characters.
     * '\0' indicates no representation of dropped characters.
     */
    logchar ellipsis;

public:
    /**
     * Creates a PatternAbbreviatorFragment.
     * @param charCount number of initial characters to preserve.
     * @param ellipsis character to represent elimination of characters,
     *    '\0' if no ellipsis is desired.
     */
    PatternAbbreviatorFragment(
      const int charCount1, const logchar ellipsis1)
          : charCount(charCount1), ellipsis(ellipsis1) {
    }
    PatternAbbreviatorFragment() : charCount(0), ellipsis(0) {
    }

    PatternAbbreviatorFragment(const PatternAbbreviatorFragment& src)
        : charCount(src.charCount), ellipsis(src.ellipsis) {
    }

    PatternAbbreviatorFragment& operator=(const PatternAbbreviatorFragment& src) {
       charCount = src.charCount;
       ellipsis = src.ellipsis;
       return *this;
    }

    /**
     * Abbreviate element of name.
     * @param buf buffer to receive element.
     * @param startPos starting index of name element.
     * @return starting index of next element.
     */
    LogString::size_type abbreviate(LogString& buf, LogString::size_type startPos) const {
      LogString::size_type nextDot = buf.find(0x2E /* '.' */, startPos);

      if (nextDot != LogString::npos) {
        if ((nextDot - startPos) > charCount) {
          buf.erase(buf.begin() + (startPos + charCount), buf.begin() + nextDot);
          nextDot = startPos + charCount;

          if (ellipsis != 0x00) {
            buf.insert(nextDot, 1, ellipsis);
            nextDot++;
          }
        }

        nextDot++;
      }

      return nextDot;
    }
  };

  /**
   * Pattern abbreviator.
   *
   *
   */
  class PatternAbbreviator : public NameAbbreviator {
    /**
     * Element abbreviation patterns.
     */
    std::vector<PatternAbbreviatorFragment> fragments;

public:
DECLARE_ABSTRACT_LOG4CXX_OBJECT(PatternAbbreviator)
BEGIN_LOG4CXX_CAST_MAP()
     LOG4CXX_CAST_ENTRY(PatternAbbreviator)
     LOG4CXX_CAST_ENTRY_CHAIN(NameAbbreviator)
END_LOG4CXX_CAST_MAP()
    /**
     * Create PatternAbbreviator.
     *
     * @param fragments element abbreviation patterns.
     */
    PatternAbbreviator(const std::vector<PatternAbbreviatorFragment>& fragments1) :
        fragments(fragments1) {
      if (fragments1.size() == 0) {
        throw IllegalArgumentException(LOG4CXX_STR("fragments parameter must contain at least one element"));
      }
    }

    /**
     * Abbreviate name.
     * @param buf buffer that abbreviated name is appended.
     * @param nameStart start of name.
     */
    void abbreviate(LogString::size_type nameStart, LogString& buf) const {
      //
      //  all non-terminal patterns are executed once
      //
      LogString::size_type pos = nameStart;

      for (LogString::size_type i = 0; (i < (fragments.size() - 1)) && (pos < buf.length());
          i++) {
        pos = fragments[i].abbreviate(buf, pos);
      }

      //
      //   last pattern in executed repeatedly
      //
      PatternAbbreviatorFragment terminalFragment =
        fragments[fragments.size() - 1];

      while (pos < buf.length()) {
        pos = terminalFragment.abbreviate(buf, pos);
      }
    }
  };
}
}

IMPLEMENT_LOG4CXX_OBJECT(NOPAbbreviator)
IMPLEMENT_LOG4CXX_OBJECT(MaxElementAbbreviator)
IMPLEMENT_LOG4CXX_OBJECT(PatternAbbreviator)



NameAbbreviatorPtr NameAbbreviator::getAbbreviator(const LogString& pattern) {
    if (pattern.length() > 0) {
      //  if pattern is just spaces and numbers then
      //     use MaxElementAbbreviator
      LogString trimmed(StringHelper::trim(pattern));

      if (trimmed.length() == 0) {
        return getDefaultAbbreviator();
      }

      LogString::size_type i = 0;

      while (
        (i < trimmed.length()) && (trimmed[i] >= 0x30 /* '0' */)
          && (trimmed[i] <= 0x39 /* '9' */)) {
        i++;
      }

      //
      //  if all blanks and digits
      //
      if (i == trimmed.length()) {
        return new MaxElementAbbreviator(StringHelper::toInt(trimmed));
      }

      std::vector<PatternAbbreviatorFragment> fragments;
      logchar ellipsis;
      int charCount;
      LogString::size_type pos = 0;

      while (pos < trimmed.length()) {
        LogString::size_type ellipsisPos = pos;

        if (trimmed[pos] == 0x2A /* '*' */) {
          charCount = INT_MAX;
          ellipsisPos++;
        } else {
          if ((trimmed[pos] >= 0x30 /* '0' */)
               && (trimmed[pos] <= 0x39 /* '9' */)) {
            charCount = trimmed[pos] - 0x30 /* '0' */;
            ellipsisPos++;
          } else {
            charCount = 0;
          }
        }

        ellipsis = 0;

        if (ellipsisPos < trimmed.length()) {
          ellipsis = trimmed[ellipsisPos];

          if (ellipsis == 0x2E /* '.' */) {
            ellipsis = 0;
          }
        }

        fragments.push_back(PatternAbbreviatorFragment(charCount, ellipsis));
        pos = trimmed.find(0x2E /* '.' */, pos);

        if (pos == LogString::npos) {
          break;
        }

        pos++;
      }

      NameAbbreviatorPtr abbrev(new PatternAbbreviator(fragments));
      return abbrev;
    }

    //
    //  no matching abbreviation, return defaultAbbreviator
    //
    return getDefaultAbbreviator();
  }

  /**
   * Gets default abbreviator.
   *
   * @return default abbreviator.
   */
NameAbbreviatorPtr NameAbbreviator::getDefaultAbbreviator() {
    static NameAbbreviatorPtr def(new NOPAbbreviator());
    return def;
}

