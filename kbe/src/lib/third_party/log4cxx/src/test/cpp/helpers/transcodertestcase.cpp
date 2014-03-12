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

#include <log4cxx/helpers/transcoder.h>
#include "../insertwide.h"
#include "../logunit.h"


using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(TranscoderTestCase)
{
        LOGUNIT_TEST_SUITE(TranscoderTestCase);
                LOGUNIT_TEST(decode1);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(decode2);
#endif
                LOGUNIT_TEST(decode3);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(decode4);
#endif
                LOGUNIT_TEST(decode7);
                LOGUNIT_TEST(decode8);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(encode1);
#endif
                LOGUNIT_TEST(encode2);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(encode3);
#endif
                LOGUNIT_TEST(encode4);
#if LOG4CXX_WCHAR_T_API
                LOGUNIT_TEST(encode5);
#endif
                LOGUNIT_TEST(encode6);
                LOGUNIT_TEST(testDecodeUTF8_1);
                LOGUNIT_TEST(testDecodeUTF8_2);
                LOGUNIT_TEST(testDecodeUTF8_3);
                LOGUNIT_TEST(testDecodeUTF8_4);
#if LOG4CXX_UNICHAR_API
                LOGUNIT_TEST(udecode2);
                LOGUNIT_TEST(udecode4);
                LOGUNIT_TEST(uencode1);
                LOGUNIT_TEST(uencode3);
                LOGUNIT_TEST(uencode5);
#endif
                
        LOGUNIT_TEST_SUITE_END();


public:
        void decode1() {
          const char* greeting = "Hello, World";
          LogString decoded(LOG4CXX_STR("foo\n"));
          Transcoder::decode(greeting, decoded);
          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\nHello, World"), decoded);
        }

#if LOG4CXX_WCHAR_T_API
        void decode2() {
          const wchar_t* greeting = L"Hello, World";
          LogString decoded(LOG4CXX_STR("foo\n"));
          Transcoder::decode(greeting, decoded);
          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\nHello, World"), decoded);
        }
#endif

        void decode3() {
           const char* nothing = "";
           LogString decoded(LOG4CXX_STR("foo\n"));
           Transcoder::decode(nothing, decoded);
           LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\n"), decoded);
        }

#if LOG4CXX_WCHAR_T_API
        void decode4() {
            const wchar_t* nothing = L"";
            LogString decoded(LOG4CXX_STR("foo\n"));
            Transcoder::decode(nothing, decoded);
            LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\n"), decoded);
        }
#endif


        enum { BUFSIZE = 255 };

        void decode7() {
            //
            //   normal characters striding over a buffer boundary
            //
            std::string longMsg(BUFSIZE - 2, 'A');
            longMsg.append("Hello");
            LogString decoded;
            Transcoder::decode(longMsg, decoded);
            LOGUNIT_ASSERT_EQUAL((size_t) BUFSIZE + 3, decoded.length());
            LOGUNIT_ASSERT_EQUAL(LogString(BUFSIZE -2, LOG4CXX_STR('A')),
                  decoded.substr(0, BUFSIZE - 2));
            LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("Hello"),
                  decoded.substr(BUFSIZE -2 ));
        }

        void decode8() {
            std::string msg("Hello, World.");
            LogString actual;
            Transcoder::decode(msg, actual);
            LogString expected(LOG4CXX_STR("Hello, World."));
            LOGUNIT_ASSERT_EQUAL(expected, actual);
        }


#if LOG4CXX_WCHAR_T_API
        void encode1() {
          const LogString greeting(LOG4CXX_STR("Hello, World"));
          std::wstring encoded;
          Transcoder::encode(greeting, encoded);
          LOGUNIT_ASSERT_EQUAL((std::wstring) L"Hello, World", encoded);
        }
#endif

        void encode2() {
          const LogString greeting(LOG4CXX_STR("Hello, World"));
          std::string encoded;
          Transcoder::encode(greeting, encoded);
          LOGUNIT_ASSERT_EQUAL((std::string) "Hello, World", encoded);
        }

#if LOG4CXX_WCHAR_T_API
        void encode3() {
          LogString greeting(BUFSIZE - 3, LOG4CXX_STR('A'));
          greeting.append(LOG4CXX_STR("Hello"));
          std::wstring encoded;
          Transcoder::encode(greeting, encoded);
          std::wstring manyAs(BUFSIZE - 3, L'A');
          LOGUNIT_ASSERT_EQUAL(manyAs, encoded.substr(0, BUFSIZE - 3));
          LOGUNIT_ASSERT_EQUAL(std::wstring(L"Hello"), encoded.substr(BUFSIZE - 3));
        }
#endif

        void encode4() {
          LogString greeting(BUFSIZE - 3, LOG4CXX_STR('A'));
          greeting.append(LOG4CXX_STR("Hello"));
          std::string encoded;
          Transcoder::encode(greeting, encoded);
          std::string manyAs(BUFSIZE - 3, 'A');
          LOGUNIT_ASSERT_EQUAL(manyAs, encoded.substr(0, BUFSIZE - 3));
          LOGUNIT_ASSERT_EQUAL(std::string("Hello"), encoded.substr(BUFSIZE - 3));
        }

