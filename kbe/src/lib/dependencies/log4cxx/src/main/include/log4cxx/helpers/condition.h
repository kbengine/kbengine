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

#ifndef _LOG4CXX_HELPERS_CONDITION_H
#define _LOG4CXX_HELPERS_CONDITION_H

#include <log4cxx/log4cxx.h>
#include <log4cxx/helpers/mutex.h>

extern "C" {
   struct apr_thread_cond_t;
}

namespace log4cxx
{
        namespace helpers
        {
                class Pool;

                /**
                 *   This class provides a means for one thread to suspend exception until
                 *   notified by another thread to resume.  This class should have 
                 *   similar semantics to java.util.concurrent.locks.Condition.
                 */
                class LOG4CXX_EXPORT Condition
                {
                public:
                        /**
                         *  Create new instance.
                         *  @param p pool on which condition will be created.  Needs to be
                         *  longer-lived than created instance.
                         */
                        Condition(log4cxx::helpers::Pool& p);
                        /**
                         *  Destructor.
                         */
                        ~Condition();
                        /**
                         *   Signal all waiting threads.
                         */
                        log4cxx_status_t signalAll();
                        /**
                         *  Await signaling of condition.
                         *  @param lock lock associated with condition, calling thread must
                         *  own lock.  Lock will be released while waiting and reacquired
                         *  before returning from wait.
                         *  @throws InterruptedException if thread is interrupted.
                         */
                        void await(Mutex& lock);

                private:
                        apr_thread_cond_t* condition;
                        Condition(const Condition&);
                        Condition& operator=(const Condition&);
                };
        } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_CONDITION_H
