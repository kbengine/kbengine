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

#include "apr_private.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_signal.h"
#include "shellapi.h"

#include "apr_arch_misc.h"       /* for WSAHighByte / WSALowByte */
#include "wchar.h"
#include "apr_arch_file_io.h"    /* bring in unicode-ness */
#include "apr_arch_threadproc.h" /* bring in apr_threadproc_init */
#include "assert.h"

/* This symbol is _private_, although it must be exported.
 */
int APR_DECLARE_DATA apr_app_init_complete = 0;

#if !defined(_WIN32_WCE)
/* Used by apr_app_initialize to reprocess the environment
 *
 * An internal apr function to convert a double-null terminated set
 * of single-null terminated strings from wide Unicode to narrow utf-8
 * as a list of strings.  These are allocated from the MSVCRT's
 * _CRT_BLOCK to trick the system into trusting our store.
 */
static int warrsztoastr(const char * const * *retarr,
                        const wchar_t * arrsz, int args)
{
    const apr_wchar_t *wch;
    apr_size_t totlen;
    apr_size_t newlen;
    apr_size_t wsize;
    char **env;
    char *pstrs;
    char *strs;
    int arg;

    if (args < 0) {
        for (args = 1, wch = arrsz; wch[0] || wch[1]; ++wch)
            if (!*wch)
                ++args;
    }
    wsize = 1 + wch - arrsz;

    /* This is a safe max allocation, we will alloc each
     * string exactly after processing and return this
     * temporary buffer to the free store.
     * 3 ucs bytes hold any single wchar_t value (16 bits)
     * 4 ucs bytes will hold a wchar_t pair value (20 bits)
     */
    newlen = totlen = wsize * 3 + 1;
    pstrs = strs = apr_malloc_dbg(newlen * sizeof(char),
                                  __FILE__, __LINE__);

    (void)apr_conv_ucs2_to_utf8(arrsz, &wsize, strs, &newlen);

    assert(newlen && !wsize);

    *retarr = env = apr_malloc_dbg((args + 1) * sizeof(char*),
                                   __FILE__, __LINE__);
    for (arg = 0; arg < args; ++arg) {
        char* p = pstrs;
        int len = 0;
        while (*p++)
            ++len;
        len += 1;

        *env = apr_malloc_dbg(len * sizeof(char),
                              __FILE__, __LINE__);
        memcpy(*env, pstrs, len * sizeof(char));

        pstrs += len;
        ++env;
    }

    *env = NULL;
    free(strs);

    return args;
}
#endif

/* Reprocess the arguments to main() for a completely apr-ized application
 */

APR_DECLARE(apr_status_t) apr_app_initialize(int *argc,
                                             const char * const * *argv,
                                             const char * const * *env)
{
    apr_status_t rv = apr_initialize();

    if (rv != APR_SUCCESS) {
        return rv;
    }

#if defined(_WIN32_WCE)
    apr_app_init_complete = 1;
#elif APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t **wstrs;
        apr_wchar_t *sysstr;
        int wstrc;
        int dupenv;

        if (apr_app_init_complete) {
            return rv;
        }

        apr_app_init_complete = 1;

        sysstr = GetCommandLineW();
        if (sysstr) {
            wstrs = CommandLineToArgvW(sysstr, &wstrc);
            if (wstrs) {
                *argc = apr_wastrtoastr(argv, wstrs, wstrc);
                GlobalFree(wstrs);
            }
        }

        sysstr = GetEnvironmentStringsW();
        dupenv = warrsztoastr(&_environ, sysstr, -1);

        if (env) {
            *env = apr_malloc_dbg((dupenv + 1) * sizeof (char *),
                                  __FILE__, __LINE__ );
            memcpy((void*)*env, _environ, (dupenv + 1) * sizeof (char *));
        }
        else {
        }

        FreeEnvironmentStringsW(sysstr);

        /* MSVCRT will attempt to maintain the wide environment calls
         * on _putenv(), which is bogus if we've passed a non-ascii
         * string to _putenv(), since they use MultiByteToWideChar
         * and breaking the implicit utf-8 assumption we've built.
         *
         * Reset _wenviron for good measure.
         */
        if (_wenviron) {
            apr_wchar_t **wenv = _wenviron;
            _wenviron = NULL;
            free(wenv);
        }

    }
#endif
    return rv;
}

static int initialized = 0;

/* Provide to win32/thread.c */
extern DWORD tls_apr_thread;

APR_DECLARE(apr_status_t) apr_initialize(void)
{
    apr_pool_t *pool;
    apr_status_t status;
    int iVersionRequested;
    WSADATA wsaData;
    int err;
    apr_oslevel_e osver;

    if (initialized++) {
        return APR_SUCCESS;
    }

    /* Initialize apr_os_level global */
    if (apr_get_oslevel(&osver) != APR_SUCCESS) {
        return APR_EEXIST;
    }

    tls_apr_thread = TlsAlloc();
    if ((status = apr_pool_initialize()) != APR_SUCCESS)
        return status;

    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        return APR_ENOPOOL;
    }

    apr_pool_tag(pool, "apr_initialize");

    iVersionRequested = MAKEWORD(WSAHighByte, WSALowByte);
    err = WSAStartup((WORD) iVersionRequested, &wsaData);
    if (err) {
        return err;
    }
    if (LOBYTE(wsaData.wVersion) != WSAHighByte ||
        HIBYTE(wsaData.wVersion) != WSALowByte) {
        WSACleanup();
        return APR_EEXIST;
    }

    apr_signal_init(pool);

    apr_threadproc_init(pool);

    return APR_SUCCESS;
}

APR_DECLARE_NONSTD(void) apr_terminate(void)
{
    initialized--;
    if (initialized) {
        return;
    }
    apr_pool_terminate();

    WSACleanup();

    TlsFree(tls_apr_thread);
}

APR_DECLARE(void) apr_terminate2(void)
{
    apr_terminate();
}
