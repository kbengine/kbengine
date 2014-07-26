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
#include <log4cxx/helpers/pool.h>
#include <apr_strings.h>
#include <log4cxx/helpers/exception.h>
#include <apr_pools.h>
#include <assert.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx::helpers;
using namespace log4cxx;


Pool::Pool() : pool(0), release(true) {
    apr_status_t stat = apr_pool_create(&pool, APRInitializer::getRootPool());
    if (stat != APR_SUCCESS) {
        throw PoolException(stat);
    }
}

Pool::Pool(apr_pool_t* p, bool release1) : pool(p), release(release1) {
    assert(p != NULL);
}

Pool::~Pool() {
    if (release) {
      apr_pool_destroy(pool);
    }
}


apr_pool_t* Pool::getAPRPool() {
   return pool;
}

apr_pool_t* Pool::create() {
    apr_pool_t* child;
    apr_status_t stat = apr_pool_create(&child, pool);
    if (stat != APR_SUCCESS) {
        throw PoolException(stat);
    }
    return child;
}

void* Pool::palloc(size_t size) {
  return apr_palloc(pool, size);
}

char* Pool::pstralloc(size_t size) {
  return (char*) palloc(size);
}

char* Pool::itoa(int n) {
    return apr_itoa(pool, n);
}

char* Pool::pstrndup(const char* s, size_t len) {
    return apr_pstrndup(pool, s, len);
}

char* Pool::pstrdup(const char* s) {
    return apr_pstrdup(pool, s);
}

char* Pool::pstrdup(const std::string& s) {
    return apr_pstrndup(pool, s.data(), s.length());
}
