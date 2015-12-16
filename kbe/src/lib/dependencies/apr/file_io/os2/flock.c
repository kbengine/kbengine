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

#include "apr_arch_file_io.h"

APR_DECLARE(apr_status_t) apr_file_lock(apr_file_t *thefile, int type)
{
    FILELOCK lockrange = { 0, 0x7fffffff };
    ULONG rc;

    rc = DosSetFileLocks(thefile->filedes, NULL, &lockrange,
                         (type & APR_FLOCK_NONBLOCK) ? 0 : (ULONG)-1,
                         (type & APR_FLOCK_TYPEMASK) == APR_FLOCK_SHARED);
    return APR_FROM_OS_ERROR(rc);
}

APR_DECLARE(apr_status_t) apr_file_unlock(apr_file_t *thefile)
{
    FILELOCK unlockrange = { 0, 0x7fffffff };
    ULONG rc;

    rc = DosSetFileLocks(thefile->filedes, &unlockrange, NULL, 0, 0);
    return APR_FROM_OS_ERROR(rc);
}
