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


#include <log4cxx/pattern/filedatepatternconverter.h>
#include <log4cxx/pattern/integerpatternconverter.h>
#include <log4cxx/pattern/patternparser.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/date.h>
#include <log4cxx/helpers/integer.h>
#include "../util/compare.h"
#include "../logunit.h"
#include "../insertwide.h"
#include <apr_time.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::pattern;


/**
 * Tests for FileNamePattern.
 *
 * 
 * 
 *
 */
LOGUNIT_CLASS(FileNamePatternTestCase) {
  LOGUNIT_TEST_SUITE(FileNamePatternTestCase);
  LOGUNIT_TEST(testFormatInteger1);
  LOGUNIT_TEST(testFormatInteger2);
  LOGUNIT_TEST(testFormatInteger3);
  LOGUNIT_TEST(testFormatInteger4);
  LOGUNIT_TEST(testFormatInteger5);
  LOGUNIT_TEST(testFormatInteger6);
  LOGUNIT_TEST(testFormatInteger7);
  LOGUNIT_TEST(testFormatInteger8);
  LOGUNIT_TEST(testFormatInteger9);
  LOGUNIT_TEST(testFormatInteger10);
  LOGUNIT_TEST(testFormatInteger11);
  LOGUNIT_TEST(testFormatDate1);
//
//   TODO: Problem with timezone offset
//  LOGUNIT_TEST(testFormatDate2);
//  LOGUNIT_TEST(testFormatDate3);
  LOGUNIT_TEST(testFormatDate4);
  LOGUNIT_TEST(testFormatDate5);
  LOGUNIT_TEST_SUITE_END();

public:
    LogString format(const LogString& pattern,
        const ObjectPtr& obj) {
        std::vector<PatternConverterPtr> converters;
        std::vector<FormattingInfoPtr> fields;
        PatternMap rules;
        rules.insert(PatternMap::value_type(LOG4CXX_STR("d"), (PatternConstructor) FileDatePatternConverter::newInstance));
        rules.insert(PatternMap::value_type(LOG4CXX_STR("i"), (PatternConstructor) IntegerPatternConverter::newInstance));
        PatternParser::parse(pattern, converters, fields, rules);
        LogString result;
        Pool pool;
        std::vector<FormattingInfoPtr>::const_iterator fieldIter = fields.begin();
        for(std::vector<PatternConverterPtr>::const_iterator converterIter = converters.begin();
            converterIter != converters.end();
            converterIter++, fieldIter++) {
            LogString::size_type i = result.length();
            (*converterIter)->format(obj, result, pool);
            (*fieldIter)->format(i, result);
        }
        return result;
    }



    void assertDatePattern(const LogString& pattern,
        int year,
        int month,
        int day, int hour,
        int min,
        const LogString& expected) {
        apr_time_exp_t tm;
        memset(&tm, 0, sizeof(tm));
        tm.tm_min = min;
        tm.tm_hour = hour;
        tm.tm_mday = day;
        tm.tm_mon = month;
        tm.tm_year = year - 1900;
        apr_time_t n;
        /*apr_status_t stat = */apr_time_exp_get(&n, &tm);
        ObjectPtr obj(new Date(n));
        LOGUNIT_ASSERT_EQUAL(expected, format(pattern, obj));
    }

    void assertIntegerPattern(const LogString& pattern, int value,
        const LogString& expected) {
        ObjectPtr obj(new Integer(value));
        LOGUNIT_ASSERT_EQUAL(expected, format(pattern, obj));
    }

    void testFormatInteger1() {
        assertIntegerPattern(LOG4CXX_STR("t"),  3, LOG4CXX_STR("t"));
    }

    void testFormatInteger2() {
        assertIntegerPattern(LOG4CXX_STR("foo"),  3, LOG4CXX_STR("foo"));
    }

    void testFormatInteger3() {
        assertIntegerPattern(LOG4CXX_STR("foo%"),  3, LOG4CXX_STR("foo%"));
    }

    void testFormatInteger4() {
        assertIntegerPattern(LOG4CXX_STR("%ifoo"),  3, LOG4CXX_STR("3foo"));
    }

    void testFormatInteger5() {
        assertIntegerPattern(LOG4CXX_STR("foo%ixixo"),  3, LOG4CXX_STR("foo3xixo"));
    }

    void testFormatInteger6() {
        assertIntegerPattern(LOG4CXX_STR("foo%i.log"),  3, LOG4CXX_STR("foo3.log"));
    }

    void testFormatInteger7() {
        assertIntegerPattern(LOG4CXX_STR("foo.%i.log"),  3, LOG4CXX_STR("foo.3.log"));
    }

    void testFormatInteger8() {
        assertIntegerPattern(LOG4CXX_STR("%ifoo%"),  3, LOG4CXX_STR("3foo%"));
    }

    void testFormatInteger9() {
        assertIntegerPattern(LOG4CXX_STR("%ifoo%%"),  3, LOG4CXX_STR("3foo%"));
    }

    void testFormatInteger10() {
        assertIntegerPattern(LOG4CXX_STR("%%foo"),  3, LOG4CXX_STR("%foo"));
    }

    void testFormatInteger11() {
        assertIntegerPattern(LOG4CXX_STR("foo%ibar%i"),  3, LOG4CXX_STR("foo3bar3"));
    }

    void testFormatDate1() {
        assertDatePattern(LOG4CXX_STR("foo%d{yyyy.MM.dd}"),  2003, 4, 20, 17, 55,
            LOG4CXX_STR("foo2003.05.20"));
    }

    void testFormatDate2() {
        assertDatePattern(LOG4CXX_STR("foo%d{yyyy.MM.dd HH:mm}"),  2003, 4, 20, 17, 55,
            LOG4CXX_STR("foo2003.05.20 17:55"));
    }

    void testFormatDate3() {
        assertDatePattern(LOG4CXX_STR("%d{yyyy.MM.dd HH:mm} foo"),  2003, 4, 20, 17, 55,
            LOG4CXX_STR("2003.05.20 17:55 foo"));
    }

    void testFormatDate4() {
        assertDatePattern(LOG4CXX_STR("foo%dyyyy.MM.dd}"),  2003, 4, 20, 17, 55,
            LOG4CXX_STR("foo2003-05-20yyyy.MM.dd}"));
    }

    void testFormatDate5() {
        assertDatePattern(LOG4CXX_STR("foo%d{yyyy.MM.dd"),  2003, 4, 20, 17, 55,
            LOG4CXX_STR("foo2003-05-20{yyyy.MM.dd"));
    }

};

//
//   See bug LOGCXX-204
//
#if !defined(_MSC_VER) || _MSC_VER > 1200
LOGUNIT_TEST_SUITE_REGISTRATION(FileNamePatternTestCase);
#endif
