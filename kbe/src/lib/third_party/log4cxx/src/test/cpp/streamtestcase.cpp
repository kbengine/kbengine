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
#include <ostream>
#include <iomanip>

#include "vectorappender.h"
#include <log4cxx/logmanager.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/spi/loggingevent.h>
#include "insertwide.h"
#include "logunit.h"
#include <log4cxx/stream.h>

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace std;

class ExceptionOnInsert {
public:
   ExceptionOnInsert() {
   }
};

//
//   define an insertion operation that will throw an
//       exception to test that evaluation was short
//       circuited
//
template<class Elem, class Tr>
::std::basic_ostream<Elem, Tr>& operator<<(
   ::std::basic_ostream<Elem, Tr>&,
   const ExceptionOnInsert&) {
   throw std::exception();
}


/**
   Unit tests for the optional stream-like interface for log4cxx
 */
LOGUNIT_CLASS(StreamTestCase)
{
        LOGUNIT_TEST_SUITE(StreamTestCase);
                LOGUNIT_TEST(testSimple);
                LOGUNIT_TEST(testMultiple);
                LOGUNIT_TEST(testShortCircuit);
                LOGUNIT_TEST_EXCEPTION(testInsertException, std::exception);
                LOGUNIT_TEST(testScientific);
                LOGUNIT_TEST(testPrecision);
                LOGUNIT_TEST(testWidth);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(testWide);
                LOGUNIT_TEST(testWideAppend);
                LOGUNIT_TEST(testWideWidth);
#endif
                LOGUNIT_TEST(testBaseFlags);
                LOGUNIT_TEST(testBasePrecisionAndWidth);
                LOGUNIT_TEST(testLogStreamSimple);
                LOGUNIT_TEST(testLogStreamMultiple);
                LOGUNIT_TEST(testLogStreamShortCircuit);
                LOGUNIT_TEST_EXCEPTION(testLogStreamInsertException, std::exception);
                LOGUNIT_TEST(testLogStreamScientific);
                LOGUNIT_TEST(testLogStreamPrecision);
                LOGUNIT_TEST(testLogStreamWidth);
                LOGUNIT_TEST(testLogStreamDelegate);
                LOGUNIT_TEST(testLogStreamFormattingPersists);
                LOGUNIT_TEST(testSetWidthInsert);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(testWLogStreamSimple);
                LOGUNIT_TEST(testWLogStreamMultiple);
                LOGUNIT_TEST(testWLogStreamShortCircuit);
                LOGUNIT_TEST_EXCEPTION(testWLogStreamInsertException, std::exception);
                LOGUNIT_TEST(testWLogStreamScientific);
                LOGUNIT_TEST(testWLogStreamPrecision);
                LOGUNIT_TEST(testWLogStreamWidth);
                LOGUNIT_TEST(testWLogStreamDelegate);
                LOGUNIT_TEST(testWLogStreamFormattingPersists);
                LOGUNIT_TEST(testWSetWidthInsert);
#endif
#if LOG4CXX_UNICHAR_API
                LOGUNIT_TEST(testUniChar);
                LOGUNIT_TEST(testUniCharAppend);
//                LOGUNIT_TEST(testUniCharWidth);
                LOGUNIT_TEST(testULogStreamSimple);
                LOGUNIT_TEST(testULogStreamMultiple);
                LOGUNIT_TEST(testULogStreamShortCircuit);
                LOGUNIT_TEST_EXCEPTION(testULogStreamInsertException, std::exception);
//                LOGUNIT_TEST(testULogStreamScientific);
//                LOGUNIT_TEST(testULogStreamPrecision);
//                LOGUNIT_TEST(testULogStreamWidth);
                LOGUNIT_TEST(testULogStreamDelegate);
//                LOGUNIT_TEST(testULogStreamFormattingPersists);
//                LOGUNIT_TEST(testUSetWidthInsert);
#endif
#if LOG4CXX_CFSTRING_API
                LOGUNIT_TEST(testCFString);
                LOGUNIT_TEST(testCFStringAppend);
                LOGUNIT_TEST(testULogStreamCFString);
                LOGUNIT_TEST(testULogStreamCFString2);
#endif
         LOGUNIT_TEST_SUITE_END();

        VectorAppenderPtr vectorAppender;

public:
        void setUp() {
           LoggerPtr root(Logger::getRootLogger());
           LayoutPtr layout(new SimpleLayout());
           vectorAppender = new VectorAppender();
           root->addAppender(vectorAppender);
        }

        void tearDown()
        {
            LogManager::shutdown();
        }

        void testSimple() {
            LoggerPtr root(Logger::getRootLogger());
            LOG4CXX_INFO(root, "This is a test");
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testMultiple() {
           LoggerPtr root(Logger::getRootLogger());
           LOG4CXX_INFO(root, "This is a test" << ": Details to follow");
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }

       void testShortCircuit() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.shortCircuit"));
         logger->setLevel(Level::getInfo());
         ExceptionOnInsert someObj;
         LOG4CXX_DEBUG(logger, someObj);
         LOGUNIT_ASSERT_EQUAL((size_t) 0, vectorAppender->getVector().size());
       }

       void testInsertException() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.insertException"));
         ExceptionOnInsert someObj;
         LOG4CXX_INFO(logger, someObj);
       }

       void testScientific() {
           LoggerPtr root(Logger::getRootLogger());
           LOG4CXX_INFO(root, std::scientific << 0.000001115);
           spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
           LogString msg(event->getMessage());
           LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("e-")) != LogString::npos ||
                msg.find(LOG4CXX_STR("E-")) != LogString::npos);
       }

       void testPrecision() {
          LoggerPtr root(Logger::getRootLogger());
          LOG4CXX_INFO(root, std::setprecision(4) << 1.000001);
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("1.00000")) == LogString::npos);
      }


      void testWidth() {
          LoggerPtr root(Logger::getRootLogger());
          LOG4CXX_INFO(root, '[' << std::fixed << std::setprecision(2) << std::setw(7) << std::right << std::setfill('_') << 10.0 << ']');
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }

       void testBaseFlags() {
           logstream base1(Logger::getRootLogger(), Level::getInfo());
           logstream base2(Logger::getRootLogger(), Level::getInfo());
           base1 << std::boolalpha;
           base2 << std::noboolalpha;
           std::ostringstream os1a, os1b, os2a, os2b;
           os1a << std::boolalpha;
           int fillchar;
           if (base1.set_stream_state(os1b, fillchar)) {
               os1b.fill(fillchar);
            }
           LOGUNIT_ASSERT_EQUAL(os1a.flags(), os1b.flags());
           os2a << std::noboolalpha;
           if (base2.set_stream_state(os2b, fillchar)) {
               os2b.fill(fillchar);
            }
           LOGUNIT_ASSERT_EQUAL(os2a.flags(), os2b.flags());
       }


       void testBasePrecisionAndWidth() {
           logstream base(Logger::getRootLogger(), Level::getInfo());
           base.precision(2);
           base.width(5);
           std::ostringstream os1, os2;
           os1.precision(2);
           os1.width(5);
           os1 << 3.1415926;
           int fillchar;
           if (base.set_stream_state(os2, fillchar)) {
               os2.fill(fillchar);
            }
           os2 << 3.1415926;
           string expected(os1.str());
           string actual(os2.str());
           LOGUNIT_ASSERT_EQUAL(expected, actual);
       }
       
        void testLogStreamSimple() {
            logstream root(Logger::getRootLogger(), Level::getInfo());
            root << "This is a test" << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testLogStreamMultiple() {
           logstream root(Logger::getRootLogger(), Level::getInfo());
           root << "This is a test" << ": Details to follow" << LOG4CXX_ENDMSG;
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }

       void testLogStreamShortCircuit() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.shortCircuit"));
         logger->setLevel(Level::getInfo());
         logstream os(logger, Level::getDebug());
         ExceptionOnInsert someObj;
         os << someObj << LOG4CXX_ENDMSG;
         LOGUNIT_ASSERT_EQUAL((size_t) 0, vectorAppender->getVector().size());
       }

       void testLogStreamInsertException() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.insertException"));
         ExceptionOnInsert someObj;
         logstream os(logger, Level::getInfo());
         os << someObj << LOG4CXX_ENDMSG;
       }

       void testLogStreamScientific() {
           LoggerPtr root(Logger::getRootLogger());
           logstream os(root, Level::getInfo());
           os << std::scientific << 0.000001115 << LOG4CXX_ENDMSG;
           spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
           LogString msg(event->getMessage());
           LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("e-")) != LogString::npos ||
                msg.find(LOG4CXX_STR("E-")) != LogString::npos);
       }

       void testLogStreamPrecision() {
          LoggerPtr root(Logger::getRootLogger());
          logstream os(root, Level::getInfo());
          os << std::setprecision(4) << 1.000001 << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("1.00000")) == LogString::npos);
      }


      void testLogStreamWidth() {
          LoggerPtr root(Logger::getRootLogger());
          logstream os(root, Level::getInfo());
          os << '[' << std::fixed << std::setprecision(2) << std::setw(7) << std::right << std::setfill('_') << 10.0 << ']' << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }
       
       void report(std::ostream& os) {
          os << "This just in: \n";
          os << "Use logstream in places that expect a std::ostream.\n";
       }
       
        void testLogStreamDelegate() {
            logstream root(Logger::getRootLogger(), Level::getInfo());
            report(root);
            root << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }
        
        void testLogStreamFormattingPersists() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          logstream os(root, Level::getDebug());
          os << std::hex << 20 << LOG4CXX_ENDMSG;
          os << Level::getInfo() << 16 << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("10")), msg);
        }

        void testSetWidthInsert() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          logstream os(root, Level::getInfo());
          os << std::setw(5);
          LOGUNIT_ASSERT_EQUAL(5, os.width());
        }
        
        

