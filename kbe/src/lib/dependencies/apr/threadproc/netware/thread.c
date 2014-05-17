/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr.h"
#include "apr_portable.h"
#include "apr_strings.h"
#include "apr_arch_threadproc.h"

static int thread_count = 0;

apr_status_t apr_threadattr_create(apr_threadattr_t **new,
                                                apr_pool_t *pool)
{
    (*new) = (apr_threadattr_t *)apr_palloc(pool, 
              sizeof(apr_threadattr_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }

    (*new)->pool = pool;
    (*new)->stack_size = APR_DEFAULT_STACK_SIZE;
    (*new)->detach = 0;
    (*new)->thread_name = NULL;
    return APR_SUCCESS;
}

apr_status_t apr_threadattr_detach_set(apr_threadattr_t *attr,apr_int32_t on)
{
    attr->detach = on;
    return APR_SUCCESS;   
}

apr_status_t apr_threadattr_detach_get(apr_threadattr_t *attr)
{
    if (attr->detach == 1)
        return APR_DETACH;
    return APR_NOTDETACH;
}

APR_DECLARE(apr_status_t) apr_threadattr_stacksize_set(apr_threadattr_t *attr,
                                                       apr_size_t stacksize)
{
    attr->stack_size = stacksize;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_threadattr_guardsize_set(apr_threadattr_t *attr,
                                                       apr_size_t size)
{
    return APR_ENOTIMPL;
}

static void *dummy_worker(void *opaque)
{
    apr_thread_t *thd = (apr_thread_t *)opaque;
    return thd->func(thd, thd->data);
}

apr_status_t apr_thread_create(apr_thread_t **new,
                               apr_threadattr_t *attr, 
                               apr_thread_start_t func,
                               void *data,
                               apr_pool_t *pool)
{
    apr_status_t stat;
    long flags = NX_THR_BIND_CONTEXT;
    char threadName[NX_MAX_OBJECT_NAME_LEN+1];
    size_t stack_size = APR_DEFAULT_STACK_SIZE;

    if (attr && attr->thread_name) {
        strncpy (threadName, attr->thread_name, NX_MAX_OBJECT_NAME_LEN);
    }
    else {
        sprintf(threadName, "APR_thread %04ld", ++thread_count);
    }

    /* An original stack size of 0 will allow NXCreateThread() to
    *   assign a default system stack size.  An original stack
    *   size of less than 0 will assign the APR default stack size.
    *   anything else will be taken as is.
    */
    if (attr && (attr->stack_size >= 0)) {
        stack_size = attr->stack_size;
    }
    
    (*new) = (apr_thread_t *)apr_palloc(pool, sizeof(apr_thread_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }
    
    (*new)->data = data;
    (*new)->func = func;
    (*new)->thread_name = (char*)apr_pstrdup(pool, threadName);
    
    stat = apr_pool_create(&(*new)->pool, pool);
    if (stat != APR_SUCCESS) {
        return stat;
    }
    
    if (attr && attr->detach) {
        flags |= NX_THR_DETACHED;
    }
    
    (*new)->ctx = NXContextAlloc(
        /* void(*start_routine)(void *arg) */ (void (*)(void *)) dummy_worker,
        /* void *arg */                       (*new),
        /* int priority */                    NX_PRIO_MED,
        /* NXSize_t stackSize */              stack_size,
        /* long flags */                      NX_CTX_NORMAL,
        /* int *error */                      &stat);

    stat = NXContextSetName(
        /* NXContext_t ctx */  (*new)->ctx,
        /* const char *name */ threadName);

    stat = NXThreadCreate(
        /* NXContext_t context */     (*new)->ctx,
        /* long flags */              flags,
        /* NXThreadId_t *thread_id */ &(*new)->td);

    if (stat == 0)
        return APR_SUCCESS;
        
    return(stat); /* if error */    
}

apr_os_thread_t apr_os_thread_current()
{
    return NXThreadGetId();
}

int apr_os_thread_equal(apr_os_thread_t tid1, apr_os_thread_t tid2)
{
    return (tid1 == tid2);
}

void apr_thread_yield()
{
    NXThreadYield();
}

apr_status_t apr_thread_exit(apr_thread_t *thd,
                             apr_status_t retval)
{
    thd->exitval = retval;
    apr_pool_destroy(thd->pool);
    NXThreadExit(NULL);
    return APR_SUCCESS;
}

apr_status_t apr_thread_join(apr_status_t *retval,
                                          apr_thread_t *thd)
{
    apr_status_t  stat;    
    NXThreadId_t dthr;

    if ((stat = NXThreadJoin(thd->td, &dthr, NULL)) == 0) {
        *retval = thd->exitval;
        return APR_SUCCESS;
    }
    else {
        return stat;
    }
}

apr_status_t apr_thread_detach(apr_thread_t *thd)
{
    return APR_SUCCESS;
}

apr_status_t apr_thread_data_get(void **data, const char *key,
                                             apr_thread_t *thread)
{
    if (thread != NULL) {
            return apr_pool_userdata_get(data, key, thread->pool);
    }
    else {
        data = NULL;
        return APR_ENOTHREAD;
    }
}

apr_status_t apr_thread_data_set(void *data, const char *key,
                              apr_status_t (*cleanup) (void *),
                              apr_thread_t *thread)
{
    if (thread != NULL) {
       return apr_pool_userdata_set(data, key, cleanup, thread->pool);
    }
    else {
        data = NULL;
        return APR_ENOTHREAD;
    }
}

APR_DECLARE(apr_status_t) apr_os_thread_get(apr_os_thread_t **thethd,
                                            apr_thread_t *thd)
{
    if (thd == NULL) {
        return APR_ENOTHREAD;
    }
    *thethd = &(thd->td);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_thread_put(apr_thread_t **thd,
                                            apr_os_thread_t *thethd,
                                            apr_pool_t *pool)
{
    if (pool == NULL) {
        return APR_ENOPOOL;
    }
    if ((*thd) == NULL) {
        (*thd) = (apr_thread_t *)apr_palloc(pool, sizeof(apr_thread_t));
        (*thd)->pool = pool;
    }
    (*thd)->td = *thethd;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_once_init(apr_thread_once_t **control,
                                               apr_pool_t *p)
{
    (*control) = apr_pcalloc(p, sizeof(**control));
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_once(apr_thread_once_t *control,
                                          void (*func)(void))
{
    if (!atomic_xchg(&control->value, 1)) {
        func();
    }
    return APR_SUCCESS;
}

APR_POOL_IMPLEMENT_ACCESSOR(thread)


