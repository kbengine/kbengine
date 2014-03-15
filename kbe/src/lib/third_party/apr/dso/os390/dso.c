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

#include "apr_portable.h"
#include "apr_strings.h"
#include "apr_arch_dso.h"
#include <errno.h>
#include <string.h>

#if APR_HAS_DSO

APR_DECLARE(apr_status_t) apr_os_dso_handle_put(apr_dso_handle_t **aprdso,
                                                apr_os_dso_handle_t osdso,
                                                apr_pool_t *pool)
{   
    *aprdso = apr_pcalloc(pool, sizeof **aprdso);
    (*aprdso)->handle = osdso;
    (*aprdso)->pool = pool;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_dso_handle_get(apr_os_dso_handle_t *osdso,
                                                apr_dso_handle_t *aprdso)
{
    *osdso = aprdso->handle;
    return APR_SUCCESS;
}

static apr_status_t dso_cleanup(void *thedso)
{
    apr_dso_handle_t *dso = thedso;
    int rc;

    if (dso->handle == 0)
        return APR_SUCCESS;
       
    rc = dllfree(dso->handle);

    if (rc == 0) {
        dso->handle = 0;
        return APR_SUCCESS;
    }
    dso->failing_errno = errno;
    return errno;
}

APR_DECLARE(apr_status_t) apr_dso_load(apr_dso_handle_t **res_handle, 
                                       const char *path, apr_pool_t *ctx)
{
    dllhandle *handle;
    int rc;

    *res_handle = apr_pcalloc(ctx, sizeof(**res_handle));
    (*res_handle)->pool = ctx;
    if ((handle = dllload(path)) != NULL) {
        (*res_handle)->handle  = handle;
        apr_pool_cleanup_register(ctx, *res_handle, dso_cleanup, apr_pool_cleanup_null);
        return APR_SUCCESS;
    }

    (*res_handle)->failing_errno = errno;
    return APR_EDSOOPEN;
}

APR_DECLARE(apr_status_t) apr_dso_unload(apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->pool, handle, dso_cleanup);
}

APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, 
                                      apr_dso_handle_t *handle, 
                                      const char *symname)
{
    void *func_ptr;
    void *var_ptr; 

    if ((var_ptr = dllqueryvar(handle->handle, symname)) != NULL) {
        *ressym = var_ptr;
        return APR_SUCCESS;
    }
    if ((func_ptr = (void *)dllqueryfn(handle->handle, symname)) != NULL) {
        *ressym = func_ptr;
        return APR_SUCCESS;
    }
    handle->failing_errno = errno;
    return APR_ESYMNOTFOUND;
}

APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *handle, char *buffer, 
                          apr_size_t buflen)
{
    apr_cpystrn(buffer, strerror(handle->failing_errno), buflen);
    return buffer;
}

#endif
