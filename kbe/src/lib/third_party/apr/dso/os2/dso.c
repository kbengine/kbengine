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

#include "apr_arch_dso.h"
#include "apr_strings.h"
#include "apr_portable.h"
#include <stdio.h>
#include <string.h>

#if APR_HAS_DSO

static apr_status_t dso_cleanup(void *thedso)
{
    apr_dso_handle_t *dso = thedso;
    int rc;

    if (dso->handle == 0)
        return APR_SUCCESS;
       
    rc = DosFreeModule(dso->handle);

    if (rc == 0)
        dso->handle = 0;

    return APR_FROM_OS_ERROR(rc);
}


APR_DECLARE(apr_status_t) apr_dso_load(apr_dso_handle_t **res_handle, const char *path, apr_pool_t *ctx)
{
    char failed_module[200];
    HMODULE handle;
    int rc;

    *res_handle = apr_pcalloc(ctx, sizeof(**res_handle));
    (*res_handle)->cont = ctx;
    (*res_handle)->load_error = APR_SUCCESS;
    (*res_handle)->failed_module = NULL;

    if ((rc = DosLoadModule(failed_module, sizeof(failed_module), path, &handle)) != 0) {
        (*res_handle)->load_error = APR_FROM_OS_ERROR(rc);
        (*res_handle)->failed_module = apr_pstrdup(ctx, failed_module);
        return APR_FROM_OS_ERROR(rc);
    }

    (*res_handle)->handle  = handle;
    apr_pool_cleanup_register(ctx, *res_handle, dso_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_dso_unload(apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->cont, handle, dso_cleanup);
}



APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, 
                                      apr_dso_handle_t *handle, 
                                      const char *symname)
{
    PFN func;
    int rc;

    if (symname == NULL || ressym == NULL)
        return APR_ESYMNOTFOUND;

    if ((rc = DosQueryProcAddr(handle->handle, 0, symname, &func)) != 0) {
        handle->load_error = APR_FROM_OS_ERROR(rc);
        return handle->load_error;
    }

    *ressym = func;
    return APR_SUCCESS;
}



APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *dso, char *buffer, apr_size_t buflen)
{
    char message[200];
    apr_strerror(dso->load_error, message, sizeof(message));

    if (dso->failed_module != NULL) {
        strcat(message, " (");
        strcat(message, dso->failed_module);
        strcat(message, ")");
    }

    apr_cpystrn(buffer, message, buflen);
    return buffer;
}



APR_DECLARE(apr_status_t) apr_os_dso_handle_put(apr_dso_handle_t **aprdso,
                                                apr_os_dso_handle_t osdso,
                                                apr_pool_t *pool)
{
    *aprdso = apr_pcalloc(pool, sizeof **aprdso);
    (*aprdso)->handle = osdso;
    (*aprdso)->cont = pool;
    (*aprdso)->load_error = APR_SUCCESS;
    (*aprdso)->failed_module = NULL;
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_os_dso_handle_get(apr_os_dso_handle_t *osdso,
                                                apr_dso_handle_t *aprdso)
{
    *osdso = aprdso->handle;
    return APR_SUCCESS;
}

#endif
