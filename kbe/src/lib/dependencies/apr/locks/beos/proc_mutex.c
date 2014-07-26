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

/*Read/Write locking implementation based on the MultiLock code from
 * Stephen Beaulieu <hippo@be.com>
 */
 
#include "apr_arch_proc_mutex.h"
#include "apr_strings.h"
#include "apr_portable.h"

static apr_status_t _proc_mutex_cleanup(void * data)
{
    apr_proc_mutex_t *lock = (apr_proc_mutex_t*)data;
    if (lock->LockCount != 0) {
        /* we're still locked... */
    	while (atomic_add(&lock->LockCount , -1) > 1){
    	    /* OK we had more than one person waiting on the lock so 
    	     * the sem is also locked. Release it until we have no more
    	     * locks left.
    	     */
            release_sem (lock->Lock);
    	}
    }
    delete_sem(lock->Lock);
    return APR_SUCCESS;
}    

APR_DECLARE(apr_status_t) apr_proc_mutex_create(apr_proc_mutex_t **mutex,
                                                const char *fname,
                                                apr_lockmech_e mech,
                                                apr_pool_t *pool)
{
    apr_proc_mutex_t *new;
    apr_status_t stat = APR_SUCCESS;
  
    if (mech != APR_LOCK_DEFAULT) {
        return APR_ENOTIMPL;
    }

    new = (apr_proc_mutex_t *)apr_pcalloc(pool, sizeof(apr_proc_mutex_t));
    if (new == NULL){
        return APR_ENOMEM;
    }
    
    if ((stat = create_sem(0, "APR_Lock")) < B_NO_ERROR) {
        _proc_mutex_cleanup(new);
        return stat;
    }
    new->LockCount = 0;
    new->Lock = stat;  
    new->pool  = pool;

    apr_pool_cleanup_register(new->pool, (void *)new, _proc_mutex_cleanup,
                              apr_pool_cleanup_null);

    (*mutex) = new;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_child_init(apr_proc_mutex_t **mutex,
                                                    const char *fname,
                                                    apr_pool_t *pool)
{
    return APR_SUCCESS;
}
    
APR_DECLARE(apr_status_t) apr_proc_mutex_lock(apr_proc_mutex_t *mutex)
{
    int32 stat;
    
	if (atomic_add(&mutex->LockCount, 1) > 0) {
		if ((stat = acquire_sem(mutex->Lock)) < B_NO_ERROR) {
		    atomic_add(&mutex->LockCount, -1);
		    return stat;
		}
	}
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_trylock(apr_proc_mutex_t *mutex)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_unlock(apr_proc_mutex_t *mutex)
{
    int32 stat;
    
	if (atomic_add(&mutex->LockCount, -1) > 1) {
        if ((stat = release_sem(mutex->Lock)) < B_NO_ERROR) {
            atomic_add(&mutex->LockCount, 1);
            return stat;
        }
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_destroy(apr_proc_mutex_t *mutex)
{
    apr_status_t stat;
    if ((stat = _proc_mutex_cleanup(mutex)) == APR_SUCCESS) {
        apr_pool_cleanup_kill(mutex->pool, mutex, _proc_mutex_cleanup);
        return APR_SUCCESS;
    }
    return stat;
}

APR_DECLARE(apr_status_t) apr_proc_mutex_cleanup(void *mutex)
{
    return _proc_mutex_cleanup(mutex);
}


APR_DECLARE(const char *) apr_proc_mutex_lockfile(apr_proc_mutex_t *mutex)
{
    return NULL;
}

APR_DECLARE(const char *) apr_proc_mutex_name(apr_proc_mutex_t *mutex)
{
    return "beossem";
}

APR_DECLARE(const char *) apr_proc_mutex_defname(void)
{
    return "beossem";
}

APR_POOL_IMPLEMENT_ACCESSOR(proc_mutex)

/* Implement OS-specific accessors defined in apr_portable.h */

APR_DECLARE(apr_status_t) apr_os_proc_mutex_get(apr_os_proc_mutex_t *ospmutex,
                                                apr_proc_mutex_t *pmutex)
{
    ospmutex->sem = pmutex->Lock;
    ospmutex->ben = pmutex->LockCount;
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
        (*pmutex) = (apr_proc_mutex_t *)apr_pcalloc(pool, sizeof(apr_proc_mutex_t));
        (*pmutex)->pool = pool;
    }
    (*pmutex)->Lock = ospmutex->sem;
    (*pmutex)->LockCount = ospmutex->ben;
    return APR_SUCCESS;
}

