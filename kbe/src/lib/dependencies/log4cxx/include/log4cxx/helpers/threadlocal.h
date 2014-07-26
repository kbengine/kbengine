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

#ifndef _LOG4CXX_HELPERS_THREAD_LOCAL_H
#define _LOG4CXX_HELPERS_THREAD_LOCAL_H

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
    struct apr_threadkey_t;
}

namespace log4cxx
{
        namespace helpers
        {

                /**
                 *  This class provides thread-local variables.  This class is similar in function
                 *  to java.lang.ThreadLocal.
                 */
                class LOG4CXX_EXPORT ThreadLocal {
                public:
                    /**
                     *   Create new instance.
                     */
                    ThreadLocal();
                    /**
                     *    Destructor.
                     */
                    ~ThreadLocal();
                    /**
                     *  Sets the value in the current thread's copy of this thread-local variable.
                     *  @param priv new value.
                     */
                    void set(void* priv);                    
                    /**
                     *  Returns the value in the current thread's copy of this thread-local variable.
                     *  @return value of thread-local variable for the current thread.
                     */
                    void* get();
               
                private:
                    /**
                     * Prevent use of default copy constructor.
                     */
                     ThreadLocal(const ThreadLocal&);
                    /**
                     *   Prevent use of default assignment operator.  
                     */
                     ThreadLocal& operator=(const ThreadLocal&);
                     
                     static apr_threadkey_t* create(Pool& p);

                Pool p;
                     apr_threadkey_t* key;
                };
        } // namespace helpers
} // namespace log4cxx

#endif //_LOG4CXX_HELPERS_THREAD_LOCAL_H
