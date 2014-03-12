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
#include "apr_file_io.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include <string.h>
#include "apr_arch_inherit.h"

static apr_status_t file_dup(apr_file_t **new_file, apr_file_t *old_file, apr_pool_t *p)
{
    int rv;
    apr_file_t *dup_file;

    if (*new_file == NULL) {
        dup_file = (apr_file_t *)apr_palloc(p, sizeof(apr_file_t));

        if (dup_file == NULL) {
            return APR_ENOMEM;
        }

        dup_file->filedes = -1;
    } else {
      dup_file = *new_file;
    }

    dup_file->pool = p;
    rv = DosDupHandle(old_file->filedes, &dup_file->filedes);

    if (rv) {
        return APR_FROM_OS_ERROR(rv);
    }

    dup_file->fname = apr_pstrdup(dup_file->pool, old_file->fname);
    dup_file->buffered = old_file->buffered;
    dup_file->isopen = old_file->isopen;
    dup_file->flags = old_file->flags & ~APR_INHERIT;
    /* TODO - dup pipes correctly */
    dup_file->pipe = old_file->pipe;

    if (*new_file == NULL) {
        apr_pool_cleanup_register(dup_file->pool, dup_file, apr_file_cleanup,
                            apr_pool_cleanup_null);
        *new_file = dup_file;
    }

    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_file_dup(apr_file_t **new_file, apr_file_t *old_file, apr_pool_t *p)
{
  if (*new_file) {
      apr_file_close(*new_file);
      (*new_file)->filedes = -1;
  }

  return file_dup(new_file, old_file, p);
}



APR_DECLARE(apr_status_t) apr_file_dup2(apr_file_t *new_file, apr_file_t *old_file, apr_pool_t *p)
{
  return file_dup(&new_file, old_file, p);
}



APR_DECLARE(apr_status_t) apr_file_setaside(apr_file_t **new_file,
                                            apr_file_t *old_file,
                                            apr_pool_t *p)
{
    *new_file = (apr_file_t *)apr_pmemdup(p, old_file, sizeof(apr_file_t));
    (*new_file)->pool = p;

    if (old_file->buffered) {
        (*new_file)->buffer = apr_palloc(p, old_file->bufsize);
        (*new_file)->bufsize = old_file->bufsize;

        if (old_file->direction == 1) {
            memcpy((*new_file)->buffer, old_file->buffer, old_file->bufpos);
        }
        else {
            memcpy((*new_file)->buffer, old_file->buffer, old_file->dataRead);
        }

        if (old_file->mutex) {
            apr_thread_mutex_create(&((*new_file)->mutex),
                                    APR_THREAD_MUTEX_DEFAULT, p);
            apr_thread_mutex_destroy(old_file->mutex);
        }
    }

    if (old_file->fname) {
        (*new_file)->fname = apr_pstrdup(p, old_file->fname);
    }

    if (!(old_file->flags & APR_FOPEN_NOCLEANUP)) {
        apr_pool_cleanup_register(p, (void *)(*new_file), 
                                  apr_file_cleanup,
                                  apr_file_cleanup);
    }

    old_file->filedes = -1;
    apr_pool_cleanup_kill(old_file->pool, (void *)old_file,
                          apr_file_cleanup);

    return APR_SUCCESS;
}
