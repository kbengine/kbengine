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
#include <log4cxx/helpers/stringhelper.h>
#include "../insertwide.h"
#include "../logunit.h"


using namespace log4cxx;
using namespace log4cxx::helpers;

/**
   Unit test for StringHelper.
   
   
*/
LOGUNIT_CLASS(StringHelperTestCase)
   {
     LOGUNIT_TEST_SUITE( StringHelperTestCase );
     LOGUNIT_TEST( testStartsWith1 );
     LOGUNIT_TEST( testStartsWith2 );
     LOGUNIT_TEST( testStartsWith3 );
     LOGUNIT_TEST( testStartsWith4 );
     LOGUNIT_TEST( testStartsWith5 );
     LOGUNIT_TEST( testEndsWith1 );
     LOGUNIT_TEST( testEndsWith2 );
     LOGUNIT_TEST( testEndsWith3 );
     LOGUNIT_TEST( testEndsWith4 );
     LOGUNIT_TEST( testEndsWith5 );
     LOGUNIT_TEST_SUITE_END();


public:

  /**
   * Check that startsWith("foobar", "foo") returns true.
   */
  void testStartsWith1() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::startsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("foo")));
  }

  /**
   * Check that startsWith("bar", "foobar") returns false.
   */
  void testStartsWith2() {
    LOGUNIT_ASSERT_EQUAL(false, StringHelper::startsWith(LOG4CXX_STR("foo"), LOG4CXX_STR("foobar")));
  }

  /**
   * Check that startsWith("foobar", "foobar") returns true.
   */
  void testStartsWith3() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::startsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("foobar")));
  }

  /**
   * Check that startsWith("foobar", "") returns true.
   */
  void testStartsWith4() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::startsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("")));
  }

  /**
   * Check that startsWith("foobar", "abc") returns false.
   */
  void testStartsWith5() {
    LOGUNIT_ASSERT_EQUAL(false, StringHelper::startsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("abc")));
  }



  /**
   * Check that endsWith("foobar", "bar") returns true.
   */
  void testEndsWith1() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::endsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("bar")));
  }

  /**
   * Check that endsWith("bar", "foobar") returns false.
   */
  void testEndsWith2() {
    LOGUNIT_ASSERT_EQUAL(false, StringHelper::endsWith(LOG4CXX_STR("bar"), LOG4CXX_STR("foobar")));
  }

  /**
   * Check that endsWith("foobar", "foobar") returns true.
   */
  void testEndsWith3() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::endsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("foobar")));
  }

  /**
   * Check that endsWith("foobar", "") returns true.
   */
  void testEndsWith4() {
    LOGUNIT_ASSERT_EQUAL(true, StringHelper::endsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("")));
  }

  /**
   * Check that endsWith("foobar", "abc") returns false.
   */
  void testEndsWith5() {
    LOGUNIT_ASSERT_EQUAL(false, StringHelper::startsWith(LOG4CXX_STR("foobar"), LOG4CXX_STR("abc")));
  }


};


LOGUNIT_TEST_SUITE_REGISTRATION(StringHelperTestCase);
