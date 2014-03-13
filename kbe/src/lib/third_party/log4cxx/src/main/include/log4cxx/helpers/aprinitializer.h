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

#ifndef _LOG4CXX_HELPERS_APRINITIALIZER_H
#define _LOG4CXX_HELPERS_APRINITIALIZER_H

#ifndef LOG4CXX
#error "aprinitializer.h should only be included by log4cxx implementation"
#endif

#include <log4cxx/helpers/pool.h>
#include <apr_pools.h>
#include <apr_thread_proc.h>

namespace log4cxx
{
  namespace helpers
  {
    class APRInitializer
    {
    public:
    static log4cxx_time_t initialize();
    static apr_pool_t* getRootPool();
    static apr_threadkey_t* getTlsKey();
    static bool isDestructed;

    private:
      APRInitializer();
      APRInitializer(const APRInitializer&);
      APRInitializer& operator=(const APRInitializer&);
      apr_pool_t* p;
      log4cxx_time_t startTime;
      apr_threadkey_t* tlsKey;
      static APRInitializer& getInstance();
      static void tlsDestruct(void*);

    public:
      ~APRInitializer();
    };
  } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_APRINITIALIZER_H
