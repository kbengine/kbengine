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

#include <log4cxx/helpers/inetaddress.h>
#include "../logunit.h"

using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(InetAddressTestCase)
{
        LOGUNIT_TEST_SUITE(InetAddressTestCase);
                LOGUNIT_TEST(testGetLocalHost);
                LOGUNIT_TEST(testByNameLocal);
                LOGUNIT_TEST(testAllByNameLocal);
                LOGUNIT_TEST(testUnreachable);
        LOGUNIT_TEST_SUITE_END();

public:
        /**
         * Tests the InetAddress::getLocalHost() method.
         */
        void testGetLocalHost() {
           InetAddressPtr addr = InetAddress::getLocalHost();

           LOGUNIT_ASSERT(addr->getHostAddress() == LOG4CXX_STR("127.0.0.1"));
           LOGUNIT_ASSERT(!addr->getHostName().empty());
        }

        /**
         * Tests the InetAddress::getByName() method with the
         * "localhost" host name.
         */
        void testByNameLocal() {
           InetAddressPtr addr = InetAddress::getByName(LOG4CXX_STR("localhost"));

           LOGUNIT_ASSERT(addr->getHostAddress() == LOG4CXX_STR("127.0.0.1"));
           LOGUNIT_ASSERT(!addr->getHostName().empty());
        }

        /**
         * Tests the InetAddress::getAllByName() method with the
         * "localhost" host name.
         */
        void testAllByNameLocal() {
           std::vector<InetAddressPtr> addr = InetAddress::getAllByName(LOG4CXX_STR("localhost"));

           LOGUNIT_ASSERT(addr.size() > 0);
        }

        /**
         * Tests the UnknownHostException.
         */
        void testUnknownHost() {
           InetAddressPtr addr = InetAddress::getByName(LOG4CXX_STR("unknown.invalid"));
        }
      
    /**
    * Tests an (likely) unreachable address.
    */
      void testUnreachable()  {
       InetAddressPtr addr(InetAddress::getByName(LOG4CXX_STR("192.168.10.254")));
      LogString addrStr(addr->toString());
      LOGUNIT_ASSERT_EQUAL(addrStr.size() - 15, addrStr.find(LOG4CXX_STR("/192.168.10.254")));
   }

};


LOGUNIT_TEST_SUITE_REGISTRATION(InetAddressTestCase);

