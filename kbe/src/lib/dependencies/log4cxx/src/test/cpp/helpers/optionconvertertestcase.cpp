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

#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/properties.h>
#include <log4cxx/helpers/system.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/pool.h>
#include "../testchar.h"
#include "../insertwide.h"
#include "../logunit.h"
#include <stdlib.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#include <apr_user.h>
#include <apr_env.h>


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

#define MAX 1000

LOGUNIT_CLASS(OptionConverterTestCase)
{
   LOGUNIT_TEST_SUITE(OptionConverterTestCase);
      LOGUNIT_TEST(varSubstTest1);
      LOGUNIT_TEST(varSubstTest2);
      LOGUNIT_TEST(varSubstTest3);
      LOGUNIT_TEST(varSubstTest4);
      LOGUNIT_TEST(varSubstTest5);
      LOGUNIT_TEST(testTmpDir);
#if APR_HAS_USER
      LOGUNIT_TEST(testUserHome);
      LOGUNIT_TEST(testUserName);
#endif
      LOGUNIT_TEST(testUserDir);
   LOGUNIT_TEST_SUITE_END();

   Properties props;
   Properties nullProperties;

public:
   void setUp()
   {
   }

   void tearDown()
   {
   }

   /**
   * Checks that environment variables were properly set
   * before invoking tests.  ::putenv not reliable.
   */
   void envCheck() {
     Pool p;
     char* toto;
     apr_status_t stat = apr_env_get(&toto, "TOTO", 
         p.getAPRPool());
     LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
     LOGUNIT_ASSERT_EQUAL("wonderful", toto);
     char* key1;
     stat = apr_env_get(&key1, "key1", 
         p.getAPRPool());
     LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
     LOGUNIT_ASSERT_EQUAL("value1", key1);
     char* key2;
     stat = apr_env_get(&key2, "key2", 
         p.getAPRPool());
     LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
     LOGUNIT_ASSERT_EQUAL("value2", key2);
   }

   void varSubstTest1()
   {
      envCheck();
      LogString r(OptionConverter::substVars(LOG4CXX_STR("hello world."), nullProperties));
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("hello world."), r);

      r = OptionConverter::substVars(LOG4CXX_STR("hello ${TOTO} world."), nullProperties);

      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("hello wonderful world."), r);
   }


   void varSubstTest2()
   {
     envCheck();
      LogString r(OptionConverter::substVars(LOG4CXX_STR("Test2 ${key1} mid ${key2} end."),
         nullProperties));
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("Test2 value1 mid value2 end."), r);
   }


   void varSubstTest3()
   {
     envCheck();
      LogString r(OptionConverter::substVars(
         LOG4CXX_STR("Test3 ${unset} mid ${key1} end."), nullProperties));
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("Test3  mid value1 end."), r);
   }


   void varSubstTest4()
   {
      LogString res;
      LogString val(LOG4CXX_STR("Test4 ${incomplete "));
      try
      {
         res = OptionConverter::substVars(val, nullProperties);
      }
      catch(IllegalArgumentException& e)
      {
         std::string witness("\"Test4 ${incomplete \" has no closing brace. Opening brace at position 6.");
         LOGUNIT_ASSERT_EQUAL(witness, (std::string) e.what());
      }
   }


   void varSubstTest5()
   {
      Properties props1;
      props1.setProperty(LOG4CXX_STR("p1"), LOG4CXX_STR("x1"));
      props1.setProperty(LOG4CXX_STR("p2"), LOG4CXX_STR("${p1}"));
      LogString res = OptionConverter::substVars(LOG4CXX_STR("${p2}"), props1);
      LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("x1"), res);
   }

    void testTmpDir()
    {
       LogString actual(OptionConverter::substVars(
          LOG4CXX_STR("${java.io.tmpdir}"), nullProperties));
       Pool p;
       const char* tmpdir = NULL;
       apr_status_t stat = apr_temp_dir_get(&tmpdir, p.getAPRPool());
       LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
       LogString expected;
       Transcoder::decode(tmpdir, expected);

       LOGUNIT_ASSERT_EQUAL(expected, actual);
    }

#if APR_HAS_USER
    void testUserHome() {
      LogString actual(OptionConverter::substVars(
         LOG4CXX_STR("${user.home}"), nullProperties));
      Pool p;

      apr_uid_t userid;
      apr_gid_t groupid;
      apr_status_t stat = apr_uid_current(&userid, &groupid, p.getAPRPool());
      if (stat == APR_SUCCESS) {
         char* username = NULL;
         stat = apr_uid_name_get(&username, userid, p.getAPRPool());
         if (stat == APR_SUCCESS) {
            char* dirname = NULL;
            stat = apr_uid_homepath_get(&dirname, username, p.getAPRPool());
            if (stat == APR_SUCCESS) {
               LogString expected;
               Transcoder::decode(dirname, expected);
               LOGUNIT_ASSERT_EQUAL(expected, actual);
             }
          }
      }   

    }

    void testUserName() {
       LogString actual(OptionConverter::substVars(
           LOG4CXX_STR("${user.name}"), nullProperties));
       Pool p;
       apr_uid_t userid;
       apr_gid_t groupid;
       apr_status_t stat = apr_uid_current(&userid, &groupid, p.getAPRPool());
       if (stat == APR_SUCCESS) {
           char* username = NULL;
          stat = apr_uid_name_get(&username, userid, p.getAPRPool());
          if (stat == APR_SUCCESS) {
             LogString expected;
             Transcoder::decode(username, expected);
             LOGUNIT_ASSERT_EQUAL(expected, actual);
          }
       }
   }
#endif

    void testUserDir() {
      LogString actual(OptionConverter::substVars(
          LOG4CXX_STR("${user.dir}"), nullProperties));
      Pool p;

      char* dirname = NULL;
      apr_status_t stat = apr_filepath_get(&dirname, APR_FILEPATH_NATIVE, p.getAPRPool());
      LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);

      LogString expected;
      Transcoder::decode(dirname, expected);

      LOGUNIT_ASSERT_EQUAL(expected, actual);
    }
};

LOGUNIT_TEST_SUITE_REGISTRATION(OptionConverterTestCase);
