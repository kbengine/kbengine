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
#include "apr_private.h"
#include "apr_general.h"
#include "apr_strings.h"
#include "apr_arch_thread_mutex.h"
#include "apr_thread_mutex.h"
#include "apr_portable.h"
#include "apr_arch_misc.h"

static apr_status_t thread_mutex_cleanup(void *data)
{
    apr_thread_mutex_t *lock = data;

    if (lock->type == thread_mutex_critical_section) {
        lock->type = -1;
        DeleteCriticalSection(&lock->section);
    }
    else {
        if (!CloseHandle(lock->handle)) {
            return apr_get_os_error();
        }
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool)
{
    (*mutex) = (apr_thread_mutex_t *)apr_palloc(pool, sizeof(**mutex));

    (*mutex)->pool = pool;

    if (flags & APR_THREAD_MUTEX_UNNESTED) {
        /* Use an auto-reset signaled event, ready to accept one
         * waiting thread.
         */
        (*mutex)->type = thread_mutex_unnested_event;
        (*mutex)->handle = CreateEvent(NULL, FALSE, TRUE, NULL);
    }
    else {
#if APR_HAS_UNICODE_FS
        /* Critical Sections are terrific, performance-wise, on NT.
         * On Win9x, we cannot 'try' on a critical section, so we 
         * use a [slower] mutex object, instead.
         */
        IF_WIN_OS_IS_UNICODE {
            InitializeCriticalSection(&(*mutex)->section);
            (*mutex)->type = thread_mutex_critical_section;
        }
#endif
#if APR_HAS_ANSI_FS
        ELSE_WIN_OS_IS_ANSI {
            (*mutex)->type = thread_mutex_nested_mutex;
            (*mutex)->handle = CreateMutex(NULL, FALSE, NULL);

        }
#endif
    }

    apr_pool_cleanup_register((*mutex)->pool, (*mutex), thread_mutex_cleanup,
                              apr_pool_cleanup_null);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_lock(apr_thread_mutex_t *mutex)
{
    if (mutex->type == thread_mutex_critical_section) {
        EnterCriticalSection(&mutex->section);
    }
    else {
        DWORD rv = WaitForSingleObject(mutex->handle, INFINITE);
	if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
            return (rv == WAIT_TIMEOUT) ? APR_EBUSY : apr_get_os_error();
	}
    }        
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_trylock(apr_thread_mutex_t *mutex)
{
    if (mutex->type == thread_mutex_critical_section) {
        if (!TryEnterCriticalSection(&mutex->section)) {
            return APR_EBUSY;
        }
    }
    else {
        DWORD rv = WaitForSingleObject(mutex->handle, 0);
	if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
            return (rv == WAIT_TIMEOUT) ? APR_EBUSY : apr_get_os_error();
	}
    }        
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_unlock(apr_thread_mutex_t *mutex)
{
    if (mutex->type == thread_mutex_critical_section) {
        LeaveCriticalSection(&mutex->section);
    }
    else if (mutex->type == thread_mutex_unnested_event) {
        if (!SetEvent(mutex->handle)) {
            return apr_get_os_error();
        }
    }
    else if (mutex->type == thread_mutex_nested_mutex) {
        if (!ReleaseMutex(mutex->handle)) {
            return apr_get_os_error();
        }
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_destroy(apr_thread_mutex_t *mutex)
{
    return apr_pool_cleanup_run(mutex->pool, mutex, thread_mutex_cleanup);
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_mutex)

