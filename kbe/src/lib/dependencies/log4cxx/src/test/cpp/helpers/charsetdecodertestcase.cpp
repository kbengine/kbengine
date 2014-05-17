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

#include <log4cxx/helpers/charsetdecoder.h>
#include "../logunit.h"
#include "../insertwide.h"
#include <log4cxx/helpers/bytebuffer.h>

using namespace log4cxx;
using namespace log4cxx::helpers;


#define APR_SUCCESS ((log4cxx_status_t) 0)



LOGUNIT_CLASS(CharsetDecoderTestCase)
{
        LOGUNIT_TEST_SUITE(CharsetDecoderTestCase);
                LOGUNIT_TEST(decode1);
                LOGUNIT_TEST(decode2);
                LOGUNIT_TEST(decode8);
        LOGUNIT_TEST_SUITE_END();

        enum { BUFSIZE = 256 };

public:


        void decode1() {
          char buf[] = "Hello, World";
          ByteBuffer src(buf, strlen(buf));

          CharsetDecoderPtr dec(CharsetDecoder::getDefaultDecoder());
          LogString greeting;
          log4cxx_status_t stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);

          stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 12, src.position());

          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("Hello, World"), greeting);
        }

        void decode2() {
          char buf[BUFSIZE + 6];
          memset(buf, 'A', BUFSIZE);
          buf[BUFSIZE - 3] = 0;
#if defined(__STDC_LIB_EXT1__) || defined(__STDC_SECURE_LIB__)
          strcat_s(buf, sizeof buf, "Hello");
#else
          strcat(buf, "Hello");
#endif
          ByteBuffer src(buf, strlen(buf));

          CharsetDecoderPtr dec(CharsetDecoder::getDefaultDecoder());

          LogString greeting;
          log4cxx_status_t stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 0, src.remaining());


          stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);

          LogString manyAs(BUFSIZE - 3, LOG4CXX_STR('A'));
          LOGUNIT_ASSERT_EQUAL(manyAs, greeting.substr(0, BUFSIZE - 3));
          LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("Hello")), greeting.substr(BUFSIZE - 3));
        }



        void decode8() {
          char buf[] = { 'H', 'e', 'l', 'l', 'o', ',', 0, 'W', 'o', 'r', 'l', 'd'};
          ByteBuffer src(buf, 12);

          CharsetDecoderPtr dec(CharsetDecoder::getDefaultDecoder());
          LogString greeting;
          log4cxx_status_t stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);

          stat = dec->decode(src, greeting);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 12, src.position());

          const logchar expected[] = { 0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x2C, 0x00, 0x57, 0x6F, 0x72, 0x6C, 0x64 };
          LOGUNIT_ASSERT_EQUAL(LogString(expected, 12), greeting);
        }



};

LOGUNIT_TEST_SUITE_REGISTRATION(CharsetDecoderTestCase);
