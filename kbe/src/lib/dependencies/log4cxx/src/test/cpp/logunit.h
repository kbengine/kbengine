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

#if !defined(_LOG4CXX_LOGUNIT_H)
#define _LOG4CXX_LOGUNIT_H


#if defined(_MSC_VER)
#pragma warning (push)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include "abts.h"
#include <exception>
#include <map>
#include <string>
#include <vector>

#include <log4cxx/logstring.h>

namespace LogUnit {
    class TestException : public std::exception {
    public:
        TestException();
        TestException(const TestException&);
        TestException& operator=(const TestException&);
    };
    class AssertException : public std::exception {
    public:
        AssertException(std::string msg, int lineno);
        AssertException(bool expected, const char* actualExpr, int lineno);
        AssertException(const AssertException&);
        AssertException& operator=(const AssertException&);
        virtual ~AssertException() throw();
        std::string getMessage() const;
        int getLine() const;
    private:
        std::string msg;
        int lineno;
    };
    class TestFixture {
    public:
        TestFixture();
        virtual ~TestFixture();
        void setCase(abts_case* tc);
        virtual void setUp();
        virtual void tearDown();
        
        void assertEquals(const int expected, const int actual, int lineno);
        void assertEquals(const std::string expected, 
            const std::string actual, 
            const char* expectedExpr, 
            const char* actualExpr, 
            int lineno);
        void assertEquals(const char* expected, 
            const char* actual, 
            const char* expectedExpr, 
            const char* actualExpr, 
            int lineno);
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_WCHAR_T_API       
        void assertEquals(const std::wstring expected, 
            const std::wstring actual, 
            const char* expectedExpr, 
            const char* actualExpr, 
            int lineno);
#endif        
#if LOG4CXX_LOGCHAR_IS_UNICHAR || LOG4CXX_UNICHAR_API || LOG4CXX_CFSTRING_API
        void assertEquals(const std::basic_string<log4cxx::UniChar> expected, 
             const std::basic_string<log4cxx::UniChar> actual, 
             const char* expectedExpr, 
             const char* actualExpr, int lineno);
#endif
        template<class T>
        void assertEquals(const T& expected, 
             const T& actual, 
             const char* expectedExpr,
             const char* actualExpr,
             int lineno) {
             if (expected != actual) {
                std::string msg(expectedExpr);
                msg.append(" != ");
                msg.append(actualExpr);
                abts_fail(tc, msg.c_str(), lineno);
             }
        }
    private:
        TestFixture(const TestFixture&);
        TestFixture& operator=(const TestFixture&);
        abts_case* tc;
    };
    template<class T>
    void runTest(abts_case* tc, void (T::*func)()) {
        T ti;
        ti.setCase(tc);
        ti.setUp();
        try {
          (ti.*func)();
        } catch(TestException&) {
        } catch(AssertException& fx) {
            abts_fail(tc, fx.getMessage().c_str(), fx.getLine());
        } catch(...) {
            abts_fail(tc, "Unexpected exception", -1);
        }
        ti.tearDown();
    }
    template<class T, class X>
    void runTestWithException(abts_case* tc, void (T::*func)()) {
        T ti;
        ti.setCase(tc);
        ti.setUp();
        try {
          (ti.*func)();
        } catch(TestException&) {
        } catch(AssertException& fx) {
            abts_fail(tc, fx.getMessage().c_str(), fx.getLine());
        } catch(X&) {
        } catch(...) {
            abts_fail(tc, "Unexpected exception", -1);
        }
        ti.tearDown();
    }
    class TestSuite {
    public:
        TestSuite(const char* filename);
        void addTest(const char* testName, test_func func);
        abts_suite* run(abts_suite* suite) const;
        std::string getName() const;
        void setDisabled(bool newVal);
        bool isDisabled() const;
    private:
        TestSuite(const TestSuite&);
        TestSuite& operator=(const TestSuite&);
        typedef std::vector<test_func> TestList;
        TestList test_funcs;
        std::string filename;
        bool disabled;
    };
    typedef std::vector< std::pair<std::string, const TestSuite*> > SuiteList;
    SuiteList& getAllSuites();
    template<class T>
    class RegisterSuite {
    public:
        RegisterSuite() {
            T::populateSuite();
            TestSuite* suite = T::getSuite();
            LogUnit::getAllSuites().push_back(SuiteList::value_type(suite->getName(), suite));
        }
    };
    template<class T>
    class RegisterDisabledSuite {
    public:
        RegisterDisabledSuite() {
            T::populateSuite();
            TestSuite* suite = T::getSuite();
            suite->setDisabled(true);
            LogUnit::getAllSuites().push_back(SuiteList::value_type(suite->getName(), suite));
        }
    };
}

#define LOGUNIT_CLASS(x) class x : public LogUnit::TestFixture


#define LOGUNIT_TEST_SUITE(TF)                        \
public:                                               \
     static LogUnit::TestSuite* getSuite() {          \
        static LogUnit::TestSuite suite(__FILE__);    \
        return &suite;                                \
     }                                                \
     private:                                         \
     class RegisterSuite {                            \
     public:                                          \
        typedef TF ThisFixture;                       
        
#define LOGUNIT_TEST(testName)          \
     class testName ## Registration {   \
     public:                            \
        testName ## Registration() {    \
            ThisFixture::getSuite()->addTest(#testName, &testName ## Registration :: run); \
        }                               \
        static void run(abts_case* tc, void*) { \
            LogUnit::runTest<ThisFixture>(tc, &ThisFixture::testName); \
        }                                       \
    } register ## testName;

#define LOGUNIT_TEST_EXCEPTION(testName, Exception)  \
     class testName ## Registration {   \
     public:                            \
        testName ## Registration() {    \
            ThisFixture::getSuite()->addTest(#testName, &testName ## Registration :: run); \
        }                               \
        static void run(abts_case* tc, void*) { \
            LogUnit::runTestWithException<ThisFixture, Exception>(tc, &ThisFixture::testName); \
        }                                       \
    } register ## testName;


#define LOGUNIT_TEST_SUITE_END() \
    };                             \
    public:                        \
    static void populateSuite() {         \
        static RegisterSuite registration; \
    }
    

#define LOGUNIT_TEST_SUITE_REGISTRATION(TF) \
static LogUnit::RegisterSuite<TF> registration;

#define LOGUNIT_TEST_SUITE_REGISTRATION_DISABLED(TF) \
static LogUnit::RegisterDisabledSuite<TF> registration;

    
#define LOGUNIT_ASSERT(x) { if (!(x)) throw LogUnit::AssertException(true, #x, __LINE__); }
#define LOGUNIT_ASSERT_EQUAL(expected, actual) assertEquals(expected, actual, #expected, #actual, __LINE__)
#define LOGUNIT_FAIL(msg) throw LogUnit::AssertException(msg, __LINE__)


#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif

