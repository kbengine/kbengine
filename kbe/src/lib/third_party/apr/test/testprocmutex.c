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

#include "apr_shm.h"
#include "apr_thread_proc.h"
#include "apr_file_io.h"
#include "apr_proc_mutex.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_getopt.h"
#include <stdio.h>
#include <stdlib.h>
#include "testutil.h"

#if APR_HAS_FORK

#define MAX_ITER 200
#define CHILDREN 6
#define MAX_COUNTER (MAX_ITER * CHILDREN)
#define MAX_WAIT_USEC (1000*1000)

static apr_proc_mutex_t *proc_lock;
static volatile int *x;

/* a slower more racy way to implement (*x)++ */
static int increment(int n)
{
    apr_sleep(1);
    return n+1;
}

static void make_child(abts_case *tc, int trylock, apr_proc_t **proc, apr_pool_t *p)
{
    apr_status_t rv;

    *proc = apr_pcalloc(p, sizeof(**proc));

    /* slight delay to allow things to settle */
    apr_sleep (1);

    rv = apr_proc_fork(*proc, p);
    if (rv == APR_INCHILD) {
        int i = 0;
        /* The parent process has setup all processes to call apr_terminate
         * at exit.  But, that means that all processes must also call
         * apr_initialize at startup.  You cannot have an unequal number
         * of apr_terminate and apr_initialize calls.  If you do, bad things
         * will happen.  In this case, the bad thing is that if the mutex
         * is a semaphore, it will be destroyed before all of the processes
         * die.  That means that the test will most likely fail.
         */
        apr_initialize();

        if (apr_proc_mutex_child_init(&proc_lock, NULL, p))
            exit(1);

        do {
            if (trylock) {
                int wait_usec = 0;

                while ((rv = apr_proc_mutex_trylock(proc_lock))) {
                    if (!APR_STATUS_IS_EBUSY(rv))
                        exit(1);
                    if (++wait_usec >= MAX_WAIT_USEC)
                        exit(1);
                    apr_sleep(1);
                }
            }
            else {
                if (apr_proc_mutex_lock(proc_lock))
                    exit(1);
            }

            i++;
            *x = increment(*x);
            if (apr_proc_mutex_unlock(proc_lock))
                exit(1);
        } while (i < MAX_ITER);
        exit(0);
    } 

    ABTS_ASSERT(tc, "fork failed", rv == APR_INPARENT);
}

/* Wait for a child process and check it terminated with success. */
static void await_child(abts_case *tc, apr_proc_t *proc)
{
    int code;
    apr_exit_why_e why;
    apr_status_t rv;

    rv = apr_proc_wait(proc, &code, &why, APR_WAIT);
    ABTS_ASSERT(tc, "child did not terminate with success",
             rv == APR_CHILD_DONE && why == APR_PROC_EXIT && code == 0);
}

static void test_exclusive(abts_case *tc, const char *lockname, 
                           apr_lockmech_e mech)
{
    apr_proc_t *child[CHILDREN];
    apr_status_t rv;
    int n;
 
    rv = apr_proc_mutex_create(&proc_lock, lockname, mech, p);
    APR_ASSERT_SUCCESS(tc, "create the mutex", rv);
    if (rv != APR_SUCCESS)
        return;
 
    for (n = 0; n < CHILDREN; n++)
        make_child(tc, 0, &child[n], p);

    for (n = 0; n < CHILDREN; n++)
        await_child(tc, child[n]);
    
    ABTS_ASSERT(tc, "Locks don't appear to work", *x == MAX_COUNTER);

    rv = apr_proc_mutex_trylock(proc_lock);
    if (rv == APR_ENOTIMPL) {
        ABTS_NOT_IMPL(tc, "apr_proc_mutex_trylock not implemented");
        return;
    }
    APR_ASSERT_SUCCESS(tc, "check for trylock", rv);

    rv = apr_proc_mutex_unlock(proc_lock);
    APR_ASSERT_SUCCESS(tc, "unlock after trylock check", rv);

    *x = 0;

    for (n = 0; n < CHILDREN; n++)
        make_child(tc, 1, &child[n], p);

    for (n = 0; n < CHILDREN; n++)
        await_child(tc, child[n]);
    
    ABTS_ASSERT(tc, "Locks don't appear to work with trylock",
                *x == MAX_COUNTER);
}
#endif

static void proc_mutex(abts_case *tc, void *data)
{
#if APR_HAS_FORK
    apr_status_t rv;
    const char *shmname = "tpm.shm";
    apr_shm_t *shm;
    apr_lockmech_e *mech = data;

    /* Use anonymous shm if available. */
    rv = apr_shm_create(&shm, sizeof(int), NULL, p);
    if (rv == APR_ENOTIMPL) {
        apr_file_remove(shmname, p);
        rv = apr_shm_create(&shm, sizeof(int), shmname, p);
    }

    APR_ASSERT_SUCCESS(tc, "create shm segment", rv);
    if (rv != APR_SUCCESS)
        return;

    x = apr_shm_baseaddr_get(shm);
    test_exclusive(tc, NULL, *mech);
    rv = apr_shm_destroy(shm);
    APR_ASSERT_SUCCESS(tc, "Error destroying shared memory block", rv);
#else
    ABTS_NOT_IMPL(tc, "APR lacks fork() support");
#endif
}


abts_suite *testprocmutex(abts_suite *suite)
{
    apr_lockmech_e mech = APR_LOCK_DEFAULT;

    suite = ADD_SUITE(suite)
    abts_run_test(suite, proc_mutex, &mech);
#if APR_HAS_POSIXSEM_SERIALIZE
    mech = APR_LOCK_POSIXSEM;
    abts_run_test(suite, proc_mutex, &mech);
#endif
#if APR_HAS_SYSVSEM_SERIALIZE
    mech = APR_LOCK_SYSVSEM;
    abts_run_test(suite, proc_mutex, &mech);
#endif
#if APR_HAS_PROC_PTHREAD_SERIALIZE
    mech = APR_LOCK_PROC_PTHREAD;
    abts_run_test(suite, proc_mutex, &mech);
#endif
#if APR_HAS_FCNTL_SERIALIZE
    mech = APR_LOCK_FCNTL;
    abts_run_test(suite, proc_mutex, &mech);
#endif
#if APR_HAS_FLOCK_SERIALIZE
    mech = APR_LOCK_FLOCK;
    abts_run_test(suite, proc_mutex, &mech);
#endif

    return suite;
}
