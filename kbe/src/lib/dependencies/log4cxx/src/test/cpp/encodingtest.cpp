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
#include "logunit.h"
//
//  If there is no support for wchar_t logging then
//     there is not a consistent way to get the test characters logged
//
#if LOG4CXX_WCHAR_T_API


#include <log4cxx/patternlayout.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/level.h>

#include "util/binarycompare.h"
#include <log4cxx/file.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::util;
using namespace log4cxx::helpers;

/**
 * Tests support for encoding specification.
 * 
 * 
 */
LOGUNIT_CLASS(EncodingTest) {
  LOGUNIT_TEST_SUITE(EncodingTest);
          LOGUNIT_TEST(testASCII);
          LOGUNIT_TEST(testLatin1);
          LOGUNIT_TEST(testUtf8);
          LOGUNIT_TEST(testUtf16);
          LOGUNIT_TEST(testUtf16LE);
          LOGUNIT_TEST(testUtf16BE);
  LOGUNIT_TEST_SUITE_END();
public:
    /**
     * Resets configuration after each test.
     */
    void tearDown() {
      Logger::getRootLogger()->getLoggerRepository()->resetConfiguration();
    }


    /**
     * Test us-ascii encoding.
     */
  void testASCII() {
      LoggerPtr root(Logger::getRootLogger());
      configure(root, LOG4CXX_STR("output/ascii.log"), LOG4CXX_STR("US-ASCII"));
      common(root);
      BinaryCompare::compare("output/ascii.log", "witness/encoding/ascii.log");
  }

    /**
     * Test iso-8859-1 encoding.
     */
    void testLatin1() {
        LoggerPtr root(Logger::getRootLogger());
        configure(root, LOG4CXX_STR("output/latin1.log"), LOG4CXX_STR("iso-8859-1"));
        common(root);
        BinaryCompare::compare("output/latin1.log", "witness/encoding/latin1.log");
    }

    /**
     * Test utf-8 encoding.
     */
    void testUtf8() {
        LoggerPtr root(Logger::getRootLogger());
        configure(root, LOG4CXX_STR("output/UTF-8.log"), LOG4CXX_STR("UTF-8"));
        common(root);
        BinaryCompare::compare("output/UTF-8.log", "witness/encoding/UTF-8.log");
    }

    /**
     * Test utf-16 encoding.
     */
    void testUtf16() {
        LoggerPtr root(Logger::getRootLogger());
        configure(root, LOG4CXX_STR("output/UTF-16.log"), LOG4CXX_STR("UTF-16"));
        common(root);
        BinaryCompare::compare("output/UTF-16.log", "witness/encoding/UTF-16.log");
    }

    /**
     * Test utf-16be encoding.
     */
    void testUtf16BE() {
        LoggerPtr root(Logger::getRootLogger());
        configure(root, LOG4CXX_STR("output/UTF-16BE.log"), LOG4CXX_STR("UTF-16BE"));
        common(root);
        BinaryCompare::compare("output/UTF-16BE.log", "witness/encoding/UTF-16BE.log");
    }

    /**
     * Test utf16-le encoding.
     */
    void testUtf16LE() {
        LoggerPtr root(Logger::getRootLogger());
        configure(root, LOG4CXX_STR("output/UTF-16LE.log"), LOG4CXX_STR("UTF-16LE"));
        common(root);
        BinaryCompare::compare("output/UTF-16LE.log", "witness/encoding/UTF-16LE.log");
    }

    /**
     * Configure logging.
     * @param logger logger
     * @param filename logging file name
     * @param encoding encoding
     */
    private:
    void configure(LoggerPtr& logger,
        const LogString& filename, const LogString& encoding) {
        PatternLayoutPtr layout(new PatternLayout());
        layout->setConversionPattern(LOG4CXX_STR("%p - %m\n"));
        Pool p;
        layout->activateOptions(p);
        FileAppenderPtr appender(new FileAppender());
        appender->setFile(filename);
        appender->setEncoding(encoding);
        appender->setAppend(false);
        appender->setLayout(layout);
        appender->activateOptions(p);
        logger->addAppender(appender);
        logger->setLevel(Level::getInfo());
    }

    /**
     * Common logging requests.
     * @param logger logger
     */
    void common(LoggerPtr& logger) {
        logger->info("Hello, World");
        // pi can be encoded in iso-8859-1
        const wchar_t pi[] = { 0x00B9, 0 };
        logger->info(pi);
        //   arbitrary, hopefully meaningless, characters from
        //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
        const wchar_t greeting[] = { L'A', 0x0605, 0x0530, 0x986, 0x4E03, 0x400, 0 };
        logger->info(greeting);

    }
};

LOGUNIT_TEST_SUITE_REGISTRATION(EncodingTest);

#endif

