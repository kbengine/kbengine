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
#include "apr_lib.h"
#include "apr_strings.h"
#include <string.h>

#define IS_SEP(c) (c == '/' || c == '\\')

/* Remove trailing separators that don't affect the meaning of PATH. */
static const char *path_canonicalize(const char *path, apr_pool_t *pool)
{
    /* At some point this could eliminate redundant components.  For
     * now, it just makes sure there is no trailing slash. */
    apr_size_t len = strlen(path);
    apr_size_t orig_len = len;

    while ((len > 0) && IS_SEP(path[len - 1])) {
        len--;
    }

    if (len != orig_len) {
        return apr_pstrndup(pool, path, len);
    }
    else {
        return path;
    }
}



/* Remove one component off the end of PATH. */
static char *path_remove_last_component(const char *path, apr_pool_t *pool)
{
    const char *newpath = path_canonicalize(path, pool);
    int i;

    for (i = strlen(newpath) - 1; i >= 0; i--) {
        if (IS_SEP(path[i])) {
            break;
        }
    }

    return apr_pstrndup(pool, path, (i < 0) ? 0 : i);
}



apr_status_t apr_dir_make_recursive(const char *path, apr_fileperms_t perm,
                                    apr_pool_t *pool)
{
    apr_status_t apr_err = APR_SUCCESS;
    
    apr_err = apr_dir_make(path, perm, pool); /* Try to make PATH right out */

    if (APR_STATUS_IS_ENOENT(apr_err)) { /* Missing an intermediate dir */
        char *dir;

        dir = path_remove_last_component(path, pool);
        apr_err = apr_dir_make_recursive(dir, perm, pool);

        if (!apr_err) {
            apr_err = apr_dir_make(path, perm, pool);
        }
    }

    /*
     * It's OK if PATH exists. Timing issues can lead to the second
     * apr_dir_make being called on existing dir, therefore this check
     * has to come last.
     */
    if (APR_STATUS_IS_EEXIST(apr_err))
        return APR_SUCCESS;

    return apr_err;
}
