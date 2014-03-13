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

#ifndef _LOG4CXX_HELPERS_POOL_H
#define _LOG4CXX_HELPERS_POOL_H

#include <log4cxx/log4cxx.h>
#include <string>

extern "C" {
   struct apr_pool_t;
}

namespace log4cxx
{
        namespace helpers
        {
                class LOG4CXX_EXPORT Pool
                {
                public:
                        Pool();
                        Pool(apr_pool_t* pool, bool release);
                        ~Pool();

                        apr_pool_t* getAPRPool();
                        apr_pool_t* create();
                        void* palloc(size_t length);
                        char* pstralloc(size_t length);
                        char* itoa(int n);
                        char* pstrndup(const char* s, size_t len);
                        char* pstrdup(const char*s);
                        char* pstrdup(const std::string&);

                protected:
                        apr_pool_t* pool;
                        const bool release;

                private:
                        Pool(const log4cxx::helpers::Pool&);
                        Pool& operator=(const Pool&);
                };
        } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_POOL_H
