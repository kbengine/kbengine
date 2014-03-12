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
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/condition.h>
#include <log4cxx/helpers/exception.h>

#include <apr_thread_cond.h>
#include <log4cxx/helpers/synchronized.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/thread.h>

using namespace log4cxx::helpers;
using namespace log4cxx;


Condition::Condition(Pool& p)
{
#if APR_HAS_THREADS
        apr_status_t stat = apr_thread_cond_create(&condition, p.getAPRPool());
        if (stat != APR_SUCCESS) {
                throw RuntimeException(stat);
        }
#endif
}

Condition::~Condition()
{
#if APR_HAS_THREADS
        apr_thread_cond_destroy(condition);
#endif
}

log4cxx_status_t Condition::signalAll()
{
#if APR_HAS_THREADS
        return apr_thread_cond_broadcast(condition);
#else
      return APR_SUCCESS;
#endif
}

void Condition::await(Mutex& mutex)
{
#if APR_HAS_THREADS
        if (Thread::interrupted()) {
             throw InterruptedException();
        }
        apr_status_t stat = apr_thread_cond_wait(
             condition,
             mutex.getAPRMutex());
        if (stat != APR_SUCCESS) {
                throw InterruptedException(stat);
        }
#endif
}

