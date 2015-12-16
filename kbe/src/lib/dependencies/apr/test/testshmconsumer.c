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
#include "apr_errno.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_time.h"
#include "testshm.h"
#include "apr.h"

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif


#if APR_HAS_SHARED_MEMORY

static int msgwait(int sleep_sec, int first_box, int last_box)
{
    int i;
    int recvd = 0;
    apr_time_t start = apr_time_now();
    apr_interval_time_t sleep_duration = apr_time_from_sec(sleep_sec);
    while (apr_time_now() - start < sleep_duration) {
        for (i = first_box; i < last_box; i++) {
            if (boxes[i].msgavail && !strcmp(boxes[i].msg, MSG)) {
                recvd++;
                boxes[i].msgavail = 0; /* reset back to 0 */
                memset(boxes[i].msg, 0, 1024);
            }
        }
        apr_sleep(apr_time_from_sec(1));
    }
    return recvd;
}

int main(void)
{
    apr_status_t rv;
    apr_pool_t *pool;
    apr_shm_t *shm;
    int recvd;

    apr_initialize();
    
    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        exit(-1);
    }

    rv = apr_shm_attach(&shm, SHARED_FILENAME, pool);
    if (rv != APR_SUCCESS) {
        exit(-2);
    }

    boxes = apr_shm_baseaddr_get(shm);

    /* consume messages on all of the boxes */
    recvd = msgwait(30, 0, N_BOXES); /* wait for 30 seconds for messages */

    rv = apr_shm_detach(shm);
    if (rv != APR_SUCCESS) {
        exit(-3);
    }

    return recvd;
}

#else /* APR_HAS_SHARED_MEMORY */

int main(void)
{
    /* Just return, this program will never be called, so we don't need
     * to print a message 
     */
    return 0;
}

#endif /* APR_HAS_SHARED_MEMORY */

