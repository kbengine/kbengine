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

#include <log4cxx/helpers/cyclicbuffer.h>
#include "../logunit.h"

#include <log4cxx/logmanager.h>
#include <log4cxx/logger.h>
#include <log4cxx/spi/loggingevent.h>
#include <log4cxx/spi/location/locationinfo.h>
#include "../testchar.h"

using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::spi;

#define MAX 1000

LOGUNIT_CLASS(CyclicBufferTestCase)
{
   LOGUNIT_TEST_SUITE(CyclicBufferTestCase);
      LOGUNIT_TEST(test0);
      LOGUNIT_TEST(test1);
      LOGUNIT_TEST(testResize);
   LOGUNIT_TEST_SUITE_END();

   LoggerPtr logger;
   std::vector<LoggingEventPtr> e;

public:
   void setUp()
   {
      e.reserve(1000);
      LoggingEventPtr event;
      for (int i = 0; i < MAX; i++)
      {
         event = new LoggingEvent(LOG4CXX_STR("x"), Level::getDebug(), LOG4CXX_STR("e"),
                          log4cxx::spi::LocationInfo::getLocationUnavailable());
         e.push_back(event);
      }
   }

   void tearDown()
   {
      LogManager::shutdown();
   }

   void test0()
   {
      int size = 2;

      CyclicBuffer cb(size);
      LOGUNIT_ASSERT_EQUAL(size, cb.getMaxSize());

      cb.add(e[0]);
      LOGUNIT_ASSERT_EQUAL(1, cb.length());
      LOGUNIT_ASSERT_EQUAL(e[0], cb.get());
      LOGUNIT_ASSERT_EQUAL(0, cb.length());
      LOGUNIT_ASSERT(cb.get() == 0);
      LOGUNIT_ASSERT_EQUAL(0, cb.length());

      CyclicBuffer cb2(size);
      cb2.add(e[0]);
      cb2.add(e[1]);
      LOGUNIT_ASSERT_EQUAL(2, cb2.length());
      LOGUNIT_ASSERT_EQUAL(e[0], cb2.get());
      LOGUNIT_ASSERT_EQUAL(1, cb2.length());
      LOGUNIT_ASSERT_EQUAL(e[1], cb2.get());
      LOGUNIT_ASSERT_EQUAL(0, cb2.length());
      LOGUNIT_ASSERT(cb2.get() == 0);
      LOGUNIT_ASSERT_EQUAL(0, cb2.length());
   }

   void test1()
   {
      for (int bufSize = 1; bufSize <= 128; bufSize *= 2)
         doTest1(bufSize);
   }

   void doTest1(int size)
   {
      //System.out.println("Doing test with size = "+size);
      CyclicBuffer cb(size);

      LOGUNIT_ASSERT_EQUAL(size, cb.getMaxSize());

      int i;

      for (i = -(size + 10); i < (size + 10); i++)
      {
         LOGUNIT_ASSERT(cb.get(i) == 0);
      }

      for (i = 0; i < MAX; i++)
      {
         cb.add(e[i]);

         int limit = (i < (size - 1)) ? i : (size - 1);

         //System.out.println("\nLimit is " + limit + ", i="+i);
         for (int j = limit; j >= 0; j--)
         {
            //System.out.println("i= "+i+", j="+j);
            LOGUNIT_ASSERT_EQUAL(e[i - (limit - j)], cb.get(j));
         }

         LOGUNIT_ASSERT(cb.get(-1) == 0);
         LOGUNIT_ASSERT(cb.get(limit + 1) == 0);
      }
   }

   void testResize()
   {
      for (int isize = 1; isize <= 128; isize *= 2)
      {
         doTestResize(isize, (isize / 2) + 1, (isize / 2) + 1);
         doTestResize(isize, (isize / 2) + 1, isize + 10);
         doTestResize(isize, isize + 10, (isize / 2) + 1);
         doTestResize(isize, isize + 10, isize + 10);
      }

   }

   void doTestResize(int initialSize, int numberOfAdds, int newSize)
   {
      //System.out.println("initialSize = "+initialSize+", numberOfAdds="
      //        +numberOfAdds+", newSize="+newSize);
      CyclicBuffer cb(initialSize);

      for (int i = 0; i < numberOfAdds; i++)
      {
         cb.add(e[i]);
      }

      cb.resize(newSize);

      int offset = numberOfAdds - initialSize;

      if (offset < 0)
      {
         offset = 0;
      }

      int len = (newSize < numberOfAdds) ? newSize : numberOfAdds;
      len = (len < initialSize) ? len : initialSize;

      //System.out.println("Len = "+len+", offset="+offset);
      for (int j = 0; j < len; j++)
      {
         LOGUNIT_ASSERT_EQUAL(e[offset + j], cb.get(j));
      }
   }
};

LOGUNIT_TEST_SUITE_REGISTRATION(CyclicBufferTestCase);
