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
#include <log4cxx/helpers/threadspecificdata.h>
#include <log4cxx/helpers/exception.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/helpers/aprinitializer.h>

using namespace log4cxx;
using namespace log4cxx::helpers;


ThreadSpecificData::ThreadSpecificData()
    : ndcStack(), mdcMap() {
}

ThreadSpecificData::~ThreadSpecificData() {
}


log4cxx::NDC::Stack& ThreadSpecificData::getStack() {
  return ndcStack;
}

log4cxx::MDC::Map& ThreadSpecificData::getMap() {
  return mdcMap;
}

ThreadSpecificData& ThreadSpecificData::getDataNoThreads() {
    static ThreadSpecificData noThreadData;
    return noThreadData;
}

ThreadSpecificData* ThreadSpecificData::getCurrentData() {
#if APR_HAS_THREADS
  void* pData = NULL;
  apr_threadkey_private_get(&pData, APRInitializer::getTlsKey());
  return (ThreadSpecificData*) pData;
#else
  return &getDataNoThreads();  
#endif
}

void ThreadSpecificData::recycle() {
#if APR_HAS_THREADS
    if(ndcStack.empty() && mdcMap.empty()) {
        void* pData = NULL;
        apr_status_t stat = apr_threadkey_private_get(&pData, APRInitializer::getTlsKey());
        if (stat == APR_SUCCESS && pData == this) {
            stat = apr_threadkey_private_set(0, APRInitializer::getTlsKey());
            if (stat == APR_SUCCESS) {
                delete this;
            }
        }
    }
#endif    
}

void ThreadSpecificData::put(const LogString& key, const LogString& val) {
    ThreadSpecificData* data = getCurrentData();
    if (data == 0) {
        data = createCurrentData();
    }
    if (data != 0) {
        data->getMap().insert(log4cxx::MDC::Map::value_type(key, val));
    }
}




void ThreadSpecificData::push(const LogString& val) {
    ThreadSpecificData* data = getCurrentData();
    if (data == 0) {
        data = createCurrentData();
    }
    if (data != 0) {
        NDC::Stack& stack = data->getStack();
        if(stack.empty()) {
            stack.push(NDC::DiagnosticContext(val, val));
        } else {
            LogString fullMessage(stack.top().second);
            fullMessage.append(1, (logchar) 0x20);
            fullMessage.append(val);
            stack.push(NDC::DiagnosticContext(val, fullMessage));
        }
    }
}

void ThreadSpecificData::inherit(const NDC::Stack& src) {
    ThreadSpecificData* data = getCurrentData();
    if (data == 0) {
        data = createCurrentData();
    }
    if (data != 0) {
        data->getStack() = src;
    }
}



ThreadSpecificData* ThreadSpecificData::createCurrentData() {
#if APR_HAS_THREADS
    ThreadSpecificData* newData = new ThreadSpecificData();
    apr_status_t stat = apr_threadkey_private_set(newData, APRInitializer::getTlsKey());
    if (stat != APR_SUCCESS) {
      delete newData;
      newData = NULL;
    }
    return newData;
#else
    return 0;
#endif
}
