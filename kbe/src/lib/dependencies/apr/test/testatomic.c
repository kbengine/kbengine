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

#include "testutil.h"
#include "apr_strings.h"
#include "apr_thread_proc.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_atomic.h"
#include "apr_time.h"

/* Use pthread_setconcurrency where it is available and not a nullop,
 * i.e. platforms using M:N or M:1 thread models: */
#if APR_HAS_THREADS && \
   ((defined(SOLARIS2) && SOLARIS2 > 6) || defined(_AIX))
/* also HP-UX, IRIX? ... */
#define HAVE_PTHREAD_SETCONCURRENCY
#endif

#ifdef HAVE_PTHREAD_SETCONCURRENCY
#include <pthread.h>
#endif

static void test_init(abts_case *tc, void *data)
{
    APR_ASSERT_SUCCESS(tc, "Could not initliaze atomics", apr_atomic_init(p));
}

static void test_set32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_atomic_set32(&y32, 2);
    ABTS_INT_EQUAL(tc, 2, y32);
}

static void test_read32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_atomic_set32(&y32, 2);
    ABTS_INT_EQUAL(tc, 2, apr_atomic_read32(&y32));
}

static void test_dec32(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    int rv;

    apr_atomic_set32(&y32, 2);

    rv = apr_atomic_dec32(&y32);
    ABTS_INT_EQUAL(tc, 1, y32);
    ABTS_ASSERT(tc, "atomic_dec returned zero when it shouldn't", rv != 0);

    rv = apr_atomic_dec32(&y32);
    ABTS_INT_EQUAL(tc, 0, y32);
    ABTS_ASSERT(tc, "atomic_dec didn't returned zero when it should", rv == 0);
}

static void test_xchg32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 100);
    oldval = apr_atomic_xchg32(&y32, 50);

    ABTS_INT_EQUAL(tc, 100, oldval);
    ABTS_INT_EQUAL(tc, 50, y32);
}

static void test_xchgptr(abts_case *tc, void *data)
{
    int a;
    void *ref = "little piggy";
    volatile void *target_ptr = ref;
    void *old_ptr;

    old_ptr = apr_atomic_xchgptr(&target_ptr, &a);
    ABTS_PTR_EQUAL(tc, ref, old_ptr);
    ABTS_PTR_EQUAL(tc, &a, (void *) target_ptr);
}

static void test_cas_equal(abts_case *tc, void *data)
{
    apr_uint32_t casval = 0;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 12, 0);
    ABTS_INT_EQUAL(tc, 0, oldval);
    ABTS_INT_EQUAL(tc, 12, casval);
}

static void test_cas_equal_nonnull(abts_case *tc, void *data)
{
    apr_uint32_t casval = 12;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 23, 12);
    ABTS_INT_EQUAL(tc, 12, oldval);
    ABTS_INT_EQUAL(tc, 23, casval);
}

static void test_cas_notequal(abts_case *tc, void *data)
{
    apr_uint32_t casval = 12;
    apr_uint32_t oldval;

    oldval = apr_atomic_cas32(&casval, 23, 2);
    ABTS_INT_EQUAL(tc, 12, oldval);
    ABTS_INT_EQUAL(tc, 12, casval);
}

static void test_casptr_equal(abts_case *tc, void *data)
{
    int a;
    volatile void *target_ptr = NULL;
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &a, NULL);
    ABTS_PTR_EQUAL(tc, NULL, old_ptr);
    ABTS_PTR_EQUAL(tc, &a, (void *) target_ptr);
}

static void test_casptr_equal_nonnull(abts_case *tc, void *data)
{
    int a, b;
    volatile void *target_ptr = &a;
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &b, &a);
    ABTS_PTR_EQUAL(tc, &a, old_ptr);
    ABTS_PTR_EQUAL(tc, &b, (void *) target_ptr);
}

