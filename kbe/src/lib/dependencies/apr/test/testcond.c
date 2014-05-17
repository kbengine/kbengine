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

#include "apr_file_io.h"
#include "apr_thread_proc.h"
#include "apr_thread_mutex.h"
#include "apr_thread_cond.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_atomic.h"
#include "testutil.h"

#define NTHREADS 10

#define ABTS_SUCCESS(rv)    ABTS_INT_EQUAL(tc, APR_SUCCESS, rv)

#if APR_HAS_THREADS

typedef struct toolbox_t toolbox_t;

struct toolbox_t {
    void *data;
    abts_case *tc;
    apr_thread_mutex_t *mutex;
    apr_thread_cond_t *cond;
    void (*func)(toolbox_t *box);
};

typedef struct toolbox_fnptr_t toolbox_fnptr_t;

struct toolbox_fnptr_t {
    void (*func)(toolbox_t *box);
};

static void lost_signal(abts_case *tc, void *data)
{
    apr_status_t rv;
    apr_thread_cond_t *cond = NULL;
    apr_thread_mutex_t *mutex = NULL;

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_DEFAULT, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, mutex);

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    rv = apr_thread_cond_signal(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_timedwait(cond, mutex, 10000);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_broadcast(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_timedwait(cond, mutex, 10000);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_destroy(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_destroy(mutex);
    ABTS_SUCCESS(rv);
}

static void *APR_THREAD_FUNC thread_routine(apr_thread_t *thd, void *data)
{
    toolbox_t *box = data;

    box->func(box);

    apr_thread_exit(thd, 0);

    return NULL;
}

static void lock_and_signal(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_signal(box->cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);
}

static void dynamic_binding(abts_case *tc, void *data)
{
    unsigned int i;
    apr_status_t rv;
    toolbox_t box[NTHREADS];
    apr_thread_t *thread[NTHREADS];
    apr_thread_mutex_t *mutex[NTHREADS];
    apr_thread_cond_t *cond = NULL;

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    for (i = 0; i < NTHREADS; i++) {
        rv = apr_thread_mutex_create(&mutex[i], APR_THREAD_MUTEX_DEFAULT, p);
        ABTS_SUCCESS(rv);

        rv = apr_thread_mutex_lock(mutex[i]);
        ABTS_SUCCESS(rv);

        box[i].tc = tc;
        box[i].cond = cond;
        box[i].mutex = mutex[i];
        box[i].func = lock_and_signal;

        rv = apr_thread_create(&thread[i], NULL, thread_routine, &box[i], p);
        ABTS_SUCCESS(rv);
    }

    /*
     * The dynamic binding should be preserved because we use only one waiter
     */

    for (i = 0; i < NTHREADS; i++) {
        rv = apr_thread_cond_wait(cond, mutex[i]);
        ABTS_SUCCESS(rv);
    }

    for (i = 0; i < NTHREADS; i++) {
        rv = apr_thread_cond_timedwait(cond, mutex[i], 10000);
        ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_TIMEUP(rv));

        rv = apr_thread_mutex_unlock(mutex[i]);
        ABTS_SUCCESS(rv);
    }

    for (i = 0; i < NTHREADS; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        ABTS_SUCCESS(rv);
    }

    rv = apr_thread_cond_destroy(cond);
    ABTS_SUCCESS(rv);

    for (i = 0; i < NTHREADS; i++) {
        rv = apr_thread_mutex_destroy(mutex[i]);
        ABTS_SUCCESS(rv);
    }
}

static void lock_and_wait(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;
    apr_uint32_t *count = box->data;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    apr_atomic_inc32(count);

    rv = apr_thread_cond_wait(box->cond, box->mutex);
    ABTS_SUCCESS(rv);

    apr_atomic_dec32(count);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);
}

static void broadcast_threads(abts_case *tc, void *data)
{
    toolbox_t box;
    unsigned int i;
    apr_status_t rv;
    apr_uint32_t count = 0;
    apr_thread_cond_t *cond = NULL;
    apr_thread_mutex_t *mutex = NULL;
    apr_thread_t *thread[NTHREADS];

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_DEFAULT, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, mutex);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    box.tc = tc;
    box.data = &count;
    box.mutex = mutex;
    box.cond = cond;
    box.func = lock_and_wait;

    for (i = 0; i < NTHREADS; i++) {
        rv = apr_thread_create(&thread[i], NULL, thread_routine, &box, p);
        ABTS_SUCCESS(rv);
    }

    do {
        rv = apr_thread_mutex_unlock(mutex);
        ABTS_SUCCESS(rv);
        apr_sleep(100000);
        rv = apr_thread_mutex_lock(mutex);
        ABTS_SUCCESS(rv);
    } while (apr_atomic_read32(&count) != NTHREADS);

    rv = apr_thread_cond_broadcast(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    for (i = 0; i < NTHREADS; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        ABTS_SUCCESS(rv);
    }

    ABTS_INT_EQUAL(tc, 0, count);

    rv = apr_thread_cond_destroy(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_destroy(mutex);
    ABTS_SUCCESS(rv);
}

static void nested_lock_and_wait(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_wait(box->cond, box->mutex);
    ABTS_SUCCESS(rv);
}

static void nested_lock_and_unlock(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_timedwait(box->cond, box->mutex, 2000000);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);
}