#if LOG4CXX_WCHAR_T_API
        void testWide() {
            LoggerPtr root(Logger::getRootLogger());
            LOG4CXX_INFO(root, L"This is a test");
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testWideAppend() {
           LoggerPtr root(Logger::getRootLogger());
           LOG4CXX_INFO(root, L"This is a test" << L": Details to follow");
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }
       
      void testWideWidth() {
          LoggerPtr root(Logger::getRootLogger());
          LOG4CXX_INFO(root, L'[' << std::fixed << std::setprecision(2) << std::setw(7) << std::right << std::setfill(L'_') << 10.0 << L"]");
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }

        void testWLogStreamSimple() {
            wlogstream root(Logger::getRootLogger(), Level::getInfo());
            root << L"This is a test" << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testWLogStreamMultiple() {
           wlogstream root(Logger::getRootLogger(), Level::getInfo());
           root << L"This is a test" << L": Details to follow" << LOG4CXX_ENDMSG;
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }

       void testWLogStreamShortCircuit() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.shortCircuit"));
         logger->setLevel(Level::getInfo());
         wlogstream os(logger, Level::getDebug());
         ExceptionOnInsert someObj;
         os << someObj << LOG4CXX_ENDMSG;
         LOGUNIT_ASSERT_EQUAL((size_t) 0, vectorAppender->getVector().size());
       }

       void testWLogStreamInsertException() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.insertException"));
         ExceptionOnInsert someObj;
         wlogstream os(logger, Level::getInfo());
         os << someObj << LOG4CXX_ENDMSG;
       }

       void testWLogStreamScientific() {
           LoggerPtr root(Logger::getRootLogger());
           wlogstream os(root, Level::getInfo());
           os << std::scientific << 0.000001115 << LOG4CXX_ENDMSG;
           spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
           LogString msg(event->getMessage());
           LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("e-")) != LogString::npos ||
                msg.find(LOG4CXX_STR("E-")) != LogString::npos);
       }

       void testWLogStreamPrecision() {
          LoggerPtr root(Logger::getRootLogger());
          wlogstream os(root, Level::getInfo());
          os << std::setprecision(4) << 1.000001 << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("1.00000")) == LogString::npos);
      }


      void testWLogStreamWidth() {
          LoggerPtr root(Logger::getRootLogger());
          wlogstream os(root, Level::getInfo());
          os << L"[" << std::fixed << std::setprecision(2) << std::setw(7) << std::right << std::setfill(L'_') << 10.0 << L"]" << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }
       
       void wreport(std::basic_ostream<wchar_t>& os) {
          os << L"This just in: \n";
          os << L"Use logstream in places that expect a std::ostream.\n";
       }
       
        void testWLogStreamDelegate() {
            wlogstream root(Logger::getRootLogger(), Level::getInfo());
            wreport(root);
            root << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }
        
        void testWLogStreamFormattingPersists() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          wlogstream os(root, Level::getDebug());
          os << std::hex << 20 << LOG4CXX_ENDMSG;
          os << Level::getInfo() << 16 << LOG4CXX_ENDMSG;
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("10")), msg);
        }

        void testWSetWidthInsert() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          wlogstream os(root, Level::getInfo());
          os << std::setw(5);
          LOGUNIT_ASSERT_EQUAL(5, os.width());
        }
        
