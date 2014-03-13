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

#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/helpers/exception.h>
#include <apr_atomic.h>

using namespace log4cxx::helpers;

ObjectPtrBase::ObjectPtrBase() {
}

ObjectPtrBase::~ObjectPtrBase() {
}

void ObjectPtrBase::checkNull(const int& null) {
    if (null != 0) {
       throw IllegalArgumentException(LOG4CXX_STR("Attempt to set pointer to a non-zero numeric value."));
    }
}

void* ObjectPtrBase::exchange(void** destination, void* newValue) {
#if _WIN32 && (!defined(_MSC_VER) || _MSC_VER >= 1300)
    return InterlockedExchangePointer(destination, newValue);
#elif APR_SIZEOF_VOIDP == 4
   return (void*) apr_atomic_xchg32((volatile apr_uint32_t*) destination,
                          (apr_uint32_t) newValue);
#else
   void* oldValue = *destination;
   *destination = newValue;
   return oldValue;
#endif
}
