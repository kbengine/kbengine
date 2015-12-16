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

#include "apr_thread_proc.h"
#include "apr_thread_mutex.h"
#include "apr_thread_rwlock.h"
#include "apr_file_io.h"
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_getopt.h"
#include "errno.h"
#include <stdio.h>
#include <stdlib.h>
#include "testutil.h"

#if !APR_HAS_THREADS
int main(void)
{
    printf("This program won't work on this platform because there is no "
           "support for threads.\n");
    return 0;
}
#else /* !APR_HAS_THREADS */

#define DEFAULT_MAX_COUNTER 1000000
#define MAX_THREADS 6

static int verbose = 0;
static long mutex_counter;
static long max_counter = DEFAULT_MAX_COUNTER;

static apr_thread_mutex_t *thread_lock;
void * APR_THREAD_FUNC thread_mutex_func(apr_thread_t *thd, void *data);
apr_status_t test_thread_mutex(int num_threads); /* apr_thread_mutex_t */

static apr_thread_rwlock_t *thread_rwlock;
void * APR_THREAD_FUNC thread_rwlock_func(apr_thread_t *thd, void *data);
apr_status_t test_thread_rwlock(int num_threads); /* apr_thread_rwlock_t */

int test_thread_mutex_nested(int num_threads);

apr_pool_t *pool;
int i = 0, x = 0;

void * APR_THREAD_FUNC thread_mutex_func(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < max_counter; i++) {
        apr_thread_mutex_lock(thread_lock);
        mutex_counter++;
        apr_thread_mutex_unlock(thread_lock);
    }
    return NULL;
}

void * APR_THREAD_FUNC thread_rwlock_func(apr_thread_t *thd, void *data)
{
    int i;

    for (i = 0; i < max_counter; i++) {
        apr_thread_rwlock_wrlock(thread_rwlock);
        mutex_counter++;
        apr_thread_rwlock_unlock(thread_rwlock);
    }
    return NULL;
}

int test_thread_mutex(int num_threads)
{
    apr_thread_t *t[MAX_THREADS];
    apr_status_t s[MAX_THREADS];
    apr_time_t time_start, time_stop;
    int i;

    mutex_counter = 0;

    printf("apr_thread_mutex_t Tests\n");
    printf("%-60s", "    Initializing the apr_thread_mutex_t (UNNESTED)");
    s[0] = apr_thread_mutex_create(&thread_lock, APR_THREAD_MUTEX_UNNESTED, pool);
    if (s[0] != APR_SUCCESS) {
        printf("Failed!\n");
        return s[0];
    }
    printf("OK\n");

    apr_thread_mutex_lock(thread_lock);
    /* set_concurrency(4)? -aaron */
    printf("    Starting %d threads    ", num_threads); 
    for (i = 0; i < num_threads; ++i) {
        s[i] = apr_thread_create(&t[i], NULL, thread_mutex_func, NULL, pool);
        if (s[i] != APR_SUCCESS) {
            printf("Failed!\n");
            return s[i];
        }
    }
    printf("OK\n");

    time_start = apr_time_now();
    apr_thread_mutex_unlock(thread_lock);

    /* printf("%-60s", "    Waiting for threads to exit"); */
    for (i = 0; i < num_threads; ++i) {
        apr_thread_join(&s[i], t[i]);
    }
    /* printf("OK\n"); */

    time_stop = apr_time_now();
    printf("microseconds: %" APR_INT64_T_FMT " usec\n",
           (time_stop - time_start));
    if (mutex_counter != max_counter * num_threads)
        printf("error: counter = %ld\n", mutex_counter);

    return APR_SUCCESS;
}

int test_thread_mutex_nested(int num_threads)
{
    apr_thread_t *t[MAX_THREADS];
    apr_status_t s[MAX_THREADS];
    apr_time_t time_start, time_stop;
    int i;

    mutex_counter = 0;

    printf("apr_thread_mutex_t Tests\n");
    printf("%-60s", "    Initializing the apr_thread_mutex_t (NESTED)");
    s[0] = apr_thread_mutex_create(&thread_lock, APR_THREAD_MUTEX_NESTED, pool);
    if (s[0] != APR_SUCCESS) {
        printf("Failed!\n");
        return s[0];
    }
    printf("OK\n");

    apr_thread_mutex_lock(thread_lock);
    /* set_concurrency(4)? -aaron */
    printf("    Starting %d threads    ", num_threads); 
    for (i = 0; i < num_threads; ++i) {
        s[i] = apr_thread_create(&t[i], NULL, thread_mutex_func, NULL, pool);
        if (s[i] != APR_SUCCESS) {
            printf("Failed!\n");
            return s[i];
        }
    }
    printf("OK\n");

    time_start = apr_time_now();
    apr_thread_mutex_unlock(thread_lock);

    /* printf("%-60s", "    Waiting for threads to exit"); */
    for (i = 0; i < num_threads; ++i) {
        apr_thread_join(&s[i], t[i]);
    }
    /* printf("OK\n"); */

    time_stop = apr_time_now();
    printf("microseconds: %" APR_INT64_T_FMT " usec\n",
           (time_stop - time_start));
    if (mutex_counter != max_counter * num_threads)
        printf("error: counter = %ld\n", mutex_counter);

    return APR_SUCCESS;
}

