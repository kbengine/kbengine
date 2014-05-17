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

#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_portable.h"
#include "apr_arch_thread_mutex.h"
#include "apr_arch_file_io.h"
#include <string.h>
#include <stddef.h>

static apr_status_t thread_mutex_cleanup(void *themutex)
{
    apr_thread_mutex_t *mutex = themutex;
    return apr_thread_mutex_destroy(mutex);
}



/* XXX: Need to respect APR_THREAD_MUTEX_[UN]NESTED flags argument
 *      or return APR_ENOTIMPL!!!
 */
APR_DECLARE(apr_status_t) apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool)
{
    apr_thread_mutex_t *new_mutex;
    ULONG rc;

    new_mutex = (apr_thread_mutex_t *)apr_palloc(pool, sizeof(apr_thread_mutex_t));
    new_mutex->pool = pool;

    rc = DosCreateMutexSem(NULL, &(new_mutex->hMutex), 0, FALSE);
    *mutex = new_mutex;

    if (!rc)
        apr_pool_cleanup_register(pool, new_mutex, thread_mutex_cleanup, apr_pool_cleanup_null);

    return APR_OS2_STATUS(rc);
}



APR_DECLARE(apr_status_t) apr_thread_mutex_lock(apr_thread_mutex_t *mutex)
{
    ULONG rc = DosRequestMutexSem(mutex->hMutex, SEM_INDEFINITE_WAIT);
    return APR_OS2_STATUS(rc);
}



APR_DECLARE(apr_status_t) apr_thread_mutex_trylock(apr_thread_mutex_t *mutex)
{
    ULONG rc = DosRequestMutexSem(mutex->hMutex, SEM_IMMEDIATE_RETURN);
    return APR_OS2_STATUS(rc);
}



APR_DECLARE(apr_status_t) apr_thread_mutex_unlock(apr_thread_mutex_t *mutex)
{
    ULONG rc = DosReleaseMutexSem(mutex->hMutex);
    return APR_OS2_STATUS(rc);
}



APR_DECLARE(apr_status_t) apr_thread_mutex_destroy(apr_thread_mutex_t *mutex)
{
    ULONG rc;

    if (mutex->hMutex == 0)
        return APR_SUCCESS;

    while (DosReleaseMutexSem(mutex->hMutex) == 0);

    rc = DosCloseMutexSem(mutex->hMutex);

    if (!rc) {
        mutex->hMutex = 0;
        return APR_SUCCESS;
    }

    return APR_FROM_OS_ERROR(rc);
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_mutex)