static void nested_wait(abts_case *tc, void *data)
{
    toolbox_fnptr_t *fnptr = data;
    toolbox_t box;
    apr_status_t rv, retval;
    apr_thread_cond_t *cond = NULL;
    apr_thread_t *thread = NULL;
    apr_thread_mutex_t *mutex = NULL;

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_NESTED, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, mutex);

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    box.tc = tc;
    box.cond = cond;
    box.mutex = mutex;
    box.func = fnptr->func;

    rv = apr_thread_create(&thread, NULL, thread_routine, &box, p);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    /* yield the processor */
    apr_sleep(500000);

    rv = apr_thread_cond_signal(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_join(&retval, thread);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_trylock(mutex);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EBUSY(rv));

    rv = apr_thread_mutex_trylock(mutex);
    ABTS_INT_EQUAL(tc, 1, APR_STATUS_IS_EBUSY(rv));
}

static volatile apr_uint64_t pipe_count;
static volatile apr_uint32_t exiting;

static void pipe_consumer(toolbox_t *box)
{
    char ch;
    apr_status_t rv;
    apr_size_t nbytes;
    abts_case *tc = box->tc;
    apr_file_t *out = box->data;
    apr_uint32_t consumed = 0;

    do {
        rv = apr_thread_mutex_lock(box->mutex);
        ABTS_SUCCESS(rv);

        while (!pipe_count && !exiting) {
            rv = apr_thread_cond_wait(box->cond, box->mutex);
            ABTS_SUCCESS(rv);
        }

        if (!pipe_count && exiting) {
            rv = apr_thread_mutex_unlock(box->mutex);
            ABTS_SUCCESS(rv);
            break;
        }

        pipe_count--;
        consumed++;

        rv = apr_thread_mutex_unlock(box->mutex);
        ABTS_SUCCESS(rv);

        rv = apr_file_read_full(out, &ch, 1, &nbytes);
        ABTS_SUCCESS(rv);
        ABTS_SIZE_EQUAL(tc, 1, nbytes);
        ABTS_TRUE(tc, ch == '.');
    } while (1);

    /* naive fairness test - it would be good to introduce or solidify
     * a solid test to ensure one thread is not starved.  
     * ABTS_INT_EQUAL(tc, 1, !!consumed);
     */
}

static void pipe_write(toolbox_t *box, char ch)
{
    apr_status_t rv;
    apr_size_t nbytes;
    abts_case *tc = box->tc;
    apr_file_t *in = box->data;

    rv = apr_file_write_full(in, &ch, 1, &nbytes);
    ABTS_SUCCESS(rv);
    ABTS_SIZE_EQUAL(tc, 1, nbytes);

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    if (!pipe_count) {
        rv = apr_thread_cond_signal(box->cond);
        ABTS_SUCCESS(rv);
    }

    pipe_count++;

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);
}

static void pipe_producer(toolbox_t *box)
{
    apr_uint32_t loop = 500;

    do {
        pipe_write(box, '.');
    } while (loop--);
}