int test_thread_rwlock(int num_threads)
{
    apr_thread_t *t[MAX_THREADS];
    apr_status_t s[MAX_THREADS];
    apr_time_t time_start, time_stop;
    int i;

    mutex_counter = 0;

    printf("apr_thread_rwlock_t Tests\n");
    printf("%-60s", "    Initializing the apr_thread_rwlock_t");
    s[0] = apr_thread_rwlock_create(&thread_rwlock, pool);
    if (s[0] != APR_SUCCESS) {
        printf("Failed!\n");
        return s[0];
    }
    printf("OK\n");

    apr_thread_rwlock_wrlock(thread_rwlock);
    /* set_concurrency(4)? -aaron */
    printf("    Starting %d threads    ", num_threads); 
    for (i = 0; i < num_threads; ++i) {
        s[i] = apr_thread_create(&t[i], NULL, thread_rwlock_func, NULL, pool);
        if (s[i] != APR_SUCCESS) {
            printf("Failed!\n");
            return s[i];
        }
    }
    printf("OK\n");

    time_start = apr_time_now();
    apr_thread_rwlock_unlock(thread_rwlock);

    /* printf("%-60s", "    Waiting for threads to exit"); */
    for (i = 0; i < num_threads; ++i) {
        apr_thread_join(&s[i], t[i]);
    }
    /* printf("OK\n"); */

    time_stop = apr_time_now();
    printf("microseconds: %" APR_INT64_T_FMT " usec\n",
           (time_stop - time_start));
    if (mutex_counter != max_counter * num_threads)
        printf("error: counter = %ld\n", mutex_counter);

    return APR_SUCCESS;
}

int main(int argc, const char * const *argv)
{
    apr_status_t rv;
    char errmsg[200];
    apr_getopt_t *opt;
    char optchar;
    const char *optarg;

    printf("APR Lock Performance Test\n==============\n\n");
        
    apr_initialize();
    atexit(apr_terminate);

    if (apr_pool_create(&pool, NULL) != APR_SUCCESS)
        exit(-1);

    if ((rv = apr_getopt_init(&opt, pool, argc, argv)) != APR_SUCCESS) {
        fprintf(stderr, "Could not set up to parse options: [%d] %s\n",
                rv, apr_strerror(rv, errmsg, sizeof errmsg));
        exit(-1);
    }
        
    while ((rv = apr_getopt(opt, "c:v", &optchar, &optarg)) == APR_SUCCESS) {
        if (optchar == 'c') {
            max_counter = atol(optarg);
        }
        else if (optchar == 'v') {
            verbose = 1;
        }
    }

    if (rv != APR_SUCCESS && rv != APR_EOF) {
        fprintf(stderr, "Could not parse options: [%d] %s\n",
                rv, apr_strerror(rv, errmsg, sizeof errmsg));
        exit(-1);
    }

    for (i = 1; i <= MAX_THREADS; ++i) {
        if ((rv = test_thread_mutex(i)) != APR_SUCCESS) {
            fprintf(stderr,"thread_mutex test failed : [%d] %s\n",
                    rv, apr_strerror(rv, (char*)errmsg, 200));
            exit(-3);
        }

        if ((rv = test_thread_mutex_nested(i)) != APR_SUCCESS) {
            fprintf(stderr,"thread_mutex (NESTED) test failed : [%d] %s\n",
                    rv, apr_strerror(rv, (char*)errmsg, 200));
            exit(-4);
        }

        if ((rv = test_thread_rwlock(i)) != APR_SUCCESS) {
            fprintf(stderr,"thread_rwlock test failed : [%d] %s\n",
                    rv, apr_strerror(rv, (char*)errmsg, 200));
            exit(-6);
        }
    }

    return 0;
}

#endif /* !APR_HAS_THREADS */
