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
#include "apr_arch_proc_mutex.h"
#include "apr_arch_file_io.h"
#include <string.h>
#include <stddef.h>

#define CurrentTid (*_threadid)

static char *fixed_name(const char *fname, apr_pool_t *pool)
{
    char *semname;

    if (fname == NULL)
        semname = NULL;
    else {
        /* Semaphores don't live in the file system, fix up the name */
        while (*fname == '/' || *fname == '\\') {
            fname++;
        }

        semname = apr_pstrcat(pool, "/SEM32/", fname, NULL);

        if (semname[8] == ':') {
            semname[8] = '$';
        }
    }

    return semname;
}



APR_DECLARE(apr_status_t) apr_proc_mutex_cleanup(void *vmutex)
{
    apr_proc_mutex_t *mutex = vmutex;
    return apr_proc_mutex_destroy(mutex);
}

APR_DECLARE(const char *) apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex)
{
    return NULL;
}

APR_DECLARE(const char *) apr_proc_mutex_name(apr_proc_mutex_t *mutex)
{
    return "os2sem";
}

APR_DECLARE(const char *) apr_proc_mutex_defname(void)
{
    return "os2sem";
}


APR_DECLARE(apr_status_t) apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool)
{
    apr_proc_mutex_t *new;
    ULONG rc;
    char *semname;

    if (mech != APR_LOCK_DEFAULT) {
        return APR_ENOTIMPL;
    }

    new = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));
    new->pool       = pool;
    new->owner      = 0;
    new->lock_count = 0;
    *mutex = new;

    semname = fixed_name(fname, pool);
    rc = DosCreateMutexSem(semname, &(new->hMutex), DC_SEM_SHARED, FALSE);

    if (!rc) {
        apr_pool_cleanup_register(pool, new, apr_proc_mutex_cleanup, apr_pool_cleanup_null);
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool)
{
    apr_proc_mutex_t *new;
    ULONG rc;
    char *semname;

    new = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));
    new->pool       = pool;
    new->owner      = 0;
    new->lock_count = 0;

    semname = fixed_name(fname, pool);
    rc = DosOpenMutexSem(semname, &(new->hMutex));
    *mutex = new;

    if (!rc) {
        apr_pool_cleanup_register(pool, new, apr_proc_mutex_cleanup, apr_pool_cleanup_null);
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_proc_mutex_lock(apr_proc_mutex_t *mutex)
{
    ULONG rc = DosRequestMutexSem(mutex->hMutex, SEM_INDEFINITE_WAIT);

    if (rc == 0) {
        mutex->owner = CurrentTid;
        mutex->lock_count++;
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_proc_mutex_trylock(apr_proc_mutex_t *mutex)
{
    ULONG rc = DosRequestMutexSem(mutex->hMutex, SEM_IMMEDIATE_RETURN);

    if (rc == 0) {
        mutex->owner = CurrentTid;
        mutex->lock_count++;
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_DECLARE(apr_status_t) apr_proc_mutex_unlock(apr_proc_mutex_t *mutex)
{
    ULONG rc;

    if (mutex->owner == CurrentTid && mutex->lock_count > 0) {
        mutex->lock_count--;
        rc = DosReleaseMutexSem(mutex->hMutex);
        return APR_FROM_OS_ERROR(rc);
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_proc_mutex_destroy(apr_proc_mutex_t *mutex)
{
    ULONG rc;
    apr_status_t status = APR_SUCCESS;

    if (mutex->owner == CurrentTid) {
        while (mutex->lock_count > 0 && status == APR_SUCCESS) {
            status = apr_proc_mutex_unlock(mutex);
        }
    }

    if (status != APR_SUCCESS) {
        return status;
    }

    if (mutex->hMutex == 0) {
        return APR_SUCCESS;
    }

    rc = DosCloseMutexSem(mutex->hMutex);

    if (!rc) {
        mutex->hMutex = 0;
    }

    return APR_FROM_OS_ERROR(rc);
}



APR_POOL_IMPLEMENT_ACCESSOR(proc_mutex)



/* Implement OS-specific accessors defined in apr_portable.h */

APR_DECLARE(apr_status_t) apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                                apr_proc_mutex_t *pmutex)
{
    *ospmutex = pmutex->hMutex;
    return APR_ENOTIMPL;
}



APR_DECLARE(apr_status_t) apr_os_proc_mutex_put(apr_proc_mutex_t **pmutex,
                                                apr_os_proc_mutex_t *ospmutex,
                                                apr_pool_t *pool)
{
    apr_proc_mutex_t *new;

    new = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));
    new->pool       = pool;
    new->owner      = 0;
    new->lock_count = 0;
    new->hMutex     = *ospmutex;
    *pmutex = new;

    return APR_SUCCESS;
}

