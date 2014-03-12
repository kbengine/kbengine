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
#include <log4cxx/helpers/mutex.h>
#include <log4cxx/helpers/exception.h>
#include <apr_thread_mutex.h>
#include <assert.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx::helpers;
using namespace log4cxx;


Mutex::Mutex(Pool& p) {
#if APR_HAS_THREADS
        apr_status_t stat = apr_thread_mutex_create(&mutex,
                APR_THREAD_MUTEX_NESTED, p.getAPRPool());
        if (stat != APR_SUCCESS) {
                throw MutexException(stat);
        }
#endif
}

Mutex::Mutex(apr_pool_t* p) {
#if APR_HAS_THREADS
        apr_status_t stat = apr_thread_mutex_create(&mutex,
                APR_THREAD_MUTEX_NESTED, p);
        if (stat != APR_SUCCESS) {
                throw MutexException(stat);
        }
#endif
}


Mutex::~Mutex() {
#if APR_HAS_THREADS
        apr_thread_mutex_destroy(mutex);
#endif
}

apr_thread_mutex_t* Mutex::getAPRMutex() const {
    return mutex;
}
