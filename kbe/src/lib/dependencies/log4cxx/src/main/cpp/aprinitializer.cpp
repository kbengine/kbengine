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
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>
#include <apr_pools.h>
#include <apr_atomic.h>
#include <apr_time.h>
#include <assert.h>
#include <log4cxx/helpers/threadspecificdata.h>

using namespace log4cxx::helpers;
using namespace log4cxx;

bool APRInitializer::isDestructed = false;

APRInitializer::APRInitializer() {
    apr_initialize();
    apr_pool_create(&p, NULL);
    apr_atomic_init(p);
    startTime = apr_time_now();
#if APR_HAS_THREADS
    apr_status_t stat = apr_threadkey_private_create(&tlsKey, tlsDestruct, p);
    assert(stat == APR_SUCCESS);
#endif
}

APRInitializer::~APRInitializer() {
    apr_terminate();
    isDestructed = true;
}

APRInitializer& APRInitializer::getInstance() {
  static APRInitializer init;
  return init;
}


log4cxx_time_t APRInitializer::initialize() {
  return getInstance().startTime;
}

apr_pool_t* APRInitializer::getRootPool() {
  return getInstance().p;
}

apr_threadkey_t* APRInitializer::getTlsKey() {
   return getInstance().tlsKey;
}

void APRInitializer::tlsDestruct(void* ptr) {
  delete ((ThreadSpecificData*) ptr);
}
