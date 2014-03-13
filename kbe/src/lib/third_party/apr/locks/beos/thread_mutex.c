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
 
#include "apr_arch_thread_mutex.h"
#include "apr_strings.h"
#include "apr_portable.h"

static apr_status_t _thread_mutex_cleanup(void * data)
{
    apr_thread_mutex_t *lock = (apr_thread_mutex_t*)data;
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

APR_DECLARE(apr_status_t) apr_thread_mutex_create(apr_thread_mutex_t **mutex,
                                                  unsigned int flags,
                                                  apr_pool_t *pool)
{
    apr_thread_mutex_t *new_m;
    apr_status_t stat = APR_SUCCESS;
  
    new_m = (apr_thread_mutex_t *)apr_pcalloc(pool, sizeof(apr_thread_mutex_t));
    if (new_m == NULL){
        return APR_ENOMEM;
    }
    
    if ((stat = create_sem(0, "APR_Lock")) < B_NO_ERROR) {
        _thread_mutex_cleanup(new_m);
        return stat;
    }
    new_m->LockCount = 0;
    new_m->Lock = stat;  
    new_m->pool  = pool;

    /* Optimal default is APR_THREAD_MUTEX_UNNESTED, 
     * no additional checks required for either flag.
     */
    new_m->nested = flags & APR_THREAD_MUTEX_NESTED;

    apr_pool_cleanup_register(new_m->pool, (void *)new_m, _thread_mutex_cleanup,
                              apr_pool_cleanup_null);

    (*mutex) = new_m;
    return APR_SUCCESS;
}

#if APR_HAS_CREATE_LOCKS_NP
APR_DECLARE(apr_status_t) apr_thread_mutex_create_np(apr_thread_mutex_t **mutex,
                                                   const char *fname,
                                                   apr_lockmech_e_np mech,
                                                   apr_pool_t *pool)
{
    return APR_ENOTIMPL;
}       
#endif
  
APR_DECLARE(apr_status_t) apr_thread_mutex_lock(apr_thread_mutex_t *mutex)
{
    int32 stat;
    thread_id me = find_thread(NULL);
    
    if (mutex->nested && mutex->owner == me) {
        mutex->owner_ref++;
        return APR_SUCCESS;
    }
    
	if (atomic_add(&mutex->LockCount, 1) > 0) {
		if ((stat = acquire_sem(mutex->Lock)) < B_NO_ERROR) {
            /* Oh dear, acquire_sem failed!!  */
		    atomic_add(&mutex->LockCount, -1);
		    return stat;
		}
	}

    mutex->owner = me;
    mutex->owner_ref = 1;
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_trylock(apr_thread_mutex_t *mutex)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_unlock(apr_thread_mutex_t *mutex)
{
    int32 stat;
        
    if (mutex->nested && mutex->owner == find_thread(NULL)) {
        mutex->owner_ref--;
        if (mutex->owner_ref > 0)
            return APR_SUCCESS;
    }
    
	if (atomic_add(&mutex->LockCount, -1) > 1) {
        if ((stat = release_sem(mutex->Lock)) < B_NO_ERROR) {
            atomic_add(&mutex->LockCount, 1);
            return stat;
        }
    }

    mutex->owner = -1;
    mutex->owner_ref = 0;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_mutex_destroy(apr_thread_mutex_t *mutex)
{
    apr_status_t stat;
    if ((stat = _thread_mutex_cleanup(mutex)) == APR_SUCCESS) {
        apr_pool_cleanup_kill(mutex->pool, mutex, _thread_mutex_cleanup);
        return APR_SUCCESS;
    }
    return stat;
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_mutex)

