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

#include "testflock.h"
#include "apr_pools.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr.h"

#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif

int main(int argc, const char * const *argv)
{
    apr_file_t *file;
    apr_status_t status;
    apr_pool_t *p;

    apr_initialize();
    apr_pool_create(&p, NULL);

    if (apr_file_open(&file, TESTFILE, APR_FOPEN_WRITE, APR_OS_DEFAULT, p)
        != APR_SUCCESS) {
        
        exit(UNEXPECTED_ERROR);
    }
    status = apr_file_lock(file, APR_FLOCK_EXCLUSIVE | APR_FLOCK_NONBLOCK);
    if (status == APR_SUCCESS) {
        exit(SUCCESSFUL_READ);
    }
    if (APR_STATUS_IS_EAGAIN(status)) {
        exit(FAILED_READ);
    }
    exit(UNEXPECTED_ERROR);
}