static void pipe_producer_consumer(abts_case *tc, void *data)
{
    apr_status_t rv;
    toolbox_t boxcons, boxprod;
    apr_thread_t *thread[NTHREADS];
    apr_thread_cond_t *cond = NULL;
    apr_thread_mutex_t *mutex = NULL;
    apr_file_t *in = NULL, *out = NULL;
    apr_uint32_t i, ncons = (apr_uint32_t)(NTHREADS * 0.70);

    rv = apr_file_pipe_create(&in, &out, p);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_DEFAULT, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, mutex);

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    boxcons.tc = tc;
    boxcons.data = in;
    boxcons.mutex = mutex;
    boxcons.cond = cond;
    boxcons.func = pipe_consumer;

    for (i = 0; i < ncons; i++) {
        rv = apr_thread_create(&thread[i], NULL, thread_routine, &boxcons, p);
        ABTS_SUCCESS(rv);
    }

    boxprod.tc = tc;
    boxprod.data = out;
    boxprod.mutex = mutex;
    boxprod.cond = cond;
    boxprod.func = pipe_producer;

    for (; i < NTHREADS; i++) {
        rv = apr_thread_create(&thread[i], NULL, thread_routine, &boxprod, p);
        ABTS_SUCCESS(rv);
    }

    for (i = ncons; i < NTHREADS; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        ABTS_SUCCESS(rv);
    }

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    exiting = 1;

    rv = apr_thread_cond_broadcast(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    for (i = 0; i < ncons; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        ABTS_SUCCESS(rv);
    }

    rv = apr_thread_cond_destroy(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_destroy(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_file_close(in);
    ABTS_SUCCESS(rv);

    rv = apr_file_close(out);
    ABTS_SUCCESS(rv);
}

volatile enum {
    TOSS,
    PING,
    PONG,
    OVER
} state;

static void ping(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    if (state == TOSS)
        state = PING;

    do {
        rv = apr_thread_cond_signal(box->cond);
        ABTS_SUCCESS(rv);

        state = PONG;

        rv = apr_thread_cond_wait(box->cond, box->mutex);
        ABTS_SUCCESS(rv);

        ABTS_TRUE(tc, state == PING || state == OVER);
    } while (state != OVER);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_broadcast(box->cond);
    ABTS_SUCCESS(rv);
}

static void pong(toolbox_t *box)
{
    apr_status_t rv;
    abts_case *tc = box->tc;

    rv = apr_thread_mutex_lock(box->mutex);
    ABTS_SUCCESS(rv);

    if (state == TOSS)
        state = PONG;

    do {
        rv = apr_thread_cond_signal(box->cond);
        ABTS_SUCCESS(rv);

        state = PING;

        rv = apr_thread_cond_wait(box->cond, box->mutex);
        ABTS_SUCCESS(rv);

        ABTS_TRUE(tc, state == PONG || state == OVER);
    } while (state != OVER);

    rv = apr_thread_mutex_unlock(box->mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_broadcast(box->cond);
    ABTS_SUCCESS(rv);
}

static void ping_pong(abts_case *tc, void *data)
{
    apr_status_t rv, retval;
    toolbox_t box_ping, box_pong;
    apr_thread_cond_t *cond = NULL;
    apr_thread_mutex_t *mutex = NULL;
    apr_thread_t *thr_ping = NULL, *thr_pong = NULL;

    rv = apr_thread_mutex_create(&mutex, APR_THREAD_MUTEX_DEFAULT, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, mutex);

    rv = apr_thread_cond_create(&cond, p);
    ABTS_SUCCESS(rv);
    ABTS_PTR_NOTNULL(tc, cond);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    box_ping.tc = tc;
    box_ping.data = NULL;
    box_ping.mutex = mutex;
    box_ping.cond = cond;
    box_ping.func = ping;

    rv = apr_thread_create(&thr_ping, NULL, thread_routine, &box_ping, p);
    ABTS_SUCCESS(rv);

    box_pong.tc = tc;
    box_pong.data = NULL;
    box_pong.mutex = mutex;
    box_pong.cond = cond;
    box_pong.func = pong;

    rv = apr_thread_create(&thr_pong, NULL, thread_routine, &box_pong, p);
    ABTS_SUCCESS(rv);

    state = TOSS;

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    apr_sleep(3000000);

    rv = apr_thread_mutex_lock(mutex);
    ABTS_SUCCESS(rv);

    state = OVER;

    rv = apr_thread_mutex_unlock(mutex);
    ABTS_SUCCESS(rv);

    rv = apr_thread_join(&retval, thr_ping);
    ABTS_SUCCESS(rv);

    rv = apr_thread_join(&retval, thr_pong);
    ABTS_SUCCESS(rv);

    rv = apr_thread_cond_destroy(cond);
    ABTS_SUCCESS(rv);

    rv = apr_thread_mutex_destroy(mutex);
    ABTS_SUCCESS(rv);
}
#endif /* !APR_HAS_THREADS */

#if !APR_HAS_THREADS
static void threads_not_impl(abts_case *tc, void *data)
{
    ABTS_NOT_IMPL(tc, "Threads not implemented on this platform");
}
#endif

abts_suite *testcond(abts_suite *suite)
{
#if APR_HAS_THREADS
    toolbox_fnptr_t fnptr;
#endif
    suite = ADD_SUITE(suite)

#if !APR_HAS_THREADS
    abts_run_test(suite, threads_not_impl, NULL);
#else
    abts_run_test(suite, lost_signal, NULL);
    abts_run_test(suite, dynamic_binding, NULL);
    abts_run_test(suite, broadcast_threads, NULL);
    fnptr.func = nested_lock_and_wait;
    abts_run_test(suite, nested_wait, &fnptr);
    fnptr.func = nested_lock_and_unlock;
    abts_run_test(suite, nested_wait, &fnptr);
    abts_run_test(suite, pipe_producer_consumer, NULL);
    abts_run_test(suite, ping_pong, NULL);
#endif

    return suite;
}
