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
#include <log4cxx/helpers/thread.h>
#include <log4cxx/helpers/exception.h>
#include <apr_thread_proc.h>
#include <apr_atomic.h>
#include <log4cxx/helpers/pool.h>
#include <log4cxx/helpers/threadlocal.h>

using namespace log4cxx::helpers;
using namespace log4cxx;

Thread::Thread() : thread(NULL), alive(0), interruptedStatus(0) {
}

Thread::~Thread() {
    join();
}

Thread::LaunchPackage::LaunchPackage(Thread* t, Runnable r, void* d) : thread(t), runnable(r), data(d) {
}

Thread* Thread::LaunchPackage::getThread() const {
    return thread;
}

Runnable Thread::LaunchPackage::getRunnable() const {
    return runnable;
}

void* Thread::LaunchPackage::getData() const {
    return data;
}

void* Thread::LaunchPackage::operator new(size_t sz, Pool& p) {
    return p.palloc(sz);
}

void Thread::LaunchPackage::operator delete(void* mem, Pool& p) {
}

void Thread::run(Runnable start, void* data) {
#if APR_HAS_THREADS
        //
        //    if attempting a second run method on the same Thread object
        //         throw an exception
        //
        if (thread != NULL) {
            throw IllegalStateException();
        }
        apr_threadattr_t* attrs;
        apr_status_t stat = apr_threadattr_create(&attrs, p.getAPRPool());
        if (stat != APR_SUCCESS) {
                throw ThreadException(stat);
        }
        
        //   create LaunchPackage on the thread's memory pool
        LaunchPackage* package = new(p) LaunchPackage(this, start, data);
        stat = apr_thread_create(&thread, attrs,
            launcher, package, p.getAPRPool());
        if (stat != APR_SUCCESS) {
                throw ThreadException(stat);
        }
#else
        throw ThreadException(LOG4CXX_STR("APR_HAS_THREADS is not true"));
#endif
}


Thread::LaunchStatus::LaunchStatus(volatile unsigned int* p) : alive(p) {
    apr_atomic_set32(alive, 0xFFFFFFFF);
}

Thread::LaunchStatus::~LaunchStatus() {
    apr_atomic_set32(alive, 0);
}
    
#if APR_HAS_THREADS
void* LOG4CXX_THREAD_FUNC Thread::launcher(apr_thread_t* thread, void* data) {
    LaunchPackage* package = (LaunchPackage*) data;
    ThreadLocal& tls = getThreadLocal();
    tls.set(package->getThread());
    LaunchStatus alive(&package->getThread()->alive);
    void* retval = (package->getRunnable())(thread, package->getData());
    apr_thread_exit(thread, 0);
    return retval;
}
#endif


void Thread::join() {
#if APR_HAS_THREADS
        if (thread != NULL) {
                apr_status_t startStat;
                apr_status_t stat = apr_thread_join(&startStat, thread);
                thread = NULL;
                if (stat != APR_SUCCESS) {
                        throw ThreadException(stat);
                }
        }
#endif
}

ThreadLocal& Thread::getThreadLocal() {
     static ThreadLocal tls;
     return tls;
}

void Thread::currentThreadInterrupt() {
#if APR_HAS_THREADS
   void* tls = getThreadLocal().get();
   if (tls != 0) {
       ((Thread*) tls)->interrupt();
   }
#endif
}

void Thread::interrupt() {
    apr_atomic_set32(&interruptedStatus, 0xFFFFFFFF);
}

bool Thread::interrupted() {
#if APR_HAS_THREADS
   void* tls = getThreadLocal().get();
   if (tls != 0) {
       return apr_atomic_xchg32(&(((Thread*) tls)->interruptedStatus), 0) != 0;
   }
#endif
   return false;
}

bool Thread::isCurrentThread() const {
#if APR_HAS_THREADS
    const void* tls = getThreadLocal().get();
    return (tls == this);
#else
    return true;
#endif
}

bool Thread::isAlive() {
    return apr_atomic_read32(&alive) != 0;
}

void Thread::ending() {
    apr_atomic_set32(&alive, 0);
}


void Thread::sleep(int duration) {
#if APR_HAS_THREADS
    if(interrupted()) {
         throw InterruptedException();
    }
#endif    
    if (duration > 0) {
        apr_sleep(duration*1000);
    }
}
