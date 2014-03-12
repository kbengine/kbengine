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

#include "apr_arch_thread_mutex.h"
#include "apr_arch_thread_cond.h"
#include "apr_strings.h"
#include "apr_portable.h"

static apr_status_t thread_cond_cleanup(void *data)
{
    struct waiter *w;
    apr_thread_cond_t *cond = (apr_thread_cond_t *)data;

    acquire_sem(cond->lock);
    delete_sem(cond->lock);

    return APR_SUCCESS;
}

static struct waiter_t *make_waiter(apr_pool_t *pool)
{
    struct waiter_t *w = (struct waiter_t*)
                       apr_palloc(pool, sizeof(struct waiter_t));
    if (w == NULL)
        return NULL;
      
    w->sem  = create_sem(0, "apr conditional waiter");
    if (w->sem < 0)
        return NULL;

    APR_RING_ELEM_INIT(w, link);
    
    return w;
}
  
APR_DECLARE(apr_status_t) apr_thread_cond_create(apr_thread_cond_t **cond,
                                                 apr_pool_t *pool)
{
    apr_thread_cond_t *new_cond;
    sem_id rv;
    int i;

    new_cond = (apr_thread_cond_t *)apr_palloc(pool, sizeof(apr_thread_cond_t));

    if (new_cond == NULL)
        return APR_ENOMEM;

    if ((rv = create_sem(1, "apr conditional lock")) < B_OK)
        return rv;
    
    new_cond->lock = rv;
    new_cond->pool = pool;
    APR_RING_INIT(&new_cond->alist, waiter_t, link);
    APR_RING_INIT(&new_cond->flist, waiter_t, link);
        
    for (i=0;i < 10 ;i++) {
        struct waiter_t *nw = make_waiter(pool);
        APR_RING_INSERT_TAIL(&new_cond->flist, nw, waiter_t, link);
    }

    apr_pool_cleanup_register(new_cond->pool,
                              (void *)new_cond, thread_cond_cleanup,
                              apr_pool_cleanup_null);

    *cond = new_cond;
    return APR_SUCCESS;
}


static apr_status_t do_wait(apr_thread_cond_t *cond, apr_thread_mutex_t *mutex,
                            int timeout)
{
    struct waiter_t *wait;
    thread_id cth = find_thread(NULL);
    apr_status_t rv;
    int flags = B_RELATIVE_TIMEOUT;
    
    /* We must be the owner of the mutex or we can't do this... */    
    if (mutex->owner != cth) {
        /* What should we return??? */
        return APR_EINVAL;
    }

    acquire_sem(cond->lock);
    wait = APR_RING_FIRST(&cond->flist);
    if (wait)
        APR_RING_REMOVE(wait, link);
    else
        wait = make_waiter(cond->pool);   
    APR_RING_INSERT_TAIL(&cond->alist, wait, waiter_t, link);
    cond->condlock = mutex;
    release_sem(cond->lock);
       
    apr_thread_mutex_unlock(cond->condlock);

    if (timeout == 0)
        flags = 0;
        
    rv = acquire_sem_etc(wait->sem, 1, flags, timeout);

    apr_thread_mutex_lock(cond->condlock);
    
    if (rv != B_OK)
        if (rv == B_TIMED_OUT)
            return APR_TIMEUP;
        return rv;       

    acquire_sem(cond->lock);
    APR_RING_REMOVE(wait, link);
    APR_RING_INSERT_TAIL(&cond->flist, wait, waiter_t, link);
    release_sem(cond->lock);
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_wait(apr_thread_cond_t *cond,
                                               apr_thread_mutex_t *mutex)
{
    return do_wait(cond, mutex, 0);
}

APR_DECLARE(apr_status_t) apr_thread_cond_timedwait(apr_thread_cond_t *cond,
                                                    apr_thread_mutex_t *mutex,
                                                    apr_interval_time_t timeout)
{
    return do_wait(cond, mutex, timeout);
}

APR_DECLARE(apr_status_t) apr_thread_cond_signal(apr_thread_cond_t *cond)
{
    struct waiter_t *wake;

    acquire_sem(cond->lock);    
    if (!APR_RING_EMPTY(&cond->alist, waiter_t, link)) {
        wake = APR_RING_FIRST(&cond->alist);
        APR_RING_REMOVE(wake, link);
        release_sem(wake->sem);
        APR_RING_INSERT_TAIL(&cond->flist, wake, waiter_t, link);
    }
    release_sem(cond->lock);
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_broadcast(apr_thread_cond_t *cond)
{
    struct waiter_t *wake;
    
    acquire_sem(cond->lock);
    while (! APR_RING_EMPTY(&cond->alist, waiter_t, link)) {
        wake = APR_RING_FIRST(&cond->alist);
        APR_RING_REMOVE(wake, link);
        release_sem(wake->sem);
        APR_RING_INSERT_TAIL(&cond->flist, wake, waiter_t, link);
    }
    release_sem(cond->lock);
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_thread_cond_destroy(apr_thread_cond_t *cond)
{
    apr_status_t stat;
    if ((stat = thread_cond_cleanup(cond)) == APR_SUCCESS) {
        apr_pool_cleanup_kill(cond->pool, cond, thread_cond_cleanup);
        return APR_SUCCESS;
    }
    return stat;
}

APR_POOL_IMPLEMENT_ACCESSOR(thread_cond)