#endif        

#if LOG4CXX_UNICHAR_API
        void testUniChar() {
            LoggerPtr root(Logger::getRootLogger());
            const log4cxx::UniChar msg[] = { 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', 0 };
            LOG4CXX_INFO(root, msg);
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testUniCharAppend() {
           LoggerPtr root(Logger::getRootLogger());
           const log4cxx::UniChar msg1[] = { 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', 0 };
           const log4cxx::UniChar msg2[] = { ':', ' ', 'D', 'e', 't', 'a', 'i', 'l', 's', ' ', 't', 'o', ' ', 'f', 'o', 'l', 'l', 'o', 'w', 0 };
           LOG4CXX_INFO(root, msg1 << msg2);
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }
       
      void testUniCharWidth() {
          LoggerPtr root(Logger::getRootLogger());
          const log4cxx::UniChar openBracket[] = { '[', 0 };
          const log4cxx::UniChar closeBracket[] = { ']', 0 };
          LOG4CXX_INFO(root, openBracket << std::fixed << std::setprecision(2) << std::setw(7) << std::right << std::setfill((log4cxx::UniChar) '_') << 10.0 << closeBracket);
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }

        void testULogStreamSimple() {
            ulogstream root(Logger::getRootLogger(), Level::getInfo());
            const log4cxx::UniChar msg[] = { 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', 0 };
            root << msg << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testULogStreamMultiple() {
           ulogstream root(Logger::getRootLogger(), Level::getInfo());
           const log4cxx::UniChar msg1[] = { 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', 0 };
           const log4cxx::UniChar msg2[] = { ':',  ' ', 'D', 'e', 't', 'a', 'i', 'l', 's', ' ', 't', 'o', ' ', 'f', 'o', 'l', 'l', 'o', 'w', 0 };
           root << msg1 << msg2 << LOG4CXX_ENDMSG;
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }

       void testULogStreamShortCircuit() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.shortCircuit"));
         logger->setLevel(Level::getInfo());
         ulogstream os(logger, Level::getDebug());
         ExceptionOnInsert someObj;
         os << someObj << LOG4CXX_ENDMSG;
         LOGUNIT_ASSERT_EQUAL((size_t) 0, vectorAppender->getVector().size());
       }

       void testULogStreamInsertException() {
         LoggerPtr logger(Logger::getLogger("StreamTestCase.insertException"));
         ExceptionOnInsert someObj;
         ulogstream os(logger, Level::getInfo());
         os << someObj << LOG4CXX_ENDMSG;
       }

       void testULogStreamScientific() {
           LoggerPtr root(Logger::getRootLogger());
           ulogstream os(root, Level::getInfo());
           os << std::scientific << 0.000001115 << LOG4CXX_ENDMSG;
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
           spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
           LogString msg(event->getMessage());
           LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("e-")) != LogString::npos ||
                msg.find(LOG4CXX_STR("E-")) != LogString::npos);
       }

       void testULogStreamPrecision() {
          LoggerPtr root(Logger::getRootLogger());
          ulogstream os(root, Level::getInfo());
          os << std::setprecision(4) << 1.000001 << LOG4CXX_ENDMSG;
          LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT(msg.find(LOG4CXX_STR("1.00000")) == LogString::npos);
      }


      void testULogStreamWidth() {
          LoggerPtr root(Logger::getRootLogger());
          ulogstream os(root, Level::getInfo());
          const log4cxx::UniChar openBracket[] = { '[', 0 };
          const log4cxx::UniChar closeBracket[] = { ']', 0 };
          
          os << openBracket << std::fixed << std::setprecision(2) << std::setw(7) << std::right 
              << std::setfill((log4cxx::UniChar) '_') << 10.0 << closeBracket << LOG4CXX_ENDMSG;
          LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("[__10.00]")), msg);
       }
       
       void ureport(std::basic_ostream<log4cxx::UniChar>& os) {
          const log4cxx::UniChar msg1[] = { 'T', 'h', 'i', 's', ' ', 'j', 'u', 's', 't', ' ' , 'i', 'n', ':', ' ' , '\n', 0 };
          const log4cxx::UniChar msg2[] = { 'U', 's', 'e', ' ', 'l', 'o', 'g', 's', 't', 'r', 'e', 'a', 'm', '\n', 0 };
          os << msg1;
          os << msg2;
       }
       
        void testULogStreamDelegate() {
            ulogstream root(Logger::getRootLogger(), Level::getInfo());
            ureport(root);
            root << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }
        
        void testULogStreamFormattingPersists() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          ulogstream os(root, Level::getDebug());
          os << std::hex << 20 << LOG4CXX_ENDMSG;
          os << Level::getInfo() << 16 << LOG4CXX_ENDMSG;
          LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
          spi::LoggingEventPtr event(vectorAppender->getVector()[0]);
          LogString msg(event->getMessage());
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("10")), msg);
        }

        void testUSetWidthInsert() {
          LoggerPtr root(Logger::getRootLogger());
          root->setLevel(Level::getInfo());
          ulogstream os(root, Level::getInfo());
          os << std::setw(5);
          LOGUNIT_ASSERT_EQUAL(5, os.width());
        }
        
