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

#include <library.h>
#include <unistd.h>

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
    sym_list *symbol = NULL;
    void *NLMHandle = getnlmhandle();

    if (dso->handle == NULL)
        return APR_SUCCESS;

    if (dso->symbols != NULL) {
        symbol = dso->symbols;
        while (symbol) {
            UnImportPublicObject(NLMHandle, symbol->symbol);
            symbol = symbol->next;
        }
    }

    if (dlclose(dso->handle) != 0)
        return APR_EINIT;

    dso->handle = NULL;
    dso->symbols = NULL;
    dso->path = NULL;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_load(apr_dso_handle_t **res_handle, 
                                       const char *path, apr_pool_t *pool)
{

    void *os_handle = NULL;
    char *fullpath = NULL;
    apr_status_t rv;

    if ((rv = apr_filepath_merge(&fullpath, NULL, path, 
                                 APR_FILEPATH_NATIVE, pool)) != APR_SUCCESS) {
        return rv;
    }

    os_handle = dlopen(fullpath, RTLD_NOW | RTLD_LOCAL);

    *res_handle = apr_pcalloc(pool, sizeof(**res_handle));

    if(os_handle == NULL) {
        (*res_handle)->errormsg = dlerror();
        return APR_EDSOOPEN;
    }

    (*res_handle)->handle = (void*)os_handle;
    (*res_handle)->pool = pool;
    (*res_handle)->errormsg = NULL;
    (*res_handle)->symbols = NULL;
    (*res_handle)->path = apr_pstrdup(pool, fullpath);

    apr_pool_cleanup_register(pool, *res_handle, dso_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
}
    
APR_DECLARE(apr_status_t) apr_dso_unload(apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->pool, handle, dso_cleanup);
}

APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, 
                                      apr_dso_handle_t *handle, 
                                      const char *symname)
{
    sym_list *symbol = NULL;
    void *retval = dlsym(handle->handle, symname);

    if (retval == NULL) {
        handle->errormsg = dlerror();
        return APR_ESYMNOTFOUND;
    }

    symbol = apr_pcalloc(handle->pool, sizeof(sym_list));
    symbol->next = handle->symbols;
    handle->symbols = symbol;
    symbol->symbol = apr_pstrdup(handle->pool, symname);

    *ressym = retval;
    
    return APR_SUCCESS;
}

APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *dso, char *buffer, 
                                        apr_size_t buflen)
{
    if (dso->errormsg) {
        apr_cpystrn(buffer, dso->errormsg, buflen);
        return dso->errormsg;
    }
    return "No Error";
}

