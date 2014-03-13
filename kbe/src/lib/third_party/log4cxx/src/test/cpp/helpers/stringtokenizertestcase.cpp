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
#include <log4cxx/helpers/stringtokenizer.h>
#include "../logunit.h"
#include "../insertwide.h"


using namespace log4cxx;
using namespace log4cxx::helpers;

LOGUNIT_CLASS(StringTokenizerTestCase)
{
   LOGUNIT_TEST_SUITE(StringTokenizerTestCase);
      LOGUNIT_TEST(testNextTokenEmptyString);
                LOGUNIT_TEST(testHasMoreTokensEmptyString);
                LOGUNIT_TEST(testNextTokenAllDelim);
                LOGUNIT_TEST(testHasMoreTokensAllDelim);
                LOGUNIT_TEST(test1);
                LOGUNIT_TEST(test2);
                LOGUNIT_TEST(test3);
                LOGUNIT_TEST(test4);
                LOGUNIT_TEST(test5);
                LOGUNIT_TEST(test6);
   LOGUNIT_TEST_SUITE_END();

public:
        void testNextTokenEmptyString() {
           LogString src;
           LogString delim(LOG4CXX_STR(" "));
           StringTokenizer tokenizer(src, delim);
           try {
             LogString token(tokenizer.nextToken());
           } catch (NoSuchElementException &ex) {
             return;
           }
           LOGUNIT_ASSERT(false);
        }

        void testHasMoreTokensEmptyString() {
           LogString src;
           LogString delim(LOG4CXX_STR(" "));
           StringTokenizer tokenizer(src, delim);
           LOGUNIT_ASSERT_EQUAL(false, tokenizer.hasMoreTokens());
        }

        void testNextTokenAllDelim() {
           LogString src(LOG4CXX_STR("==="));
           LogString delim(LOG4CXX_STR("="));
           StringTokenizer tokenizer(src, delim);
           try {
             LogString token(tokenizer.nextToken());
           } catch (NoSuchElementException &ex) {
             return;
           }
           LOGUNIT_ASSERT(false);
        }

        void testHasMoreTokensAllDelim() {
           LogString src(LOG4CXX_STR("==="));
           LogString delim(LOG4CXX_STR("="));
           StringTokenizer tokenizer(src, delim);
           LOGUNIT_ASSERT_EQUAL(false, tokenizer.hasMoreTokens());
        }

        void testBody(const LogString& src, const LogString& delim) {
           StringTokenizer tokenizer(src, delim);
           LOGUNIT_ASSERT_EQUAL(true, tokenizer.hasMoreTokens());
           LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("log4j"), tokenizer.nextToken());
           LOGUNIT_ASSERT_EQUAL(true, tokenizer.hasMoreTokens());
           LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("properties"), tokenizer.nextToken());
           LOGUNIT_ASSERT_EQUAL(true, tokenizer.hasMoreTokens());
           LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("txt"), tokenizer.nextToken());
           LOGUNIT_ASSERT_EQUAL(false, tokenizer.hasMoreTokens());
           try {
              LogString token(tokenizer.nextToken());
           } catch (NoSuchElementException& ex) {
             return;
           }
           LOGUNIT_ASSERT(false);
        }

        void test1() {
          LogString src(LOG4CXX_STR("log4j.properties.txt"));
          LogString delim(LOG4CXX_STR("."));
          testBody(src, delim);
        }

        void test2() {
          LogString src(LOG4CXX_STR(".log4j.properties.txt"));
          LogString delim(LOG4CXX_STR("."));
          testBody(src, delim);
        }

        void test3() {
          LogString src(LOG4CXX_STR("log4j.properties.txt."));
          LogString delim(LOG4CXX_STR("."));
          testBody(src, delim);
        }

        void test4() {
          LogString src(LOG4CXX_STR("log4j..properties....txt"));
          LogString delim(LOG4CXX_STR("."));
          testBody(src, delim);
        }

        void test5() {
          LogString src(LOG4CXX_STR("log4j properties,txt"));
          LogString delim(LOG4CXX_STR(" ,"));
          testBody(src, delim);
        }

        void test6() {
           LogString src(LOG4CXX_STR(" log4j properties,txt "));
           LogString delim(LOG4CXX_STR(" ,"));
           testBody(src, delim);
        }

};

LOGUNIT_TEST_SUITE_REGISTRATION(StringTokenizerTestCase);