#endif        

#if LOG4CXX_CFSTRING_API
        void testCFString() {
            LoggerPtr root(Logger::getRootLogger());
            LOG4CXX_INFO(root, CFSTR("This is a test"));
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testCFStringAppend() {
           LoggerPtr root(Logger::getRootLogger());
           LOG4CXX_INFO(root, CFSTR("This is a test") << CFSTR(": Details to follow"));
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }

        void testULogStreamCFString() {
            ulogstream root(Logger::getRootLogger(), Level::getInfo());
            root << CFSTR("This is a test") << LOG4CXX_ENDMSG;
            LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
        }

        void testULogStreamCFString2() {
           ulogstream root(Logger::getRootLogger(), Level::getInfo());
           root << CFSTR("This is a test") << CFSTR(": Details to follow") << LOG4CXX_ENDMSG;
           LOGUNIT_ASSERT_EQUAL((size_t) 1, vectorAppender->getVector().size());
       }
#endif

};

LOGUNIT_TEST_SUITE_REGISTRATION(StreamTestCase);


#if !LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE
//
//   The following code tests compilation errors
//      around bug LOGCXX-150 and is not intended to be executed.
//      Skipped for VC6 since it can't handle having the
//      templated operator<< in class scope.s
namespace foo
{
  class Bar
  {
    void fn();
  };
  
std::ostream &operator<<(std::ostream &o, Bar const &b)
  {
    return o << "Bar";
  }
}


using namespace foo;

namespace
{
  log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("foo"));
  log4cxx::logstream lout(logger, log4cxx::Level::getDebug());
}

void Bar::fn()
{
  lout << "hi" << LOG4CXX_ENDMSG;
}
#endif

