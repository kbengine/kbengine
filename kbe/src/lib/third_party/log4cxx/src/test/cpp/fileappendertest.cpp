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
#include <log4cxx/helpers/pool.h>
#include <log4cxx/fileappender.h>
#include <log4cxx/patternlayout.h>
#include "logunit.h"

using namespace log4cxx;
using namespace log4cxx::helpers;


/**
 *
 * FileAppender tests.
 */
LOGUNIT_CLASS(FileAppenderTest) {
  LOGUNIT_TEST_SUITE(FileAppenderTest);
          LOGUNIT_TEST(testDirectoryCreation);
          LOGUNIT_TEST(testgetSetThreshold);
          LOGUNIT_TEST(testIsAsSevereAsThreshold);
  LOGUNIT_TEST_SUITE_END();
public:
  /**
   * Tests that any necessary directories are attempted to
   * be created if they don't exist.  See bug 9150.
   *
   */
  void testDirectoryCreation() {
      File newFile(LOG4CXX_STR("output/newdir/temp.log"));
      Pool p;
      newFile.deleteFile(p);

      File newDir(LOG4CXX_STR("output/newdir"));
      newDir.deleteFile(p);

      FileAppenderPtr wa(new FileAppender());
      wa->setFile(LOG4CXX_STR("output/newdir/temp.log"));
      wa->setLayout(new PatternLayout(LOG4CXX_STR("%m%n")));
      wa->activateOptions(p);

      LOGUNIT_ASSERT(File(LOG4CXX_STR("output/newdir/temp.log")).exists(p));
  }

  /**
   * Tests getThreshold and setThreshold.
   */
  void testgetSetThreshold() {
    FileAppenderPtr appender = new FileAppender();
    LevelPtr debug = Level::getDebug();
    //
    //  different from log4j where threshold is null.
    //
    LOGUNIT_ASSERT_EQUAL(Level::getAll(), appender->getThreshold());
    appender->setThreshold(debug);
    LOGUNIT_ASSERT_EQUAL(debug, appender->getThreshold());
  }

  /**
   * Tests isAsSevereAsThreshold.
   */
  void testIsAsSevereAsThreshold() {
    FileAppenderPtr appender = new FileAppender();
    LevelPtr debug = Level::getDebug();
    LOGUNIT_ASSERT(appender->isAsSevereAsThreshold(debug));
  }
};

LOGUNIT_TEST_SUITE_REGISTRATION(FileAppenderTest);

