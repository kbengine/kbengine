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
#include "apr_portable.h"
#include "apr_arch_file_io.h"
#include "apr_arch_proc_mutex.h"
#include "apr_arch_misc.h"

static apr_status_t proc_mutex_cleanup(void *mutex_)
{
    apr_proc_mutex_t *mutex = mutex_;

    if (mutex->handle) {
        if (CloseHandle(mutex->handle) == 0) {
            return apr_get_os_error();
        }
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool)
{
    HANDLE hMutex;
    void *mutexkey;

    /* res_name_from_filename turns fname into a pseduo-name
     * without slashes or backslashes, and prepends the \global
     * prefix on Win2K and later
     */
    if (fname) {
        mutexkey = res_name_from_filename(fname, 1, pool);
    }
    else {
        mutexkey = NULL;
    }

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        hMutex = CreateMutexW(NULL, FALSE, mutexkey);
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        hMutex = CreateMutexA(NULL, FALSE, mutexkey);
    }
#endif

    if (!hMutex) {
        return apr_get_os_error();
    }

    *mutex = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));
    (*mutex)->pool = pool;
    (*mutex)->handle = hMutex;
    (*mutex)->fname = fname;
    apr_pool_cleanup_register((*mutex)->pool, *mutex, 
                              proc_mutex_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool)
{
    HANDLE hMutex;
    void *mutexkey;

    if (!fname) {
        /* Reinitializing unnamed mutexes is a noop in the Unix code. */
        return APR_SUCCESS;
    }

    /* res_name_from_filename turns file into a pseudo-name
     * without slashes or backslashes, and prepends the \global
     * prefix on Win2K and later
     */
    mutexkey = res_name_from_filename(fname, 1, pool);

#if defined(_WIN32_WCE)
    hMutex = CreateMutex(NULL, FALSE, mutexkey);
    if (hMutex && ERROR_ALREADY_EXISTS != GetLastError()) {
        CloseHandle(hMutex);
        hMutex = NULL;
        SetLastError(ERROR_FILE_NOT_FOUND);
    }
#else
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        hMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, mutexkey);
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexkey);
    }
#endif
#endif

    if (!hMutex) {
        return apr_get_os_error();
    }

    *mutex = (apr_proc_mutex_t *)apr_palloc(pool, sizeof(apr_proc_mutex_t));
    (*mutex)->pool = pool;
    (*mutex)->handle = hMutex;
    (*mutex)->fname = fname;
    apr_pool_cleanup_register((*mutex)->pool, *mutex, 
                              proc_mutex_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}
    
APR_DECLARE(apr_status_t) apr_proc_mutex_lock(apr_proc_mutex_t *mutex)
{
    DWORD rv;

    rv = WaitForSingleObject(mutex->handle, INFINITE);

    if (rv == WAIT_OBJECT_0 || rv == WAIT_ABANDONED) {
        return APR_SUCCESS;
    }
    return apr_get_os_error();
}

APR_DECLARE(apr_status_t) apr_proc_mutex_trylock(apr_proc_mutex_t *mutex)
{
    DWORD rv;

    rv = WaitForSingleObject(mutex->handle, 0);

    if (rv == WAIT_OBJECT_0 || rv == WAIT_ABANDONED) {
        return APR_SUCCESS;
    } 
    else if (rv == WAIT_TIMEOUT) {
        return APR_EBUSY;
    }
    return apr_get_os_error();
}

APR_DECLARE(apr_status_t) apr_proc_mutex_unlock(apr_proc_mutex_t *mutex)
{
    if (ReleaseMutex(mutex->handle) == 0) {
        return apr_get_os_error();
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_destroy(apr_proc_mutex_t *mutex)
{
    apr_status_t stat;

    stat = proc_mutex_cleanup(mutex);
    if (stat == APR_SUCCESS) {
        apr_pool_cleanup_kill(mutex->pool, mutex, proc_mutex_cleanup);
    }
    return stat;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_cleanup(void *mutex)
{
    return apr_proc_mutex_destroy((apr_proc_mutex_t *)mutex);
}

APR_DECLARE(const char *) apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex)
{
    return mutex->fname;
}

APR_DECLARE(const char *) apr_proc_mutex_name(apr_proc_mutex_t *mutex)
{
    return apr_proc_mutex_defname();
}

APR_DECLARE(const char *) apr_proc_mutex_defname(void)
{
    return "win32mutex";
}

APR_POOL_IMPLEMENT_ACCESSOR(proc_mutex)

/* Implement OS-specific accessors defined in apr_portable.h */

APR_DECLARE(apr_status_t) apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                                apr_proc_mutex_t *mutex)
{
    *ospmutex = mutex->handle;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_proc_mutex_put(apr_proc_mutex_t **pmutex,
                                                apr_os_proc_mutex_t *ospmutex,
                                                apr_pool_t *pool)
{
    if (pool == NULL) {
        return APR_ENOPOOL;
    }
    if ((*pmutex) == NULL) {
        (*pmutex) = (apr_proc_mutex_t *)apr_palloc(pool,
                                                   sizeof(apr_proc_mutex_t));
        (*pmutex)->pool = pool;
    }
    (*pmutex)->handle = *ospmutex;
    return APR_SUCCESS;
}