static void test_casptr_notequal(abts_case *tc, void *data)
{
    int a, b;
    volatile void *target_ptr = &a;
    void *old_ptr;

    old_ptr = apr_atomic_casptr(&target_ptr, &a, &b);
    ABTS_PTR_EQUAL(tc, &a, old_ptr);
    ABTS_PTR_EQUAL(tc, &a, (void *) target_ptr);
}

static void test_add32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_add32(&y32, 4);
    ABTS_INT_EQUAL(tc, 23, oldval);
    ABTS_INT_EQUAL(tc, 27, y32);
}

static void test_add32_neg(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_add32(&y32, -10);
    ABTS_INT_EQUAL(tc, 23, oldval);
    ABTS_INT_EQUAL(tc, 13, y32);
}

static void test_inc32(abts_case *tc, void *data)
{
    apr_uint32_t oldval;
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 23);
    oldval = apr_atomic_inc32(&y32);
    ABTS_INT_EQUAL(tc, 23, oldval);
    ABTS_INT_EQUAL(tc, 24, y32);
}

static void test_set_add_inc_sub(abts_case *tc, void *data)
{
    apr_uint32_t y32;

    apr_atomic_set32(&y32, 0);
    apr_atomic_add32(&y32, 20);
    apr_atomic_inc32(&y32);
    apr_atomic_sub32(&y32, 10);

    ABTS_INT_EQUAL(tc, 11, y32);
}

static void test_wrap_zero(abts_case *tc, void *data)
{
    apr_uint32_t y32;
    apr_uint32_t rv;
    apr_uint32_t minus1 = -1;
    char *str;

    apr_atomic_set32(&y32, 0);
    rv = apr_atomic_dec32(&y32);

    ABTS_ASSERT(tc, "apr_atomic_dec32 on zero returned zero.", rv != 0);
    str = apr_psprintf(p, "zero wrap failed: 0 - 1 = %d", y32);
    ABTS_ASSERT(tc, str, y32 == minus1);
}

static void test_inc_neg1(abts_case *tc, void *data)
{
    apr_uint32_t y32 = -1;
    apr_uint32_t minus1 = -1;
    apr_uint32_t rv;
    char *str;

    rv = apr_atomic_inc32(&y32);

    ABTS_ASSERT(tc, "apr_atomic_inc32 didn't return the old value.", rv == minus1);
    str = apr_psprintf(p, "zero wrap failed: -1 + 1 = %d", y32);
    ABTS_ASSERT(tc, str, y32 == 0);
}


#if APR_HAS_THREADS

void *APR_THREAD_FUNC thread_func_mutex(apr_thread_t *thd, void *data);
void *APR_THREAD_FUNC thread_func_atomic(apr_thread_t *thd, void *data);

apr_thread_mutex_t *thread_lock;
volatile apr_uint32_t mutex_locks = 0;
volatile apr_uint32_t atomic_ops = 0;
apr_status_t exit_ret_val = 123; /* just some made up number to check on later */

#define NUM_THREADS 40
#define NUM_ITERATIONS 20000

void *APR_THREAD_FUNC thread_func_mutex(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < NUM_ITERATIONS; i++) {
        apr_thread_mutex_lock(thread_lock);
        mutex_locks++;
        apr_thread_mutex_unlock(thread_lock);
    }
    apr_thread_exit(thd, exit_ret_val);
    return NULL;
}

void *APR_THREAD_FUNC thread_func_atomic(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < NUM_ITERATIONS ; i++) {
        apr_atomic_inc32(&atomic_ops);
        apr_atomic_add32(&atomic_ops, 2);
        apr_atomic_dec32(&atomic_ops);
        apr_atomic_dec32(&atomic_ops);
    }
    apr_thread_exit(thd, exit_ret_val);
    return NULL;
}

