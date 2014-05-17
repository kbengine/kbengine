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
#include "apr_portable.h"
#include "apr_arch_proc_mutex.h"
#include "apr_arch_thread_mutex.h"

APR_DECLARE(apr_status_t) apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool)
{
    apr_status_t ret;
    apr_proc_mutex_t *new_mutex = NULL;
    new_mutex = (apr_proc_mutex_t *)apr_pcalloc(pool, sizeof(apr_proc_mutex_t));
	
	if(new_mutex ==NULL) {
        return APR_ENOMEM;
    }     
    
    new_mutex->pool = pool;
    ret = apr_thread_mutex_create(&(new_mutex->mutex), APR_THREAD_MUTEX_DEFAULT, pool);

    if (ret == APR_SUCCESS)
        *mutex = new_mutex;

    return ret;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool)
{
    return APR_SUCCESS;
}
    
APR_DECLARE(apr_status_t) apr_proc_mutex_lock(apr_proc_mutex_t *mutex)
{
    if (mutex)
        return apr_thread_mutex_lock(mutex->mutex);
    return APR_ENOLOCK;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_trylock(apr_proc_mutex_t *mutex)
{
    if (mutex)
        return apr_thread_mutex_trylock(mutex->mutex);
    return APR_ENOLOCK;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_unlock(apr_proc_mutex_t *mutex)
{
    if (mutex)
        return apr_thread_mutex_unlock(mutex->mutex);
    return APR_ENOLOCK;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_cleanup(void *mutex)
{
    return apr_proc_mutex_destroy(mutex);
}

APR_DECLARE(apr_status_t) apr_proc_mutex_destroy(apr_proc_mutex_t *mutex)
{
    if (mutex)
        return apr_thread_mutex_destroy(mutex->mutex);
    return APR_ENOLOCK;
}

APR_DECLARE(const char *) apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex)
{
    return NULL;
}

APR_DECLARE(const char *) apr_proc_mutex_name(apr_proc_mutex_t *mutex)
{
    return "netwarethread";
}

APR_DECLARE(const char *) apr_proc_mutex_defname(void)
{
    return "netwarethread";
}

APR_POOL_IMPLEMENT_ACCESSOR(proc_mutex)

/* Implement OS-specific accessors defined in apr_portable.h */

apr_status_t apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                   apr_proc_mutex_t *pmutex)
{
    if (pmutex)
        ospmutex = pmutex->mutex->mutex;
    return APR_ENOLOCK;
}

apr_status_t apr_os_proc_mutex_put(apr_proc_mutex_t **pmutex,
                                   apr_os_proc_mutex_t *ospmutex,
                                   apr_pool_t *pool)
{
    return APR_ENOTIMPL;
}

