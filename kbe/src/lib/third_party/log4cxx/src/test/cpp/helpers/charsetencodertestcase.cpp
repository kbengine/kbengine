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

#include <log4cxx/helpers/charsetencoder.h>
#include "../logunit.h"
#include "../insertwide.h"
#include <log4cxx/helpers/bytebuffer.h>
#include <log4cxx/helpers/thread.h>
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/condition.h>
#include <log4cxx/helpers/synchronized.h>
#include <apr.h>
#include <apr_atomic.h>


using namespace log4cxx;
using namespace log4cxx::helpers;


LOGUNIT_CLASS(CharsetEncoderTestCase)
{
        LOGUNIT_TEST_SUITE(CharsetEncoderTestCase);
                LOGUNIT_TEST(encode1);
                LOGUNIT_TEST(encode2);
                LOGUNIT_TEST(encode3);
                LOGUNIT_TEST(encode4);
#if APR_HAS_THREADS        
                LOGUNIT_TEST(thread1);
#endif                
        LOGUNIT_TEST_SUITE_END();

        enum { BUFSIZE = 256 };

public:


        void encode1() {
          const LogString greeting(LOG4CXX_STR("Hello, World"));
          CharsetEncoderPtr enc(CharsetEncoder::getEncoder(LOG4CXX_STR("US-ASCII")));
          char buf[BUFSIZE];
          ByteBuffer out(buf, BUFSIZE);
          LogString::const_iterator iter = greeting.begin();
          log4cxx_status_t stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT(iter == greeting.end());

          stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 12, out.position());

          out.flip();
          std::string encoded((const char*) out.data(), out.limit());
          LOGUNIT_ASSERT_EQUAL((std::string) "Hello, World", encoded);
          LOGUNIT_ASSERT(iter == greeting.end());
        }

        void encode2() {
          LogString greeting(BUFSIZE - 3, LOG4CXX_STR('A'));
          greeting.append(LOG4CXX_STR("Hello"));

          CharsetEncoderPtr enc(CharsetEncoder::getEncoder(LOG4CXX_STR("US-ASCII")));

          char buf[BUFSIZE];
          ByteBuffer out(buf, BUFSIZE);
          LogString::const_iterator iter = greeting.begin();
          log4cxx_status_t stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 0, out.remaining());
          LOGUNIT_ASSERT_EQUAL(LOG4CXX_STR('o'), *(iter+1));

          out.flip();
          std::string encoded((char*) out.data(), out.limit());
          out.clear();

          stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          LOGUNIT_ASSERT_EQUAL((size_t) 2, out.position());
          LOGUNIT_ASSERT(iter == greeting.end());

          stat = enc->encode(greeting, iter, out);
          out.flip();
          LOGUNIT_ASSERT_EQUAL(APR_SUCCESS, stat);
          encoded.append(out.data(), out.limit());

          std::string manyAs(BUFSIZE - 3, 'A');
          LOGUNIT_ASSERT_EQUAL(manyAs, encoded.substr(0, BUFSIZE - 3));
          LOGUNIT_ASSERT_EQUAL(std::string("Hello"), encoded.substr(BUFSIZE - 3));
        }


        void encode3() {
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_LOGCHAR_IS_UNICHAR
          //   arbitrary, hopefully meaningless, characters from
          //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
          const logchar greet[] = { L'A', 0x0605, 0x0530, 0x986, 0x4E03, 0x400, 0 };
#endif

#if LOG4CXX_LOGCHAR_IS_UTF8
          const char greet[] = { 'A',
                                    (char) 0xD8, (char) 0x85,
                                    (char) 0xD4, (char) 0xB0,
                                    (char) 0xE0, (char) 0xA6, (char) 0x86,
                                    (char) 0xE4, (char) 0xB8, (char) 0x83,
                                    (char) 0xD0, (char) 0x80,
                                    0 };
#endif
          LogString greeting(greet);

          CharsetEncoderPtr enc(CharsetEncoder::getEncoder(LOG4CXX_STR("US-ASCII")));

          char buf[BUFSIZE];
          ByteBuffer out(buf, BUFSIZE);

          LogString::const_iterator iter = greeting.begin();
          log4cxx_status_t stat = enc->encode(greeting, iter, out);
          out.flip();
          LOGUNIT_ASSERT_EQUAL(true, CharsetEncoder::isError(stat));
          LOGUNIT_ASSERT_EQUAL((size_t) 1, out.limit());
          LOGUNIT_ASSERT_EQUAL(greet[1], *iter);
          LOGUNIT_ASSERT_EQUAL('A', out.data()[0]);
        }


        void encode4() {
          const char utf8_greet[] = { 'A',
                                    (char) 0xD8, (char) 0x85,
                                    (char) 0xD4, (char) 0xB0,
                                    (char) 0xE0, (char) 0xA6, (char) 0x86,
                                    (char) 0xE4, (char) 0xB8, (char) 0x83,
                                    (char) 0xD0, (char) 0x80,
                                    0 };
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_LOGCHAR_IS_UNICHAR
          //   arbitrary, hopefully meaningless, characters from
          //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
          const logchar greet[] = { L'A', 0x0605, 0x0530, 0x986, 0x4E03, 0x400, 0 };
#endif

#if LOG4CXX_LOGCHAR_IS_UTF8
          const logchar *greet = utf8_greet;
#endif
          LogString greeting(greet);

          CharsetEncoderPtr enc(CharsetEncoder::getEncoder(LOG4CXX_STR("UTF-8")));

          char buf[BUFSIZE];
          ByteBuffer out(buf, BUFSIZE);
          LogString::const_iterator iter = greeting.begin();
          log4cxx_status_t stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(false, CharsetEncoder::isError(stat));
          stat = enc->encode(greeting, iter, out);
          LOGUNIT_ASSERT_EQUAL(false, CharsetEncoder::isError(stat));

          out.flip();
          LOGUNIT_ASSERT_EQUAL((size_t) 13, out.limit());
          for(size_t i = 0; i < out.limit(); i++) {
             LOGUNIT_ASSERT_EQUAL((int) utf8_greet[i], (int) out.data()[i]);
          }
          LOGUNIT_ASSERT(iter == greeting.end());
        }
        
