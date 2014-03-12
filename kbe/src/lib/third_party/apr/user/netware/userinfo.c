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

#include "apr_strings.h"
#include "apr_portable.h"
#include "apr_user.h"
#include "apr_private.h"
#ifdef HAVE_PWD_H
#include <pwd.h>
#endif
#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if APR_HAVE_UNISTD_H
#include <unistd.h> /* for _POSIX_THREAD_SAFE_FUNCTIONS */
#endif

#define PWBUF_SIZE 512

static apr_status_t getpwnam_safe(const char *username,
                                  struct passwd *pw,
                                  char pwbuf[PWBUF_SIZE])
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_uid_homepath_get(char **dirname,
                                               const char *username,
                                               apr_pool_t *p)
{
    return APR_ENOTIMPL;
}



APR_DECLARE(apr_status_t) apr_uid_current(apr_uid_t *uid,
                                          apr_gid_t *gid,
                                          apr_pool_t *p)
{
    return APR_ENOTIMPL;
}




APR_DECLARE(apr_status_t) apr_uid_get(apr_uid_t *uid, apr_gid_t *gid,
                                      const char *username, apr_pool_t *p)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_uid_name_get(char **username, apr_uid_t userid,
                                           apr_pool_t *p)
{
    return APR_ENOTIMPL;
}

