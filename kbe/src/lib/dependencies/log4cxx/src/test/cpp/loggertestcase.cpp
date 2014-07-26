
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

#include <log4cxx/logger.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/appenderskeleton.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/logmanager.h>
#include <log4cxx/level.h>
#include <log4cxx/hierarchy.h>
#include <log4cxx/spi/rootlogger.h>
#include <log4cxx/helpers/propertyresourcebundle.h>
#include "insertwide.h"
#include "testchar.h"
#include "logunit.h"
#include <log4cxx/helpers/locale.h>
#include "vectorappender.h"

using namespace log4cxx;
using namespace log4cxx::spi;
using namespace log4cxx::helpers;

class CountingAppender;
typedef helpers::ObjectPtrT<CountingAppender> CountingAppenderPtr;

class CountingAppender : public AppenderSkeleton
{
public:
        int counter;

        CountingAppender() : counter(0)
                {}

        void close()
                {}

        void append(const spi::LoggingEventPtr& /*event*/, Pool& /*p*/)
                { counter++; }

        bool requiresLayout() const
                { return true; }
};

LOGUNIT_CLASS(LoggerTestCase)
{
        LOGUNIT_TEST_SUITE(LoggerTestCase);
                LOGUNIT_TEST(testAppender1);
                LOGUNIT_TEST(testAppender2);
                LOGUNIT_TEST(testAdditivity1);
                LOGUNIT_TEST(testAdditivity2);
                LOGUNIT_TEST(testAdditivity3);
                LOGUNIT_TEST(testDisable1);
//    LOGUNIT_TEST(testRB1);
//    LOGUNIT_TEST(testRB2);  //TODO restore
//    LOGUNIT_TEST(testRB3);
                LOGUNIT_TEST(testExists);
                LOGUNIT_TEST(testHierarchy1);
                LOGUNIT_TEST(testTrace);
                LOGUNIT_TEST(testIsTraceEnabled);
        LOGUNIT_TEST_SUITE_END();

public:
        void setUp()
        {
        }

        void tearDown()
        {
                BasicConfigurator::resetConfiguration();
                a1 = 0;
                a2 = 0;
        logger = 0;
        }

        /**
        Add an appender and see if it can be retrieved.
        */
        void testAppender1()
        {
                logger = Logger::getLogger(LOG4CXX_TEST_STR("test"));
                a1 = new FileAppender();
                a1->setName(LOG4CXX_STR("testAppender1"));
                logger->addAppender(a1);

                AppenderList list = logger->getAllAppenders();
                AppenderPtr aHat = list.front();
                LOGUNIT_ASSERT_EQUAL(a1, aHat);
        }

        /**
        Add an appender X, Y, remove X and check if Y is the only
        remaining appender.
        */
        void testAppender2()
        {
                a1 = new FileAppender();
                a1->setName(LOG4CXX_STR("testAppender2.1"));
                a2 = new FileAppender();
                a2->setName(LOG4CXX_STR("testAppender2.2"));

                logger = Logger::getLogger(LOG4CXX_TEST_STR("test"));
                logger->addAppender(a1);
                logger->addAppender(a2);
                logger->removeAppender((LogString) LOG4CXX_STR("testAppender2.1"));

                AppenderList list = logger->getAllAppenders();
                AppenderPtr aHat = list.front();
                LOGUNIT_ASSERT_EQUAL(a2, aHat);
                LOGUNIT_ASSERT(list.size() == 1);
        }

        /**
        Test if LoggerPtr a.b inherits its appender from a.
        */
        void testAdditivity1()
        {
                LoggerPtr a = Logger::getLogger(LOG4CXX_TEST_STR("a"));
                LoggerPtr ab = Logger::getLogger(LOG4CXX_TEST_STR("a.b"));
                CountingAppenderPtr ca = new CountingAppender();
                a->addAppender(ca);

                LOGUNIT_ASSERT_EQUAL(ca->counter, 0);
                ab->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(ca->counter, 1);
                ab->info(MSG);
                LOGUNIT_ASSERT_EQUAL(ca->counter, 2);
                ab->warn(MSG);
                LOGUNIT_ASSERT_EQUAL(ca->counter, 3);
                ab->error(MSG);
                LOGUNIT_ASSERT_EQUAL(ca->counter, 4);
        }

        /**
        Test multiple additivity.
        */
        void testAdditivity2()
        {
                LoggerPtr a = Logger::getLogger(LOG4CXX_TEST_STR("a"));
                LoggerPtr ab = Logger::getLogger(LOG4CXX_TEST_STR("a.b"));
                LoggerPtr abc = Logger::getLogger(LOG4CXX_TEST_STR("a.b.c"));
                LoggerPtr x = Logger::getLogger(LOG4CXX_TEST_STR("x"));

                CountingAppenderPtr ca1 = new CountingAppender();
                CountingAppenderPtr ca2 = new CountingAppender();

                a->addAppender(ca1);
                abc->addAppender(ca2);

                LOGUNIT_ASSERT_EQUAL(ca1->counter, 0);
                LOGUNIT_ASSERT_EQUAL(ca2->counter, 0);

                ab->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(ca1->counter, 1);
                LOGUNIT_ASSERT_EQUAL(ca2->counter, 0);

                abc->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(ca1->counter, 2);
                LOGUNIT_ASSERT_EQUAL(ca2->counter, 1);

                x->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(ca1->counter, 2);
                LOGUNIT_ASSERT_EQUAL(ca2->counter, 1);
        }

        /**
        Test additivity flag.
        */
        void testAdditivity3()
        {
                LoggerPtr root = Logger::getRootLogger();
                LoggerPtr a = Logger::getLogger(LOG4CXX_TEST_STR("a"));
                LoggerPtr ab = Logger::getLogger(LOG4CXX_TEST_STR("a.b"));
                LoggerPtr abc = Logger::getLogger(LOG4CXX_TEST_STR("a.b.c"));
                LoggerPtr x = Logger::getLogger(LOG4CXX_TEST_STR("x"));

                CountingAppenderPtr caRoot = new CountingAppender();
                CountingAppenderPtr caA = new CountingAppender();
                CountingAppenderPtr caABC = new CountingAppender();

                root->addAppender(caRoot);
                a->addAppender(caA);
                abc->addAppender(caABC);

                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 0);
                LOGUNIT_ASSERT_EQUAL(caA->counter, 0);
                LOGUNIT_ASSERT_EQUAL(caABC->counter, 0);

                ab->setAdditivity(false);

                a->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caA->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caABC->counter, 0);

                ab->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caA->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caABC->counter, 0);

                abc->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caA->counter, 1);
                LOGUNIT_ASSERT_EQUAL(caABC->counter, 1);
        }

        void testDisable1()
        {
                CountingAppenderPtr caRoot = new CountingAppender();
                LoggerPtr root = Logger::getRootLogger();
                root->addAppender(caRoot);

                LoggerRepositoryPtr h = LogManager::getLoggerRepository();

                //h.disableDebug();
                h->setThreshold(Level::getInfo());
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 0);

                root->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 0);
                root->info(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 1);
                root->log(Level::getWarn(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 2);
                root->warn(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 3);

                //h.disableInfo();
                h->setThreshold(Level::getWarn());
                root->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 3);
                root->info(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 3);
                root->log(Level::getWarn(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 4);
                root->error(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 5);
                root->log(Level::getError(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);

                //h.disableAll();
                h->setThreshold(Level::getOff());
                root->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->info(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getWarn(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->error(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getFatal(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getFatal(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);

                //h.disable(Level::getFatalLevel());
                h->setThreshold(Level::getOff());
                root->debug(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->info(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getWarn(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->error(MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getWarn(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
                root->log(Level::getFatal(), MSG);
                LOGUNIT_ASSERT_EQUAL(caRoot->counter, 6);
        }


        ResourceBundlePtr getBundle(const LogString& lang, const LogString& region)
        {
             Locale l(lang, region);
             ResourceBundlePtr bundle(
                 PropertyResourceBundle::getBundle(LOG4CXX_STR("L7D"),l));
             LOGUNIT_ASSERT(bundle != 0);
             return bundle;
        }

        void testRB1()
        {
                ResourceBundlePtr rbUS(getBundle(LOG4CXX_STR("en"), LOG4CXX_STR("US")));
                ResourceBundlePtr rbFR(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("FR")));
                ResourceBundlePtr rbCH(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("CH")));

                LoggerPtr root = Logger::getRootLogger();
                root->setResourceBundle(rbUS);

                ResourceBundlePtr t = root->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);

                LoggerPtr x = Logger::getLogger(LOG4CXX_TEST_STR("x"));
                LoggerPtr x_y = Logger::getLogger(LOG4CXX_TEST_STR("x.y"));
                LoggerPtr x_y_z = Logger::getLogger(LOG4CXX_TEST_STR("x.y.z"));

                t = x->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);
                t = x_y->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);
                t = x_y_z->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);
        }

        void testRB2()
        {
                LoggerPtr root = Logger::getRootLogger();
                ResourceBundlePtr rbUS(getBundle(LOG4CXX_STR("en"), LOG4CXX_STR("US")));
                ResourceBundlePtr rbFR(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("FR")));
                ResourceBundlePtr rbCH(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("CH")));

                root->setResourceBundle(rbUS);

                ResourceBundlePtr t = root->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);

                LoggerPtr x = Logger::getLogger(LOG4CXX_TEST_STR("x"));
                LoggerPtr x_y = Logger::getLogger(LOG4CXX_TEST_STR("x.y"));
                LoggerPtr x_y_z = Logger::getLogger(LOG4CXX_TEST_STR("x.y.z"));

                x_y->setResourceBundle(rbFR);
                t = x->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);
                t = x_y->getResourceBundle();
                LOGUNIT_ASSERT(t == rbFR);
                t = x_y_z->getResourceBundle();
                LOGUNIT_ASSERT(t == rbFR);
        }

        void testRB3()
        {
                ResourceBundlePtr rbUS(getBundle(LOG4CXX_STR("en"), LOG4CXX_STR("US")));
                ResourceBundlePtr rbFR(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("FR")));
                ResourceBundlePtr rbCH(getBundle(LOG4CXX_STR("fr"), LOG4CXX_STR("CH")));

                LoggerPtr root = Logger::getRootLogger();
                root->setResourceBundle(rbUS);

                ResourceBundlePtr t = root->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);

                LoggerPtr x = Logger::getLogger(LOG4CXX_TEST_STR("x"));
                LoggerPtr x_y = Logger::getLogger(LOG4CXX_TEST_STR("x.y"));
                LoggerPtr x_y_z = Logger::getLogger(LOG4CXX_TEST_STR("x.y.z"));

                x_y->setResourceBundle(rbFR);
                x_y_z->setResourceBundle(rbCH);
                t = x->getResourceBundle();
                LOGUNIT_ASSERT(t == rbUS);
                t = x_y->getResourceBundle();
                LOGUNIT_ASSERT(t == rbFR);
                t = x_y_z->getResourceBundle();
                LOGUNIT_ASSERT(t == rbCH);
        }

        void testExists()
        {
                LoggerPtr a = Logger::getLogger(LOG4CXX_TEST_STR("a"));
                LoggerPtr a_b = Logger::getLogger(LOG4CXX_TEST_STR("a.b"));
                LoggerPtr a_b_c = Logger::getLogger(LOG4CXX_TEST_STR("a.b.c"));

                LoggerPtr t;
                t = LogManager::exists(LOG4CXX_TEST_STR("xx"));
                LOGUNIT_ASSERT(t == 0);
                t = LogManager::exists(LOG4CXX_TEST_STR("a"));
                LOGUNIT_ASSERT_EQUAL(a, t);
                t = LogManager::exists(LOG4CXX_TEST_STR("a.b"));
                LOGUNIT_ASSERT_EQUAL(a_b, t);
                t = LogManager::exists(LOG4CXX_TEST_STR("a.b.c"));
                LOGUNIT_ASSERT_EQUAL(a_b_c, t);
        }

        void testHierarchy1()
        {
                LoggerRepositoryPtr h = new Hierarchy();
                LoggerPtr root(h->getRootLogger());
                root->setLevel(Level::getError());
                LoggerPtr a0 = h->getLogger(LOG4CXX_STR("a"));
                LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("a"), a0->getName());
                LOGUNIT_ASSERT(a0->getLevel() == 0);
                LOGUNIT_ASSERT(Level::getError() == a0->getEffectiveLevel());

                LoggerPtr a11 = h->getLogger(LOG4CXX_STR("a"));
                LOGUNIT_ASSERT_EQUAL(a0, a11);
        }
        
        void compileTestForLOGCXX202() const {
           //
           //   prior to fix, these line would compile. 
           //
           (*logger).info("Hello, World.");
           ((Logger*) logger)->info("Hello, World.");
           //
           //   this one would not.
           //
           logger->info("Hello, World.");
        }
        
        
  /**
   * Tests logger.trace(Object).
   * 
   */
  void testTrace() {
      VectorAppenderPtr appender = new VectorAppender();
      LoggerPtr root = Logger::getRootLogger();
      root->addAppender(appender);
      root->setLevel(Level::getInfo());

      LoggerPtr tracer = Logger::getLogger("com.example.Tracer");
      tracer->setLevel(Level::getTrace());

      LOG4CXX_TRACE(tracer, "Message 1");
      LOG4CXX_TRACE(root, "Discarded Message");
      LOG4CXX_TRACE(root, "Discarded Message");

      std::vector<LoggingEventPtr> msgs(appender->vector);
      LOGUNIT_ASSERT_EQUAL((size_t) 1, msgs.size());
      LoggingEventPtr event = msgs[0];
      LOGUNIT_ASSERT_EQUAL((int) Level::TRACE_INT, event->getLevel()->toInt());
      LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("Message 1")), event->getMessage());
  }

    /**
     * Tests isTraceEnabled.
     * 
     */
    void testIsTraceEnabled() {
        VectorAppenderPtr appender = new VectorAppender();
        LoggerPtr root = Logger::getRootLogger();
        root->addAppender(appender);
        root->setLevel(Level::getInfo());

        LoggerPtr tracer = Logger::getLogger("com.example.Tracer");
        tracer->setLevel(Level::getTrace());

        LOGUNIT_ASSERT_EQUAL(true, tracer->isTraceEnabled());
        LOGUNIT_ASSERT_EQUAL(false, root->isTraceEnabled());
    }

protected:
        static LogString MSG;
        LoggerPtr logger;
        AppenderPtr a1;
        AppenderPtr a2;
};

LogString LoggerTestCase::MSG(LOG4CXX_STR("M"));

LOGUNIT_TEST_SUITE_REGISTRATION(LoggerTestCase);