static void test_atomics_threaded(abts_case *tc, void *data)
{
    apr_thread_t *t1[NUM_THREADS];
    apr_thread_t *t2[NUM_THREADS];
    apr_status_t rv;
    int i;

#ifdef HAVE_PTHREAD_SETCONCURRENCY
    pthread_setconcurrency(8);
#endif

    rv = apr_thread_mutex_create(&thread_lock, APR_THREAD_MUTEX_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "Could not create lock", rv);

    for (i = 0; i < NUM_THREADS; i++) {
        apr_status_t r1, r2;
        r1 = apr_thread_create(&t1[i], NULL, thread_func_mutex, NULL, p);
        r2 = apr_thread_create(&t2[i], NULL, thread_func_atomic, NULL, p);
        ABTS_ASSERT(tc, "Failed creating threads", !r1 && !r2);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        apr_status_t s1, s2;
        apr_thread_join(&s1, t1[i]);
        apr_thread_join(&s2, t2[i]);

        ABTS_ASSERT(tc, "Invalid return value from thread_join",
                    s1 == exit_ret_val && s2 == exit_ret_val);
    }

    ABTS_INT_EQUAL(tc, NUM_THREADS * NUM_ITERATIONS, mutex_locks);
    ABTS_INT_EQUAL(tc, NUM_THREADS * NUM_ITERATIONS,
                   apr_atomic_read32(&atomic_ops));

    rv = apr_thread_mutex_destroy(thread_lock);
    ABTS_ASSERT(tc, "Failed creating threads", rv == APR_SUCCESS);
}

#undef NUM_THREADS
#define NUM_THREADS 7

typedef struct tbox_t tbox_t;

struct tbox_t {
    abts_case *tc;
    apr_uint32_t *mem;
    apr_uint32_t preval;
    apr_uint32_t postval;
    apr_uint32_t loop;
    void (*func)(tbox_t *box);
};

static APR_INLINE void busyloop_read32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        val = apr_atomic_read32(tbox->mem);

        if (val != tbox->preval)
            apr_thread_yield();
        else
            break;
    } while (1);
}

static void busyloop_set32(tbox_t *tbox)
{
    do {
        busyloop_read32(tbox);
        apr_atomic_set32(tbox->mem, tbox->postval);
    } while (--tbox->loop);
}

static void busyloop_add32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_add32(tbox->mem, tbox->postval);
        apr_thread_mutex_lock(thread_lock);
        ABTS_INT_EQUAL(tbox->tc, val, tbox->preval);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_sub32(tbox_t *tbox)
{
    do {
        busyloop_read32(tbox);
        apr_atomic_sub32(tbox->mem, tbox->postval);
    } while (--tbox->loop);
}

static void busyloop_inc32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_inc32(tbox->mem);
        apr_thread_mutex_lock(thread_lock);
        ABTS_INT_EQUAL(tbox->tc, val, tbox->preval);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_dec32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_dec32(tbox->mem);
        apr_thread_mutex_lock(thread_lock);
        ABTS_INT_NEQUAL(tbox->tc, 0, val);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void busyloop_cas32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        do {
            val = apr_atomic_cas32(tbox->mem, tbox->postval, tbox->preval);

            if (val != tbox->preval)
                apr_thread_yield();
            else
                break;
        } while (1);
    } while (--tbox->loop);
}

static void busyloop_xchg32(tbox_t *tbox)
{
    apr_uint32_t val;

    do {
        busyloop_read32(tbox);
        val = apr_atomic_xchg32(tbox->mem, tbox->postval);
        apr_thread_mutex_lock(thread_lock);
        ABTS_INT_EQUAL(tbox->tc, val, tbox->preval);
        apr_thread_mutex_unlock(thread_lock);
    } while (--tbox->loop);
}

static void *APR_THREAD_FUNC thread_func_busyloop(apr_thread_t *thd, void *data)
{
    tbox_t *tbox = data;

    tbox->func(tbox);

    apr_thread_exit(thd, 0);

    return NULL;
}

