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
}


namespace log4cxx
{
        namespace helpers
        {
                class Pool;
                class ThreadLocal;

            typedef void* (LOG4CXX_THREAD_FUNC *Runnable)(apr_thread_t* thread, void* data);
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
                        Thread(const Thread&);
                        Thread& operator=(const Thread&);
                        
                        /**
                         *   This class is used to encapsulate the parameters to
                         *   Thread::run when they are passed to Thread::launcher.
                         *
                         */
                        class LaunchPackage {
                        public:
                            /**
                             *  Placement new to create LaunchPackage in specified pool.
                             *  LaunchPackage needs to be dynamically allocated since
                             *  since a stack allocated instance may go out of scope
                             *  before thread is launched.
                             */
                            static void* operator new(size_t, Pool& p);
                            /**
                            *  operator delete would be called if exception during construction.
                     */
                            static void operator delete(void*, Pool& p);
                            /**
                             *  Create new instance.
                             */
                            LaunchPackage(Thread* thread, Runnable runnable, void* data);
                            /**
                             * Gets thread parameter.
                             * @return thread.
                             */
                            Thread* getThread() const;
                            /**
                             *  Gets runnable parameter.
                             *  @return runnable.
                             */
                            Runnable getRunnable() const;
                            /**
                             *  gets data parameter.
                             *  @return thread.
                             */
                            void* getData() const;
                        private:
                            LaunchPackage(const LaunchPackage&);
                            LaunchPackage& operator=(const LaunchPackage&);
                            Thread* thread;
                            Runnable runnable; 
                            void* data;
                        };
                        
                        /**
                         *  This object atomically sets the specified memory location
                         *  to non-zero on construction and to zero on destruction.  
                         *  Used to maintain Thread.alive.
                         */
                        class LaunchStatus {
                        public:
                            /*
                             *  Construct new instance.
                             *  @param p address of memory to set to non-zero on construction, zero on destruction.
                             */
                            LaunchStatus(volatile unsigned int* p);
                            /**
                             *  Destructor.
                             */
                            ~LaunchStatus();
                        private:
                            LaunchStatus(const LaunchStatus&);
                            LaunchStatus& operator=(const LaunchStatus&);
                            volatile unsigned int* alive;
                        };
                        
                        /**
                         *  This method runs on the created thread and sets up thread-local storage
                         *  used to keep the reference to the corresponding Thread object and
                         *  is responsible for maintaining Thread.alive.
                         */
                        static void* LOG4CXX_THREAD_FUNC launcher(apr_thread_t* thread, void* data);
                        /**
                         *   Get a key to the thread local storage used to hold the reference to
                         *   the corresponding Thread object.
                         */                        
                        static ThreadLocal& getThreadLocal();
                };
        } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_THREAD_H
