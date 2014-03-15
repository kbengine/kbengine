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

#include "apr.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_signal.h"

#include "apr_arch_misc.h"       /* for WSAHighByte / WSALowByte */
#include "apr_arch_proc_mutex.h" /* for apr_proc_mutex_unix_setup_lock() */
#include "apr_arch_internal_time.h"
#include "apr_ldap.h"            /* for apr_ldap_rebind_init() */

#ifdef USE_WINSOCK
/* Prototypes missing from older NDKs */
int WSAStartupRTags(WORD wVersionRequested,
                    LPWSADATA lpWSAData,
                    rtag_t WSAStartupRTag,
                    rtag_t WSPSKTRTag,
                    rtag_t lookUpServiceBeginRTag,
                    rtag_t WSAEventRTag,
                    rtag_t WSPCPRTag);

int WSACleanupRTag(rtag_t rTag);

/*
** Resource tag signatures for using NetWare WinSock 2. These will no longer
** be needed by anyone once the new WSAStartupWithNlmHandle() is available
** since WinSock will make the calls to AllocateResourceTag().
*/
#define WS_LOAD_ENTRY_SIGNATURE     (*(unsigned long *) "WLDE")
#define WS_SKT_SIGNATURE            (*(unsigned long *) "WSKT")
#define WS_LOOKUP_SERVICE_SIGNATURE (*(unsigned long *) "WLUP")
#define WS_WSAEVENT_SIGNATURE       (*(unsigned long *) "WEVT")
#define WS_CPORT_SIGNATURE          (*(unsigned long *) "WCPT")


int (*WSAStartupWithNLMHandle)( WORD version, LPWSADATA data, void *handle ) = NULL;
int (*WSACleanupWithNLMHandle)( void *handle ) = NULL;

static int wsa_startup_with_handle (WORD wVersionRequested, LPWSADATA data, void *handle)
{
    APP_DATA *app_data;
    
    if (!(app_data = (APP_DATA*) get_app_data(gLibId)))
        return APR_EGENERAL;

    app_data->gs_startup_rtag = AllocateResourceTag(handle, "WinSock Start-up", WS_LOAD_ENTRY_SIGNATURE);
    app_data->gs_socket_rtag  = AllocateResourceTag(handle, "WinSock socket()", WS_SKT_SIGNATURE);
    app_data->gs_lookup_rtag  = AllocateResourceTag(handle, "WinSock Look-up", WS_LOOKUP_SERVICE_SIGNATURE);
    app_data->gs_event_rtag   = AllocateResourceTag(handle, "WinSock Event", WS_WSAEVENT_SIGNATURE);
    app_data->gs_pcp_rtag     = AllocateResourceTag(handle, "WinSock C-Port", WS_CPORT_SIGNATURE);

    return WSAStartupRTags(wVersionRequested, data, 
                           app_data->gs_startup_rtag, 
                           app_data->gs_socket_rtag, 
                           app_data->gs_lookup_rtag, 
                           app_data->gs_event_rtag, 
                           app_data->gs_pcp_rtag);
}

static int wsa_cleanup_with_handle (void *handle)
{
    APP_DATA *app_data;
    
    if (!(app_data = (APP_DATA*) get_app_data(gLibId)))
        return APR_EGENERAL;

    return WSACleanupRTag(app_data->gs_startup_rtag);
}

static int UnregisterAppWithWinSock (void *nlm_handle)
{
    if (!WSACleanupWithNLMHandle)
    {
        if (!(WSACleanupWithNLMHandle = ImportPublicObject(gLibHandle, "WSACleanupWithNLMHandle")))
            WSACleanupWithNLMHandle = wsa_cleanup_with_handle;
    }

    return (*WSACleanupWithNLMHandle)(nlm_handle);
}

static int RegisterAppWithWinSock (void *nlm_handle)
{
    int err;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(WSAHighByte, WSALowByte);

    if (!WSAStartupWithNLMHandle)
    {
        if (!(WSAStartupWithNLMHandle = ImportPublicObject(gLibHandle, "WSAStartupWithNLMHandle")))
            WSAStartupWithNLMHandle = wsa_startup_with_handle;
    }

    err = (*WSAStartupWithNLMHandle)(wVersionRequested, &wsaData, nlm_handle);

    if (LOBYTE(wsaData.wVersion) != WSAHighByte ||
        HIBYTE(wsaData.wVersion) != WSALowByte) {
        
        UnregisterAppWithWinSock (nlm_handle);
        return APR_EEXIST;
    }

    return err;
}
#endif



APR_DECLARE(apr_status_t) apr_app_initialize(int *argc, 
                                             const char * const * *argv, 
                                             const char * const * *env)
{
    /* An absolute noop.  At present, only Win32 requires this stub, but it's
     * required in order to move command arguments passed through the service
     * control manager into the process, and it's required to fix the char*
     * data passed in from win32 unicode into utf-8, win32's apr internal fmt.
     */
    return apr_initialize();
}

APR_DECLARE(apr_status_t) apr_initialize(void)
{
    apr_pool_t *pool;
    void *nlmhandle = getnlmhandle();

    /* Register the NLM as using APR. If it is already
        registered then just return. */
    if (register_NLM(nlmhandle) != 0) {
        return APR_SUCCESS;
    }

    /* apr_pool_initialize() is being called from the library
        startup code since all of the memory resources belong 
        to the library rather than the application. */
    
    if (apr_pool_create(&pool, NULL) != APR_SUCCESS) {
        return APR_ENOPOOL;
    }

    apr_pool_tag(pool, "apr_initilialize");

#ifdef USE_WINSOCK
    {
        int err;
        if ((err = RegisterAppWithWinSock (nlmhandle))) {
            return err;
        }
    }
#endif

    apr_signal_init(pool);
#if APR_HAS_LDAP
    apr_ldap_rebind_init(pool);
#endif

    return APR_SUCCESS;
}

APR_DECLARE_NONSTD(void) apr_terminate(void)
{
    APP_DATA *app_data;

    /* Get our instance data for shutting down. */
    if (!(app_data = (APP_DATA*) get_app_data(gLibId)))
        return;

    /* Unregister the NLM. If it is not registered
        then just return. */
    if (unregister_NLM(app_data->gs_nlmhandle) != 0) {
        return;
    }

    /* apr_pool_terminate() is being called from the 
        library shutdown code since the memory resources
        belong to the library rather than the application */

    /* Just clean up the memory for the app that is going
        away. */
    netware_pool_proc_cleanup ();

#ifdef USE_WINSOCK
    UnregisterAppWithWinSock (app_data->gs_nlmhandle);
#endif
}

APR_DECLARE(void) apr_terminate2(void)
{
    apr_terminate();
}