static void test_atomics_busyloop_threaded(abts_case *tc, void *data)
{
    unsigned int i;
    apr_status_t rv;
    apr_uint32_t count = 0;
    tbox_t tbox[NUM_THREADS];
    apr_thread_t *thread[NUM_THREADS];

    rv = apr_thread_mutex_create(&thread_lock, APR_THREAD_MUTEX_DEFAULT, p);
    APR_ASSERT_SUCCESS(tc, "Could not create lock", rv);

    /* get ready */
    for (i = 0; i < NUM_THREADS; i++) {
        tbox[i].tc = tc;
        tbox[i].mem = &count;
        tbox[i].loop = 50;
    }

    tbox[0].preval = 98;
    tbox[0].postval = 3891;
    tbox[0].func = busyloop_add32;

    tbox[1].preval = 3989;
    tbox[1].postval = 1010;
    tbox[1].func = busyloop_sub32;

    tbox[2].preval = 2979;
    tbox[2].postval = 0; /* not used */
    tbox[2].func = busyloop_inc32;

    tbox[3].preval = 2980;
    tbox[3].postval = 16384;
    tbox[3].func = busyloop_set32;

    tbox[4].preval = 16384;
    tbox[4].postval = 0; /* not used */
    tbox[4].func = busyloop_dec32;

    tbox[5].preval = 16383;
    tbox[5].postval = 1048576;
    tbox[5].func = busyloop_cas32;

    tbox[6].preval = 1048576;
    tbox[6].postval = 98; /* goto tbox[0] */
    tbox[6].func = busyloop_xchg32;

    /* get set */
    for (i = 0; i < NUM_THREADS; i++) {
        rv = apr_thread_create(&thread[i], NULL, thread_func_busyloop,
                               &tbox[i], p);
        ABTS_ASSERT(tc, "Failed creating thread", rv == APR_SUCCESS);
    }

    /* go! */
    apr_atomic_set32(tbox->mem, 98);

    for (i = 0; i < NUM_THREADS; i++) {
        apr_status_t retval;
        rv = apr_thread_join(&retval, thread[i]);
        ABTS_ASSERT(tc, "Thread join failed", rv == APR_SUCCESS);
        ABTS_ASSERT(tc, "Invalid return value from thread_join", retval == 0);
    }

    ABTS_INT_EQUAL(tbox->tc, 98, count);

    rv = apr_thread_mutex_destroy(thread_lock);
    ABTS_ASSERT(tc, "Failed creating threads", rv == APR_SUCCESS);
}

#endif /* !APR_HAS_THREADS */

abts_suite *testatomic(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    abts_run_test(suite, test_init, NULL);
    abts_run_test(suite, test_set32, NULL);
    abts_run_test(suite, test_read32, NULL);
    abts_run_test(suite, test_dec32, NULL);
    abts_run_test(suite, test_xchg32, NULL);
    abts_run_test(suite, test_xchgptr, NULL);
    abts_run_test(suite, test_cas_equal, NULL);
    abts_run_test(suite, test_cas_equal_nonnull, NULL);
    abts_run_test(suite, test_cas_notequal, NULL);
    abts_run_test(suite, test_casptr_equal, NULL);
    abts_run_test(suite, test_casptr_equal_nonnull, NULL);
    abts_run_test(suite, test_casptr_notequal, NULL);
    abts_run_test(suite, test_add32, NULL);
    abts_run_test(suite, test_add32_neg, NULL);
    abts_run_test(suite, test_inc32, NULL);
    abts_run_test(suite, test_set_add_inc_sub, NULL);
    abts_run_test(suite, test_wrap_zero, NULL);
    abts_run_test(suite, test_inc_neg1, NULL);

#if APR_HAS_THREADS
    abts_run_test(suite, test_atomics_threaded, NULL);
    abts_run_test(suite, test_atomics_busyloop_threaded, NULL);
#endif

    return suite;
}