#if LOG4CXX_WCHAR_T_API
        void encode5() {
          //   arbitrary, hopefully meaningless, characters from
          //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
          const wchar_t greeting[] = { L'A', 0x0605, 0x0530, 0x984, 0x40E3, 0x400, 0 };
          //
          //  decode to LogString (UTF-16 or UTF-8)
          //
          LogString decoded;
          Transcoder::decode(greeting, decoded);
          //
          //  decode to wstring
          //
          std::wstring encoded;
          Transcoder::encode(decoded, encoded);
          //
          //   should be lossless
          //
          LOGUNIT_ASSERT_EQUAL((std::wstring) greeting, encoded);
        }
#endif

        void encode6() {
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_LOGCHAR_IS_UNICHAR
          //   arbitrary, hopefully meaningless, characters from
          //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
          const logchar greeting[] = { L'A', 0x0605, 0x0530, 0x984, 0x40E3, 0x400, 0 };
#endif

#if LOG4CXX_LOGCHAR_IS_UTF8
          const char greeting[] = { 'A',
                                    (char) 0xD8, (char) 0x85,
                                    (char) 0xD4, (char) 0xB0,
                                    (char) 0xE0, (char) 0xCC, (char) 0x84,
                                    (char) 0xE8, (char) 0x87, (char) 0x83,
                                    (char) 0xD0, (char) 0x80,
                                    0 };
#endif

          //
          //  decode to LogString (UTF-16 or UTF-8)
          //
          LogString decoded;
          Transcoder::decode(greeting, decoded);
          //
          //  decode to wstring
          //
          std::string encoded;
          //
          //   likely 'A\u0605\u0530\u0984\u40E3\u0400'
          //
          Transcoder::encode(decoded, encoded);
        }

    void testDecodeUTF8_1() {
        std::string src("a");
        LogString out;
        Transcoder::decodeUTF8(src, out);
        LOGUNIT_ASSERT_EQUAL(LogString(LOG4CXX_STR("a")), out);
    }

    void testDecodeUTF8_2() {
        std::string src(1, 0x80);
        LogString out;
        Transcoder::decodeUTF8(src, out);
        LOGUNIT_ASSERT_EQUAL(LogString(1, Transcoder::LOSSCHAR), out);
    }

    void testDecodeUTF8_3() {
        std::string src("\xC2");
        LogString out;
        Transcoder::decodeUTF8(src, out);
        LOGUNIT_ASSERT_EQUAL(LogString(1, Transcoder::LOSSCHAR), out);
    }

    void testDecodeUTF8_4() {
        std::string src("\xC2\xA9");
        LogString out;
        Transcoder::decodeUTF8(src, out);
        LogString::const_iterator iter = out.begin();
        unsigned int sv = Transcoder::decode(out, iter);
        LOGUNIT_ASSERT_EQUAL((unsigned int) 0xA9, sv);
        LOGUNIT_ASSERT_EQUAL(true, iter == out.end());
    }


#if LOG4CXX_UNICHAR_API
        void udecode2() {
          const UniChar greeting[] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
          LogString decoded(LOG4CXX_STR("foo\n"));
          Transcoder::decode(greeting, decoded);
          LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\nHello, World"), decoded);
        }

        void udecode4() {
            const UniChar nothing[] = { 0 };
            LogString decoded(LOG4CXX_STR("foo\n"));
            Transcoder::decode(nothing, decoded);
            LOGUNIT_ASSERT_EQUAL((LogString) LOG4CXX_STR("foo\n"), decoded);
        }

        void uencode1() {
          const LogString greeting(LOG4CXX_STR("Hello, World"));
          std::basic_string<UniChar> encoded;
          Transcoder::encode(greeting, encoded);
          const UniChar expected[] = { 'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', 0 };
          LOGUNIT_ASSERT_EQUAL(std::basic_string<UniChar>(expected), encoded);
        }

        void uencode3() {
          LogString greeting(BUFSIZE - 3, LOG4CXX_STR('A'));
          greeting.append(LOG4CXX_STR("Hello"));
          std::basic_string<UniChar> encoded;
          Transcoder::encode(greeting, encoded);
          std::basic_string<UniChar> manyAs(BUFSIZE - 3, 'A');
          LOGUNIT_ASSERT_EQUAL(manyAs, encoded.substr(0, BUFSIZE - 3));
          const UniChar hello[] = { 'H', 'e', 'l', 'l', 'o', 0 };
          LOGUNIT_ASSERT_EQUAL(std::basic_string<UniChar>(hello), encoded.substr(BUFSIZE - 3));
        }

        void uencode5() {
          //   arbitrary, hopefully meaningless, characters from
          //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
          const UniChar greeting[] = { L'A', 0x0605, 0x0530, 0x984, 0x40E3, 0x400, 0 };
          //
          //  decode to LogString (UTF-16 or UTF-8)
          //
          LogString decoded;
          Transcoder::decode(greeting, decoded);
          //
          //  decode to basic_string<UniChar>
          //
          std::basic_string<UniChar> encoded;
          Transcoder::encode(decoded, encoded);
          //
          //   should be lossless
          //
          LOGUNIT_ASSERT_EQUAL(std::basic_string<UniChar>(greeting), encoded);
        }
#endif


};

LOGUNIT_TEST_SUITE_REGISTRATION(TranscoderTestCase);
