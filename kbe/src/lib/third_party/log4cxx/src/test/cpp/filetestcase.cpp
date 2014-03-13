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

#include <log4cxx/file.h>
#include "logunit.h"
#include "insertwide.h"
#include <log4cxx/helpers/pool.h>
#include <apr_errno.h>
#include <log4cxx/helpers/exception.h>
#include <log4cxx/helpers/fileinputstream.h>

#include <log4cxx/helpers/outputstreamwriter.h>
#include <log4cxx/helpers/fileoutputstream.h>
#include <log4cxx/helpers/inputstreamreader.h>
#include <log4cxx/helpers/fileinputstream.h>

#if LOG4CXX_CFSTRING_API
#include <CoreFoundation/CFString.h>
#endif

using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(FileTestCase)
{
        LOGUNIT_TEST_SUITE(FileTestCase);
                LOGUNIT_TEST(defaultConstructor);
                LOGUNIT_TEST(defaultExists);
                LOGUNIT_TEST(defaultRead);
                LOGUNIT_TEST(propertyRead);
                LOGUNIT_TEST(propertyExists);
                LOGUNIT_TEST(fileWrite1);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(wcharConstructor);
#endif
#if LOG4CXX_UNICHAR_API
                LOGUNIT_TEST(unicharConstructor);
#endif
#if LOG4CXX_CFSTRING_API
                LOGUNIT_TEST(cfstringConstructor);
#endif
                LOGUNIT_TEST(copyConstructor);
                LOGUNIT_TEST(assignment);
                LOGUNIT_TEST(deleteBackslashedFileName);
        LOGUNIT_TEST_SUITE_END();

public:
        void defaultConstructor() {
          File defFile;
          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR(""), defFile.getPath());
        }



        void defaultExists() {
          File defFile;
          Pool pool;
          bool exists = defFile.exists(pool);
          LOGUNIT_ASSERT_EQUAL(false, exists);
        }

        // check default constructor. read() throws an exception
        // if no file name was given.
        void defaultRead() {
          File defFile;
          Pool pool;
          try {
            InputStreamPtr defInput = new FileInputStream(defFile);
            InputStreamReaderPtr inputReader = new InputStreamReader(defInput);
            LogString contents(inputReader->read(pool));
            LOGUNIT_ASSERT(false);
          } catch(IOException &ex) {
          }
        }


#if LOG4CXX_WCHAR_T_API
        void wcharConstructor() {
            File propFile(L"input/patternLayout1.properties");
            Pool pool;
            bool exists = propFile.exists(pool);
            LOGUNIT_ASSERT_EQUAL(true, exists);
       }
#endif

#if LOG4CXX_UNICHAR_API
        void unicharConstructor() {
            const log4cxx::UniChar filename[] = { 'i', 'n', 'p', 'u', 't', '/', 
               'p', 'a', 't', 't', 'e', 'r', 'n', 'L', 'a', 'y', 'o', 'u', 't', '1', '.', 
               'p', 'r', 'o', 'p', 'e', 'r', 't', 'i', 'e', 's', 0 };
            File propFile(filename);
            Pool pool;
            bool exists = propFile.exists(pool);
            LOGUNIT_ASSERT_EQUAL(true, exists);
       }
#endif

#if LOG4CXX_CFSTRING_API
        void cfstringConstructor() {
            File propFile(CFSTR("input/patternLayout.properties"));
            Pool pool;
            bool exists = propFile.exists(pool);
            LOGUNIT_ASSERT_EQUAL(true, exists);
       }
#endif

        void copyConstructor() {
            File propFile("input/patternLayout1.properties");
            File copy(propFile);
            Pool pool;
            bool exists = copy.exists(pool);
            LOGUNIT_ASSERT_EQUAL(true, exists);
        }

        void assignment() {
            File propFile("input/patternLayout1.properties");
            File copy = propFile;
            Pool pool;
            bool exists = copy.exists(pool);
            LOGUNIT_ASSERT_EQUAL(true, exists);
        }

        void propertyRead() {
          File propFile("input/patternLayout1.properties");
          Pool pool;
          InputStreamPtr propStream = new FileInputStream(propFile);
          InputStreamReaderPtr propReader = new InputStreamReader(propStream);
          LogString props(propReader->read(pool));
          LogString line1(LOG4CXX_STR("# Licensed to the Apache Software Foundation (ASF) under one or more"));
          LOGUNIT_ASSERT_EQUAL(line1, props.substr(0, line1.length()));
        }

        void propertyExists() {
          File propFile("input/patternLayout1.properties");
          Pool pool;
          bool exists = propFile.exists(pool);
          LOGUNIT_ASSERT_EQUAL(true, exists);
        }

        void fileWrite1() {
          OutputStreamPtr fos =
                      new FileOutputStream(LOG4CXX_STR("output/fileWrite1.txt"));
          OutputStreamWriterPtr osw = new OutputStreamWriter(fos);

          Pool pool;
          LogString greeting(LOG4CXX_STR("Hello, World"));
          greeting.append(LOG4CXX_EOL);
          osw->write(greeting, pool);

          InputStreamPtr is =
                      new FileInputStream(LOG4CXX_STR("output/fileWrite1.txt"));
          InputStreamReaderPtr isr = new InputStreamReader(is);
          LogString reply = isr->read(pool);

          LOGUNIT_ASSERT_EQUAL(greeting, reply);
        }

        /**
         *  Tests conversion of backslash containing file names.
         *  Would cause infinite loop due to bug LOGCXX-105.
         */
        void deleteBackslashedFileName() {
          File file("output\\bogus.txt");
          Pool pool;
          /*bool deleted = */file.deleteFile(pool);
        }
};


LOGUNIT_TEST_SUITE_REGISTRATION(FileTestCase);
