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

#include "apr_private.h"
#include "apr_arch_threadproc.h"
#include "apr_thread_proc.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_portable.h"
#if APR_HAVE_PROCESS_H
#include <process.h>
#endif
#include "apr_arch_misc.h"   

/* Chosen for us by apr_initialize */
DWORD tls_apr_thread = 0;

APR_DECLARE(apr_status_t) apr_threadattr_create(apr_threadattr_t **new,
                                                apr_pool_t *pool)
{
    (*new) = (apr_threadattr_t *)apr_palloc(pool, 
              sizeof(apr_threadattr_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }

    (*new)->pool = pool;
    (*new)->detach = 0;
    (*new)->stacksize = 0;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_threadattr_detach_set(apr_threadattr_t *attr,
                                                   apr_int32_t on)
{
    attr->detach = on;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_threadattr_detach_get(apr_threadattr_t *attr)
{
    if (attr->detach == 1)
        return APR_DETACH;
    return APR_NOTDETACH;
}

APR_DECLARE(apr_status_t) apr_threadattr_stacksize_set(apr_threadattr_t *attr,
                                                       apr_size_t stacksize)
{
    attr->stacksize = stacksize;
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
    TlsSetValue(tls_apr_thread, thd->td);
    return thd->func(thd, thd->data);
}

APR_DECLARE(apr_status_t) apr_thread_create(apr_thread_t **new,
                                            apr_threadattr_t *attr,
                                            apr_thread_start_t func,
                                            void *data, apr_pool_t *pool)
{
    apr_status_t stat;
	unsigned temp;
    HANDLE handle;

    (*new) = (apr_thread_t *)apr_palloc(pool, sizeof(apr_thread_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }

    (*new)->data = data;
    (*new)->func = func;
    (*new)->td   = NULL;
    stat = apr_pool_create(&(*new)->pool, pool);
    if (stat != APR_SUCCESS) {
        return stat;
    }

    /* Use 0 for default Thread Stack Size, because that will
     * default the stack to the same size as the calling thread.
     */
#ifndef _WIN32_WCE
    if ((handle = (HANDLE)_beginthreadex(NULL,
                        (DWORD) (attr ? attr->stacksize : 0),
                        (unsigned int (APR_THREAD_FUNC *)(void *))dummy_worker,
                        (*new), 0, &temp)) == 0) {
        return APR_FROM_OS_ERROR(_doserrno);
    }
#else
   if ((handle = CreateThread(NULL,
                        attr && attr->stacksize > 0 ? attr->stacksize : 0,
                        (unsigned int (APR_THREAD_FUNC *)(void *))dummy_worker,
                        (*new), 0, &temp)) == 0) {
        return apr_get_os_error();
    }
#endif
    if (attr && attr->detach) {
        CloseHandle(handle);
    }
    else
        (*new)->td = handle;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_exit(apr_thread_t *thd,
                                          apr_status_t retval)
{
    thd->exitval = retval;
    apr_pool_destroy(thd->pool);
    thd->pool = NULL;
#ifndef _WIN32_WCE
    _endthreadex(0);
#else
    ExitThread(0);
#endif
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_join(apr_status_t *retval,
                                          apr_thread_t *thd)
{
    apr_status_t rv = APR_SUCCESS;
    
    if (!thd->td) {
        /* Can not join on detached threads */
        return APR_DETACH;
    }
    rv = WaitForSingleObject(thd->td, INFINITE);
    if ( rv == WAIT_OBJECT_0 || rv == WAIT_ABANDONED) {
        /* If the thread_exit has been called */
        if (!thd->pool)
            *retval = thd->exitval;
        else
            rv = APR_INCOMPLETE;
    }
    else
        rv = apr_get_os_error();
    CloseHandle(thd->td);
    thd->td = NULL;

    return rv;
}

APR_DECLARE(apr_status_t) apr_thread_detach(apr_thread_t *thd)
{
    if (thd->td && CloseHandle(thd->td)) {
        thd->td = NULL;
        return APR_SUCCESS;
    }
    else {
        return apr_get_os_error();
    }
}

APR_DECLARE(void) apr_thread_yield()
{
    /* SwitchToThread is not supported on Win9x, but since it's
     * primarily a noop (entering time consuming code, therefore
     * providing more critical threads a bit larger timeslice)
     * we won't worry too much if it's not available.
     */
#ifndef _WIN32_WCE
    if (apr_os_level >= APR_WIN_NT) {
        SwitchToThread();
    }
#endif
}

APR_DECLARE(apr_status_t) apr_thread_data_get(void **data, const char *key,
                                             apr_thread_t *thread)
{
    return apr_pool_userdata_get(data, key, thread->pool);
}

APR_DECLARE(apr_status_t) apr_thread_data_set(void *data, const char *key,
                                             apr_status_t (*cleanup) (void *),
                                             apr_thread_t *thread)
{
    return apr_pool_userdata_set(data, key, cleanup, thread->pool);
}


APR_DECLARE(apr_os_thread_t) apr_os_thread_current(void)
{
    HANDLE hthread = (HANDLE)TlsGetValue(tls_apr_thread);
    HANDLE hproc;

    if (hthread) {
        return hthread;
    }
    
    hproc = GetCurrentProcess();
    hthread = GetCurrentThread();
    if (!DuplicateHandle(hproc, hthread, 
                         hproc, &hthread, 0, FALSE, 
                         DUPLICATE_SAME_ACCESS)) {
        return NULL;
    }
    TlsSetValue(tls_apr_thread, hthread);
    return hthread;
}

APR_DECLARE(apr_status_t) apr_os_thread_get(apr_os_thread_t **thethd,
                                            apr_thread_t *thd)
{
    if (thd == NULL) {
        return APR_ENOTHREAD;
    }
    *thethd = thd->td;
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
    (*thd)->td = thethd;
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
    if (!InterlockedExchange(&control->value, 1)) {
        func();
    }
    return APR_SUCCESS;
}

APR_DECLARE(int) apr_os_thread_equal(apr_os_thread_t tid1,
                                     apr_os_thread_t tid2)
{
    /* Since the only tid's we support our are own, and
     * apr_os_thread_current returns the identical handle
     * to the one we created initially, the test is simple.
     */
    return (tid1 == tid2);
}

APR_POOL_IMPLEMENT_ACCESSOR(thread)
