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

#define LOG4CXX_TEST 1
#include <log4cxx/private/log4cxx_private.h>

#if LOG4CXX_HAVE_SMTP

#include <log4cxx/net/smtpappender.h>
#include "../appenderskeletontestcase.h"
#include <log4cxx/xml/domconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/ttcclayout.h>

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::net;
using namespace log4cxx::xml;
using namespace log4cxx::spi;

namespace log4cxx {
    namespace net {

                class MockTriggeringEventEvaluator :
                        public virtual spi::TriggeringEventEvaluator,
                        public virtual helpers::ObjectImpl
                {
                public:
                        DECLARE_LOG4CXX_OBJECT(MockTriggeringEventEvaluator)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(MockTriggeringEventEvaluator)
                                LOG4CXX_CAST_ENTRY(spi::TriggeringEventEvaluator)
                        END_LOG4CXX_CAST_MAP()

                        MockTriggeringEventEvaluator() {
                        }

                        virtual bool isTriggeringEvent(const spi::LoggingEventPtr& event) {
                            return true;
                        }
                private:
                         MockTriggeringEventEvaluator(const MockTriggeringEventEvaluator&);
                         MockTriggeringEventEvaluator& operator=(const MockTriggeringEventEvaluator&);
                };
    }
}

IMPLEMENT_LOG4CXX_OBJECT(MockTriggeringEventEvaluator)


/**
   Unit tests of log4cxx::SocketAppender
 */
class SMTPAppenderTestCase : public AppenderSkeletonTestCase
{
   LOGUNIT_TEST_SUITE(SMTPAppenderTestCase);
                //
                //    tests inherited from AppenderSkeletonTestCase
                //
                LOGUNIT_TEST(testDefaultThreshold);
                LOGUNIT_TEST(testSetOptionThreshold);
                LOGUNIT_TEST(testTrigger);
                LOGUNIT_TEST(testInvalid);
   LOGUNIT_TEST_SUITE_END();


public:

        AppenderSkeleton* createAppenderSkeleton() const {
          return new log4cxx::net::SMTPAppender();
        }
        
   void setUp() {
   }
   
   void tearDown() {
       LogManager::resetConfiguration();
   }

    /**
     * Tests that triggeringPolicy element will set evaluator.
     */
  void testTrigger() {
      DOMConfigurator::configure("input/xml/smtpAppender1.xml");
      SMTPAppenderPtr appender(Logger::getRootLogger()->getAppender(LOG4CXX_STR("A1")));
      TriggeringEventEvaluatorPtr evaluator(appender->getEvaluator());
      LOGUNIT_ASSERT_EQUAL(true, evaluator->instanceof(MockTriggeringEventEvaluator::getStaticClass()));
  }
  
  void testInvalid() {
      SMTPAppenderPtr appender(new SMTPAppender());
      appender->setSMTPHost(LOG4CXX_STR("smtp.invalid"));
      appender->setTo(LOG4CXX_STR("you@example.invalid"));
      appender->setFrom(LOG4CXX_STR("me@example.invalid"));
      appender->setLayout(new TTCCLayout());
      Pool p;
      appender->activateOptions(p);
      LoggerPtr root(Logger::getRootLogger());
      root->addAppender(appender);
      LOG4CXX_INFO(root, "Hello, World.");
      LOG4CXX_ERROR(root, "Sending Message");
  }

};

LOGUNIT_TEST_SUITE_REGISTRATION(SMTPAppenderTestCase);

#endif
