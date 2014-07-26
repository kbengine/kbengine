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
 
#include <log4cxx/helpers/messagebuffer.h>
#include <iomanip>
#include "../insertwide.h"
#include "../logunit.h"
#include <log4cxx/logstring.h>

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;

/**
 *  Test MessageBuffer.
 */
LOGUNIT_CLASS(MessageBufferTest)
{
   LOGUNIT_TEST_SUITE(MessageBufferTest);
      LOGUNIT_TEST(testInsertChar);
      LOGUNIT_TEST(testInsertConstStr);
      LOGUNIT_TEST(testInsertStr);
      LOGUNIT_TEST(testInsertString);
      LOGUNIT_TEST(testInsertNull);
      LOGUNIT_TEST(testInsertInt);
      LOGUNIT_TEST(testInsertManipulator);
#if LOG4CXX_WCHAR_T_API
      LOGUNIT_TEST(testInsertConstWStr);
      LOGUNIT_TEST(testInsertWString);
      LOGUNIT_TEST(testInsertWStr);
#endif
#if LOG4CXX_UNICHAR_API
      LOGUNIT_TEST(testInsertConstUStr);
      LOGUNIT_TEST(testInsertUString);
#endif
#if LOG4CXX_CFSTRING_API
      LOGUNIT_TEST(testInsertCFString);
#endif
   LOGUNIT_TEST_SUITE_END();


public:
    void testInsertChar() {
        MessageBuffer buf;
        std::string greeting("Hello, World");
        CharMessageBuffer& retval = buf << "Hello, Worl" << 'd';
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertConstStr() {
        MessageBuffer buf;
        std::string greeting("Hello, World");
        CharMessageBuffer& retval = buf << "Hello" << ", World";
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertStr() {
        MessageBuffer buf;
        std::string greeting("Hello, World");
   char* part1 = (char*) malloc(10*sizeof(wchar_t));
   strcpy(part1, "Hello");
   char* part2 = (char*) malloc(10*sizeof(wchar_t));
   strcpy(part2, ", World");
        CharMessageBuffer& retval = buf << part1 << part2;
   free(part1);
   free(part2);
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertString() {
        MessageBuffer buf;
        std::string greeting("Hello, World");
        CharMessageBuffer& retval = buf << std::string("Hello") << std::string(", World");
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }
    
    void testInsertNull() {
        MessageBuffer buf;
        std::string greeting("Hello, null");
        CharMessageBuffer& retval = buf << "Hello, " << (const char*) 0;
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }
    
    void testInsertInt() {
        MessageBuffer buf;
        std::string greeting("Hello, 5");
        std::ostream& retval = buf << "Hello, " << 5;
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval));
        LOGUNIT_ASSERT_EQUAL(true, buf.hasStream());
    }
        
    void testInsertManipulator() {
        MessageBuffer buf;
        std::string greeting("pi=3.142");
        std::ostream& retval = buf << "pi=" << std::setprecision(4) << 3.1415926;
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval));
        LOGUNIT_ASSERT_EQUAL(true, buf.hasStream());
    }

#if LOG4CXX_WCHAR_T_API
    void testInsertConstWStr() {
        MessageBuffer buf;
        std::wstring greeting(L"Hello, World");
        WideMessageBuffer& retval = buf << L"Hello" << L", World";
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertWString() {
        MessageBuffer buf;
        std::wstring greeting(L"Hello, World");
        WideMessageBuffer& retval = buf << std::wstring(L"Hello") << std::wstring(L", World");
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertWStr() {
        MessageBuffer buf;
        std::wstring greeting(L"Hello, World");
       wchar_t* part1 = (wchar_t*) malloc(10*sizeof(wchar_t));
       wcscpy(part1, L"Hello");
       wchar_t* part2 = (wchar_t*) malloc(10*sizeof(wchar_t));
       wcscpy(part2, L", World");
        WideMessageBuffer& retval = buf << part1 << part2;
       free(part1);
       free(part2);
        LOGUNIT_ASSERT_EQUAL(greeting, buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }
#endif

#if LOG4CXX_UNICHAR_API
    void testInsertConstUStr() {
        MessageBuffer buf;
        const log4cxx::UniChar hello[] = { 'H', 'e', 'l', 'l', 'o', 0 };
        const log4cxx::UniChar world[] = { ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
        const log4cxx::UniChar greeting[] = { 'H', 'e', 'l', 'l', 'o', 
                                  ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
        UniCharMessageBuffer& retval = buf << hello << world;
        LOGUNIT_ASSERT_EQUAL(std::basic_string<log4cxx::UniChar>(greeting), buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

    void testInsertUString() {
        MessageBuffer buf;
        const log4cxx::UniChar hello[] = { 'H', 'e', 'l', 'l', 'o', 0 };
        const log4cxx::UniChar world[] = { ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
        const log4cxx::UniChar greeting[] = { 'H', 'e', 'l', 'l', 'o', 
                                  ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
        UniCharMessageBuffer& retval = buf << std::basic_string<log4cxx::UniChar>(hello) 
                                           << std::basic_string<log4cxx::UniChar>(world);
        LOGUNIT_ASSERT_EQUAL(std::basic_string<log4cxx::UniChar>(greeting), buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

#endif

#if LOG4CXX_CFSTRING_API
    void testInsertCFString() {
        MessageBuffer buf;
        const log4cxx::UniChar greeting[] = { 'H', 'e', 'l', 'l', 'o', 
                                  ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
        UniCharMessageBuffer& retval = buf << CFSTR("Hello") 
                                           << CFSTR(", World");
        LOGUNIT_ASSERT_EQUAL(std::basic_string<log4cxx::UniChar>(greeting), buf.str(retval)); 
        LOGUNIT_ASSERT_EQUAL(false, buf.hasStream());
    }

#endif

};

LOGUNIT_TEST_SUITE_REGISTRATION(MessageBufferTest);

