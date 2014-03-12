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
#include "apr_private.h"
#include "apr_arch_file_io.h"
#include "apr_arch_utf8.h"

#if APR_HAS_DSO

APR_DECLARE(apr_status_t) apr_os_dso_handle_put(apr_dso_handle_t **aprdso,
                                                apr_os_dso_handle_t osdso,
                                                apr_pool_t *pool)
{
    *aprdso = apr_pcalloc(pool, sizeof **aprdso);
    (*aprdso)->handle = osdso;
    (*aprdso)->cont = pool;
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

    if (dso->handle != NULL && !FreeLibrary(dso->handle)) {
        return apr_get_os_error();
    }
    dso->handle = NULL;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_dso_load(struct apr_dso_handle_t **res_handle, 
                                       const char *path, apr_pool_t *ctx)
{
    HINSTANCE os_handle;
    apr_status_t rv;
#ifndef _WIN32_WCE
    UINT em;
#endif

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE 
    {
        apr_wchar_t wpath[APR_PATH_MAX];
        if ((rv = utf8_to_unicode_path(wpath, sizeof(wpath) 
                                            / sizeof(apr_wchar_t), path))
                != APR_SUCCESS) {
            *res_handle = apr_pcalloc(ctx, sizeof(**res_handle));
            return ((*res_handle)->load_error = rv);
        }
        /* Prevent ugly popups from killing our app */
#ifndef _WIN32_WCE
        em = SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
        os_handle = LoadLibraryExW(wpath, NULL, 0);
        if (!os_handle)
            os_handle = LoadLibraryExW(wpath, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!os_handle)
            rv = apr_get_os_error();
#ifndef _WIN32_WCE
        SetErrorMode(em);
#endif
    }
#endif /* APR_HAS_UNICODE_FS */
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        char fspec[APR_PATH_MAX], *p = fspec;
        /* Must convert path from / to \ notation.
         * Per PR2555, the LoadLibraryEx function is very picky about slashes.
         * Debugging on NT 4 SP 6a reveals First Chance Exception within NTDLL.
         * LoadLibrary in the MS PSDK also reveals that it -explicitly- states
         * that backslashes must be used for the LoadLibrary family of calls.
         */
        apr_cpystrn(fspec, path, sizeof(fspec));
        while ((p = strchr(p, '/')) != NULL)
            *p = '\\';
        
        /* Prevent ugly popups from killing our app */
        em = SetErrorMode(SEM_FAILCRITICALERRORS);
        os_handle = LoadLibraryEx(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!os_handle)
            os_handle = LoadLibraryEx(path, NULL, 0);
        if (!os_handle)
            rv = apr_get_os_error();
        else
            rv = APR_SUCCESS;
        SetErrorMode(em);
    }
#endif

    *res_handle = apr_pcalloc(ctx, sizeof(**res_handle));
    (*res_handle)->cont = ctx;

    if (rv) {
        return ((*res_handle)->load_error = rv);
    }

    (*res_handle)->handle = (void*)os_handle;
    (*res_handle)->load_error = APR_SUCCESS;

    apr_pool_cleanup_register(ctx, *res_handle, dso_cleanup, apr_pool_cleanup_null);

    return APR_SUCCESS;
}
    
APR_DECLARE(apr_status_t) apr_dso_unload(struct apr_dso_handle_t *handle)
{
    return apr_pool_cleanup_run(handle->cont, handle, dso_cleanup);
}

APR_DECLARE(apr_status_t) apr_dso_sym(apr_dso_handle_sym_t *ressym, 
                         struct apr_dso_handle_t *handle, 
                         const char *symname)
{
#ifdef _WIN32_WCE
    apr_size_t symlen = strlen(symname) + 1;
    apr_size_t wsymlen = 256;
    apr_wchar_t wsymname[256];
    apr_status_t rv;

    rv = apr_conv_utf8_to_ucs2(wsymname, &wsymlen, symname, &symlen);
    if (rv != APR_SUCCESS) {
        return rv;
    }
    else if (symlen) {
        return APR_ENAMETOOLONG;
    }

    *ressym = (apr_dso_handle_sym_t)GetProcAddressW(handle->handle, wsymname);
#else
    *ressym = (apr_dso_handle_sym_t)GetProcAddress(handle->handle, symname);
#endif
    if (!*ressym) {
        return apr_get_os_error();
    }
    return APR_SUCCESS;
}

APR_DECLARE(const char *) apr_dso_error(apr_dso_handle_t *dso, char *buf, apr_size_t bufsize)
{
    return apr_strerror(dso->load_error, buf, bufsize);
}

#endif
