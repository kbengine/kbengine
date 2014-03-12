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
#include "apr_arch_thread_rwlock.h"
#include "apr_arch_file_io.h"
#include <string.h>

static apr_status_t thread_rwlock_cleanup(void *therwlock)
{
    apr_thread_rwlock_t *rwlock = therwlock;
    return apr_thread_rwlock_destroy(rwlock);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_create(apr_thread_rwlock_t **rwlock,
                                                   apr_pool_t *pool)
{
    apr_thread_rwlock_t *new_rwlock;
    ULONG rc;

    new_rwlock = (apr_thread_rwlock_t *)apr_palloc(pool, sizeof(apr_thread_rwlock_t));
    new_rwlock->pool = pool;
    new_rwlock->readers = 0;

    rc = DosCreateMutexSem(NULL, &(new_rwlock->write_lock), 0, FALSE);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    rc = DosCreateEventSem(NULL, &(new_rwlock->read_done), 0, FALSE);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    *rwlock = new_rwlock;

    if (!rc)
        apr_pool_cleanup_register(pool, new_rwlock, thread_rwlock_cleanup,
                                  apr_pool_cleanup_null);

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_rdlock(apr_thread_rwlock_t *rwlock)
{
    ULONG rc, posts;

    rc = DosRequestMutexSem(rwlock->write_lock, SEM_INDEFINITE_WAIT);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    /* We've successfully acquired the writer mutex so we can't be locked
     * for write which means it's ok to add a reader lock. The writer mutex
     * doubles as race condition protection for the readers counter.
     */
    rwlock->readers++;
    DosResetEventSem(rwlock->read_done, &posts);
    rc = DosReleaseMutexSem(rwlock->write_lock);
    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_tryrdlock(apr_thread_rwlock_t *rwlock)
{
    /* As above but with different wait time */
    ULONG rc, posts;

    rc = DosRequestMutexSem(rwlock->write_lock, SEM_IMMEDIATE_RETURN);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    rwlock->readers++;
    DosResetEventSem(rwlock->read_done, &posts);
    rc = DosReleaseMutexSem(rwlock->write_lock);
    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_wrlock(apr_thread_rwlock_t *rwlock)
{
    ULONG rc;

    rc = DosRequestMutexSem(rwlock->write_lock, SEM_INDEFINITE_WAIT);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    /* We've got the writer lock but we have to wait for all readers to
     * unlock before it's ok to use it
     */

    if (rwlock->readers) {
        rc = DosWaitEventSem(rwlock->read_done, SEM_INDEFINITE_WAIT);

        if (rc)
            DosReleaseMutexSem(rwlock->write_lock);
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_trywrlock(apr_thread_rwlock_t *rwlock)
{
    ULONG rc;

    rc = DosRequestMutexSem(rwlock->write_lock, SEM_IMMEDIATE_RETURN);

    if (rc)
        return APR_FROM_OS_ERROR(rc);

    /* We've got the writer lock but we have to wait for all readers to
     * unlock before it's ok to use it
     */

    if (rwlock->readers) {
        /* There are readers active, give up */
        DosReleaseMutexSem(rwlock->write_lock);
        rc = ERROR_TIMEOUT;
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_unlock(apr_thread_rwlock_t *rwlock)
{
    ULONG rc;

    /* First, guess that we're unlocking a writer */
    rc = DosReleaseMutexSem(rwlock->write_lock);

    if (rc == ERROR_NOT_OWNER) {
        /* Nope, we must have a read lock */
        if (rwlock->readers) {
            DosEnterCritSec();
            rwlock->readers--;

            if (rwlock->readers == 0) {
                DosPostEventSem(rwlock->read_done);
            }

            DosExitCritSec();
            rc = 0;
        }
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_thread_rwlock_destroy(apr_thread_rwlock_t *rwlock)
{
    ULONG rc;

    if (rwlock->write_lock == 0)
        return APR_SUCCESS;

    while (DosReleaseMutexSem(rwlock->write_lock) == 0);

    rc = DosCloseMutexSem(rwlock->write_lock);

    if (!rc) {
        rwlock->write_lock = 0;
        DosCloseEventSem(rwlock->read_done);
        return APR_SUCCESS;
    }

    return APR_FROM_OS_ERROR(rc);
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_rwlock)

