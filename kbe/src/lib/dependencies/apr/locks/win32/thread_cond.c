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
#include "apr_arch_thread_cond.h"
#include "apr_portable.h"

#include <limits.h>

static apr_status_t thread_cond_cleanup(void *data)
{
    apr_thread_cond_t *cond = data;
    CloseHandle(cond->semaphore);
    DeleteCriticalSection(&cond->csection);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_create(apr_thread_cond_t **cond,
                                                 apr_pool_t *pool)
{
    apr_thread_cond_t *cv;

    cv = apr_pcalloc(pool, sizeof(**cond));
    if (cv == NULL) {
        return APR_ENOMEM;
    }

    cv->semaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    if (cv->semaphore == NULL) {
        return apr_get_os_error();
    }

    *cond = cv;
    cv->pool = pool;
    InitializeCriticalSection(&cv->csection);
    apr_pool_cleanup_register(cv->pool, cv, thread_cond_cleanup,
                              apr_pool_cleanup_null);

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_destroy(apr_thread_cond_t *cond)
{
    return apr_pool_cleanup_run(cond->pool, cond, thread_cond_cleanup);
}

static APR_INLINE apr_status_t _thread_cond_timedwait(apr_thread_cond_t *cond,
                                                      apr_thread_mutex_t *mutex,
                                                      DWORD timeout_ms )
{
    DWORD res;
    apr_status_t rv;
    unsigned int wake = 0;
    unsigned long generation;

    EnterCriticalSection(&cond->csection);
    cond->num_waiting++;
    generation = cond->generation;
    LeaveCriticalSection(&cond->csection);

    apr_thread_mutex_unlock(mutex);

    do {
        res = WaitForSingleObject(cond->semaphore, timeout_ms);

        EnterCriticalSection(&cond->csection);

        if (cond->num_wake) {
            if (cond->generation != generation) {
                cond->num_wake--;
                cond->num_waiting--;
                rv = APR_SUCCESS;
                break;
            } else {
                wake = 1;
            }
        }
        else if (res != WAIT_OBJECT_0) {
            cond->num_waiting--;
            rv = APR_TIMEUP;
            break;
        }

        LeaveCriticalSection(&cond->csection);

        if (wake) {
            wake = 0;
            ReleaseSemaphore(cond->semaphore, 1, NULL);
        }
    } while (1);

    LeaveCriticalSection(&cond->csection);
    apr_thread_mutex_lock(mutex);

    return rv;
}

APR_DECLARE(apr_status_t) apr_thread_cond_wait(apr_thread_cond_t *cond,
                                               apr_thread_mutex_t *mutex)
{
    return _thread_cond_timedwait(cond, mutex, INFINITE);
}

APR_DECLARE(apr_status_t) apr_thread_cond_timedwait(apr_thread_cond_t *cond,
                                                    apr_thread_mutex_t *mutex,
                                                    apr_interval_time_t timeout)
{
    DWORD timeout_ms = (DWORD) apr_time_as_msec(timeout);

    return _thread_cond_timedwait(cond, mutex, timeout_ms);
}

APR_DECLARE(apr_status_t) apr_thread_cond_signal(apr_thread_cond_t *cond)
{
    unsigned int wake = 0;

    EnterCriticalSection(&cond->csection);
    if (cond->num_waiting > cond->num_wake) {
        wake = 1;
        cond->num_wake++;
        cond->generation++;
    }
    LeaveCriticalSection(&cond->csection);

    if (wake) {
        ReleaseSemaphore(cond->semaphore, 1, NULL);
    }

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_broadcast(apr_thread_cond_t *cond)
{
    unsigned long num_wake = 0;

    EnterCriticalSection(&cond->csection);
    if (cond->num_waiting > cond->num_wake) {
        num_wake = cond->num_waiting - cond->num_wake;
        cond->num_wake = cond->num_waiting;
        cond->generation++;
    }
    LeaveCriticalSection(&cond->csection);

    if (num_wake) {
        ReleaseSemaphore(cond->semaphore, num_wake, NULL);
    }

    return APR_SUCCESS;
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_cond)
