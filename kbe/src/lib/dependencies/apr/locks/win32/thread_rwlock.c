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
#include "apr_arch_thread_rwlock.h"
#include "apr_portable.h"

static apr_status_t thread_rwlock_cleanup(void *data)
{
    apr_thread_rwlock_t *rwlock = data;
    
    if (! CloseHandle(rwlock->read_event))
        return apr_get_os_error();

    if (! CloseHandle(rwlock->write_mutex))
        return apr_get_os_error();
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t)apr_thread_rwlock_create(apr_thread_rwlock_t **rwlock,
                                                  apr_pool_t *pool)
{
    *rwlock = apr_palloc(pool, sizeof(**rwlock));

    (*rwlock)->pool        = pool;
    (*rwlock)->readers     = 0;

    if (! ((*rwlock)->read_event = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        *rwlock = NULL;
        return apr_get_os_error();
    }

    if (! ((*rwlock)->write_mutex = CreateMutex(NULL, FALSE, NULL))) {
        CloseHandle((*rwlock)->read_event);
        *rwlock = NULL;
        return apr_get_os_error();
    }

    apr_pool_cleanup_register(pool, *rwlock, thread_rwlock_cleanup,
                              apr_pool_cleanup_null);

    return APR_SUCCESS;
}

static apr_status_t apr_thread_rwlock_rdlock_core(apr_thread_rwlock_t *rwlock,
                                                  DWORD  milliseconds)
{
    DWORD   code = WaitForSingleObject(rwlock->write_mutex, milliseconds);

    if (code == WAIT_FAILED || code == WAIT_TIMEOUT)
        return APR_FROM_OS_ERROR(code);

    /* We've successfully acquired the writer mutex, we can't be locked
     * for write, so it's OK to add the reader lock.  The writer mutex
     * doubles as race condition protection for the readers counter.   
     */
    InterlockedIncrement(&rwlock->readers);
    
    if (! ResetEvent(rwlock->read_event))
        return apr_get_os_error();
    
    if (! ReleaseMutex(rwlock->write_mutex))
        return apr_get_os_error();
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_rwlock_rdlock(apr_thread_rwlock_t *rwlock)
{
    return apr_thread_rwlock_rdlock_core(rwlock, INFINITE);
}

APR_DECLARE(apr_status_t) 
apr_thread_rwlock_tryrdlock(apr_thread_rwlock_t *rwlock)
{
    return apr_thread_rwlock_rdlock_core(rwlock, 0);
}

static apr_status_t 
apr_thread_rwlock_wrlock_core(apr_thread_rwlock_t *rwlock, DWORD milliseconds)
{
    DWORD   code = WaitForSingleObject(rwlock->write_mutex, milliseconds);

    if (code == WAIT_FAILED || code == WAIT_TIMEOUT)
        return APR_FROM_OS_ERROR(code);

    /* We've got the writer lock but we have to wait for all readers to
     * unlock before it's ok to use it.
     */
    if (rwlock->readers) {
        /* Must wait for readers to finish before returning, unless this
         * is an trywrlock (milliseconds == 0):
         */
        code = milliseconds
          ? WaitForSingleObject(rwlock->read_event, milliseconds)
          : WAIT_TIMEOUT;
        
        if (code == WAIT_FAILED || code == WAIT_TIMEOUT) {
            /* Unable to wait for readers to finish, release write lock: */
            if (! ReleaseMutex(rwlock->write_mutex))
                return apr_get_os_error();
            
            return APR_FROM_OS_ERROR(code);
        }
    }

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_rwlock_wrlock(apr_thread_rwlock_t *rwlock)
{
    return apr_thread_rwlock_wrlock_core(rwlock, INFINITE);
}

APR_DECLARE(apr_status_t)apr_thread_rwlock_trywrlock(apr_thread_rwlock_t *rwlock)
{
    return apr_thread_rwlock_wrlock_core(rwlock, 0);
}

APR_DECLARE(apr_status_t) apr_thread_rwlock_unlock(apr_thread_rwlock_t *rwlock)
{
    apr_status_t rv = 0;

    /* First, guess that we're unlocking a writer */
    if (! ReleaseMutex(rwlock->write_mutex))
        rv = apr_get_os_error();
    
    if (rv == APR_FROM_OS_ERROR(ERROR_NOT_OWNER)) {
        /* Nope, we must have a read lock */
        if (rwlock->readers &&
            ! InterlockedDecrement(&rwlock->readers) &&
            ! SetEvent(rwlock->read_event)) {
            rv = apr_get_os_error();
        }
        else {
            rv = 0;
        }
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_thread_rwlock_destroy(apr_thread_rwlock_t *rwlock)
{
    return apr_pool_cleanup_run(rwlock->pool, rwlock, thread_rwlock_cleanup);
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_rwlock)