#if APR_HAS_THREADS        
        class ThreadPackage {
        public:
            ThreadPackage(CharsetEncoderPtr& enc, int repetitions) : 
                p(), lock(p), condition(p), passCount(0), failCount(0), enc(enc), repetitions(repetitions) {
            }
            
            void await() {
                synchronized sync(lock);
                condition.await(lock);
            }
            
            void signalAll() {
                synchronized sync(lock);
                condition.signalAll();
            }
            
            void fail() {
                apr_atomic_inc32(&failCount);
            }
            
            void pass() {
                apr_atomic_inc32(&passCount);
            }
            
            apr_uint32_t getFail() {
               return apr_atomic_read32(&failCount);
            }
            
            apr_uint32_t getPass() {
               return apr_atomic_read32(&passCount);
            }
            
            int getRepetitions() {
                return repetitions;
            }
            
            CharsetEncoderPtr& getEncoder() {
               return enc;
            }
            
        private:
            ThreadPackage(const ThreadPackage&);
            ThreadPackage& operator=(ThreadPackage&);
            Pool p;
            Mutex lock;
            Condition condition;
            volatile apr_uint32_t passCount;
            volatile apr_uint32_t failCount; 
            CharsetEncoderPtr enc;
            int repetitions;
        };
        
        static void* LOG4CXX_THREAD_FUNC thread1Action(apr_thread_t* /* thread */, void* data) {
            ThreadPackage* package = (ThreadPackage*) data;
#if LOG4CXX_LOGCHAR_IS_UTF8
            const logchar greet[] = { 'H', 'e', 'l', 'l', 'o', ' ',
                                    (char) 0xC2, (char) 0xA2,  //  cent sign
                                    (char) 0xC2, (char) 0xA9,  //  copyright
                                    (char) 0xc3, (char) 0xb4,  //  latin small letter o with circumflex
                                    0 };
#endif
#if LOG4CXX_LOGCHAR_IS_WCHAR || LOG4CXX_LOGCHAR_IS_UNICHAR
            //   arbitrary, hopefully meaningless, characters from
            //     Latin, Arabic, Armenian, Bengali, CJK and Cyrillic
            const logchar greet[] = { L'H', L'e', L'l', L'l', L'o', L' ',
                0x00A2, 0x00A9, 0x00F4 , 0 };
#endif
          
          const char expected[] =  { 'H', 'e', 'l', 'l', 'o', ' ',
                (char) 0x00A2, (char) 0x00A9, (char) 0x00F4 };

          LogString greeting(greet);

          package->await();
          for(int i = 0; i < package->getRepetitions(); i++) {
            bool pass = true;
            char buf[BUFSIZE];
            ByteBuffer out(buf, BUFSIZE);
            LogString::const_iterator iter = greeting.begin();
            log4cxx_status_t stat = package->getEncoder()->encode(greeting, iter, out);
            pass = (false == CharsetEncoder::isError(stat));
            if (pass) {
                stat = package->getEncoder()->encode(greeting, iter, out);
                pass = (false == CharsetEncoder::isError(stat));
                if (pass) {
                    out.flip();
                    pass = (sizeof(expected) == out.limit());
                    for(size_t i = 0; i < out.limit() && pass; i++) {
                        pass = (expected[i] == out.data()[i]);
                    }
                    pass = pass && (iter == greeting.end());
                }
            }
            if (pass) {
                package->pass();
            } else {
                package->fail();
            }
          }
            return 0;
        }

        void thread1() {
              enum { THREAD_COUNT = 10, THREAD_REPS = 10000 };
              Thread threads[THREAD_COUNT];
              CharsetEncoderPtr enc(CharsetEncoder::getEncoder(LOG4CXX_STR("ISO-8859-1")));
              ThreadPackage* package = new ThreadPackage(enc, THREAD_REPS);
              { for(int i = 0; i < THREAD_COUNT; i++) {
                  threads[i].run(thread1Action, package);
              } }
              //
              //   give time for all threads to be launched so
              //      we don't signal before everybody is waiting.
              Thread::sleep(100);
              package->signalAll();
              for(int i = 0; i < THREAD_COUNT; i++) {
                  threads[i].join();
              }
              LOGUNIT_ASSERT_EQUAL((apr_uint32_t) 0, package->getFail());
              LOGUNIT_ASSERT_EQUAL((apr_uint32_t) THREAD_COUNT * THREAD_REPS, package->getPass());
              delete package;
        }
#endif

};

LOGUNIT_TEST_SUITE_REGISTRATION(CharsetEncoderTestCase);
