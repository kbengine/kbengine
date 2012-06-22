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

#ifndef _LOG4CXX_HELPERS_THREAD_H
#define _LOG4CXX_HELPERS_THREAD_H

#include <log4cxx/log4cxx.h>
#include <log4cxx/helpers/pool.h>

#if !defined(LOG4CXX_THREAD_FUNC)
#if defined(_WIN32)
#define LOG4CXX_THREAD_FUNC __stdcall
#else
#define LOG4CXX_THREAD_FUNC
#endif
#endif

extern "C" {
    typedef struct apr_thread_t apr_thread_t;
    typedef struct apr_thread_cond_t apr_thread_cond_t;
    typedef struct apr_thread_mutex_t apr_thread_mutex_t;
}


namespace log4cxx
{
        namespace helpers
        {
                class Pool;
                class ThreadLocal;

                typedef void* (LOG4CXX_THREAD_FUNC *Runnable)(apr_thread_t* thread, void* data);
                namespace ThreadLaunch {
		    extern "C" void* LOG4CXX_THREAD_FUNC launcher(apr_thread_t* thread, void* data);
                }

                /**
                 *  This class implements an approximation of java.util.Thread.
                 */
                class LOG4CXX_EXPORT Thread
                {
                public:
                        /**
                         *  Create new instance.
                         */
                        Thread();
                        /**
                         *  Destructor.
                         */
                        ~Thread();

                        /**
                         *  Runs the specified method on a newly created thread.
                         */
                        void run(Runnable start, void* data);
                        void join();

                        inline bool isActive() { return thread != 0; }

                        /**
                         * Causes the currently executing thread to sleep for the
                         * specified number of milliseconds.
                         * @param millis milliseconds.
                         * @throws Interrupted Exception if the thread is interrupted.
                         */
                        static void sleep(int millis);
                        /**
                         *  Sets interrupted status for current thread to true.  
                         */
                        static void currentThreadInterrupt();
                        /**
                         *  Sets interrupted status to true.  
                         */
                        void interrupt();
                        /**
                         *  Tests if the current thread has been interrupted and
                         *  sets the interrupted status to false.
                         */
                        static bool interrupted();
                        
                        bool isAlive();
                        bool isCurrentThread() const;
                        void ending();
                        

                private:
                        Pool p;
                        apr_thread_t* thread;
                        volatile unsigned int alive;
                        volatile unsigned int interruptedStatus;
                        apr_thread_mutex_t* interruptedMutex;
                        apr_thread_cond_t* interruptedCondition;
                        Thread(const Thread&);
                        Thread& operator=(const Thread&);
                        friend void* LOG4CXX_THREAD_FUNC ThreadLaunch::launcher(apr_thread_t* thread, void* data); 
                };
        } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_THREAD_H
