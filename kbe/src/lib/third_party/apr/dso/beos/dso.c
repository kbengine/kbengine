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
#include "apr_portable.h"

#if APR_HAS_DSO

static apr_status_t dso_cleanup(void *thedso)
{
    apr_dso_handle_t *dso = thedso;

    if (dso->handle > 0 && unload_add_on(dso->handle) < B_NO_ERROR)
        return APR_EINIT;
    dso->handle = -1;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_load(apr_dso_handle_t **res_handle, 
                                       const char *path, apr_pool_t *pool)
{
    image_id newid = -1;

    *res_handle = apr_pcalloc(pool, sizeof(*res_handle));

    if((newid = load_add_on(path)) < B_NO_ERROR) {
        (*res_handle)->errormsg = strerror(newid);
        return APR_EDSOOPEN;
	}
	
    (*res_handle)->pool = pool;
    (*res_handle)->handle = newid;

    apr_pool_cleanup_register(pool, *res_handle, dso_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_unload(apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->pool, handle, dso_cleanup);
}

APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, apr_dso_handle_t *handle,
               const char *symname)
{
    int err;

    if (symname == NULL)
        return APR_ESYMNOTFOUND;

    err = get_image_symbol(handle->handle, symname, B_SYMBOL_TYPE_ANY, 
			 ressym);

    if(err != B_OK)
        return APR_ESYMNOTFOUND;

    return APR_SUCCESS;
}

APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *dso, char *buffer, apr_size_t buflen)
{
    strncpy(buffer, strerror(errno), buflen);
    return buffer;
}

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

#endif
