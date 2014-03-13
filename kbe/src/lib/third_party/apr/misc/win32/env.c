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

#define APR_WANT_STRFUNC
#include "apr_want.h"
#include "apr.h"
#include "apr_arch_misc.h"
#include "apr_arch_utf8.h"
#include "apr_env.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_strings.h"

#if APR_HAS_UNICODE_FS && !defined(_WIN32_WCE)
static apr_status_t widen_envvar_name (apr_wchar_t *buffer,
                                       apr_size_t bufflen,
                                       const char *envvar)
{
    apr_size_t inchars;
    apr_status_t status;

    inchars = strlen(envvar) + 1;
    status = apr_conv_utf8_to_ucs2(envvar, &inchars, buffer, &bufflen);
    if (status == APR_INCOMPLETE)
        status = APR_ENAMETOOLONG;

    return status;
}
#endif


APR_DECLARE(apr_status_t) apr_env_get(char **value,
                                      const char *envvar,
                                      apr_pool_t *pool)
{
#if defined(_WIN32_WCE)
    return APR_ENOTIMPL;
#else
    char *val = NULL;
    DWORD size;

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wenvvar[APR_PATH_MAX];
        apr_size_t inchars, outchars;
        apr_wchar_t *wvalue, dummy;
        apr_status_t status;

        status = widen_envvar_name(wenvvar, APR_PATH_MAX, envvar);
        if (status)
            return status;

        SetLastError(0);
        size = GetEnvironmentVariableW(wenvvar, &dummy, 0);
        if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
            /* The environment variable doesn't exist. */
            return APR_ENOENT;

        if (size == 0) {
            /* The environment value exists, but is zero-length. */
            *value = apr_pstrdup(pool, "");
            return APR_SUCCESS;
        }

        wvalue = apr_palloc(pool, size * sizeof(*wvalue));
        size = GetEnvironmentVariableW(wenvvar, wvalue, size);

        inchars = wcslen(wvalue) + 1;
        outchars = 3 * inchars; /* Enough for any UTF-8 representation */
        val = apr_palloc(pool, outchars);
        status = apr_conv_ucs2_to_utf8(wvalue, &inchars, val, &outchars);
        if (status)
            return status;
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        char dummy;

        SetLastError(0);
        size = GetEnvironmentVariableA(envvar, &dummy, 0);
        if (GetLastError() == ERROR_ENVVAR_NOT_FOUND)
            /* The environment variable doesn't exist. */
            return APR_ENOENT;

        if (size == 0) {
            /* The environment value exists, but is zero-length. */
            *value = apr_pstrdup(pool, "");
            return APR_SUCCESS;
        }

        val = apr_palloc(pool, size);
        size = GetEnvironmentVariableA(envvar, val, size);
        if (size == 0)
            /* Mid-air collision?. Somebody must've changed the env. var. */
            return APR_INCOMPLETE;
    }
#endif

    *value = val;
    return APR_SUCCESS;
#endif
}


APR_DECLARE(apr_status_t) apr_env_set(const char *envvar,
                                      const char *value,
                                      apr_pool_t *pool)
{
#if defined(_WIN32_WCE)
    return APR_ENOTIMPL;
#else
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wenvvar[APR_PATH_MAX];
        apr_wchar_t *wvalue;
        apr_size_t inchars, outchars;
        apr_status_t status;

        status = widen_envvar_name(wenvvar, APR_PATH_MAX, envvar);
        if (status)
            return status;

        outchars = inchars = strlen(value) + 1;
        wvalue = apr_palloc(pool, outchars * sizeof(*wvalue));
        status = apr_conv_utf8_to_ucs2(value, &inchars, wvalue, &outchars);
        if (status)
            return status;

        if (!SetEnvironmentVariableW(wenvvar, wvalue))
            return apr_get_os_error();
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        if (!SetEnvironmentVariableA(envvar, value))
            return apr_get_os_error();
    }
#endif

    return APR_SUCCESS;
#endif
}


APR_DECLARE(apr_status_t) apr_env_delete(const char *envvar, apr_pool_t *pool)
{
#if defined(_WIN32_WCE)
    return APR_ENOTIMPL;
#else
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wenvvar[APR_PATH_MAX];
        apr_status_t status;

        status = widen_envvar_name(wenvvar, APR_PATH_MAX, envvar);
        if (status)
            return status;

        if (!SetEnvironmentVariableW(wenvvar, NULL))
            return apr_get_os_error();
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        if (!SetEnvironmentVariableA(envvar, NULL))
            return apr_get_os_error();
    }
#endif

    return APR_SUCCESS;
#endif
}
