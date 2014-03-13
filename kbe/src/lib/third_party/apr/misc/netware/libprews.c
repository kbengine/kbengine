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
#include <netware.h>
#include <library.h>
#include <nks/synch.h>

#include "apr_pools.h"
#include "apr_private.h"
#include "apr_arch_internal_time.h"


/* library-private data...*/
int          gLibId = -1;
void         *gLibHandle = (void *) NULL;
NXMutex_t    *gLibLock = (NXMutex_t *) NULL;

/* internal library function prototypes...*/
int DisposeLibraryData(void *);

int _NonAppStart
(
    void        *NLMHandle,
    void        *errorScreen,
    const char  *cmdLine,
    const char  *loadDirPath,
    size_t      uninitializedDataLength,
    void        *NLMFileHandle,
    int         (*readRoutineP)( int conn, void *fileHandle, size_t offset,
                    size_t nbytes, size_t *bytesRead, void *buffer ),
    size_t      customDataOffset,
    size_t      customDataSize,
    int         messageCount,
    const char  **messages
)
{
#ifdef USE_WINSOCK
    WSADATA wsaData;
#endif
    apr_status_t status;
    
    NX_LOCK_INFO_ALLOC(liblock, "Per-Application Data Lock", 0);

#pragma unused(cmdLine)
#pragma unused(loadDirPath)
#pragma unused(uninitializedDataLength)
#pragma unused(NLMFileHandle)
#pragma unused(readRoutineP)
#pragma unused(customDataOffset)
#pragma unused(customDataSize)
#pragma unused(messageCount)
#pragma unused(messages)

    gLibId = register_library(DisposeLibraryData);

    if (gLibId < -1)
    {
        OutputToScreen(errorScreen, "Unable to register library with kernel.\n");
        return -1;
    }

    gLibHandle = NLMHandle;

    gLibLock = NXMutexAlloc(0, 0, &liblock);

    if (!gLibLock)
    {
        OutputToScreen(errorScreen, "Unable to allocate library data lock.\n");
        return -1;
    }

    apr_netware_setup_time();

    if ((status = apr_pool_initialize()) != APR_SUCCESS)
        return status;

#ifdef USE_WINSOCK
    return WSAStartup((WORD) MAKEWORD(2, 0), &wsaData);
#else
    return 0;
#endif
}

void _NonAppStop( void )
{
    apr_pool_terminate();

#ifdef USE_WINSOCK
    WSACleanup();
#endif

    unregister_library(gLibId);
    NXMutexFree(gLibLock);
}

int  _NonAppCheckUnload( void )
{
    return 0;
}

int register_NLM(void *NLMHandle)
{
    APP_DATA *app_data = (APP_DATA*) get_app_data(gLibId);

    NXLock(gLibLock);
    if (!app_data) {
        app_data = (APP_DATA*)library_malloc(gLibHandle, sizeof(APP_DATA));

        if (app_data) {
            memset (app_data, 0, sizeof(APP_DATA));
            set_app_data(gLibId, app_data);
            app_data->gs_nlmhandle = NLMHandle;
        }
    }

    if (app_data && (!app_data->initialized)) {
        app_data->initialized = 1;
        NXUnlock(gLibLock);
        return 0;
    }

    NXUnlock(gLibLock);
    return 1;
}

int unregister_NLM(void *NLMHandle)
{
    APP_DATA *app_data = (APP_DATA*) get_app_data(gLibId);

    NXLock(gLibLock);
    if (app_data) {
        app_data->initialized = 0;
        NXUnlock(gLibLock);
        return 0;
    }
    NXUnlock(gLibLock);
    return 1;
}

int DisposeLibraryData(void *data)
{
    if (data)
    {
        library_free(data);
    }

    return 0;
}

int setGlobalPool(void *data)
{
    APP_DATA *app_data = (APP_DATA*) get_app_data(gLibId);

    NXLock(gLibLock);

    if (app_data && !app_data->gPool) {
        app_data->gPool = data;
    }

    NXUnlock(gLibLock);
    return 1;
}

void* getGlobalPool()
{
    APP_DATA *app_data = (APP_DATA*) get_app_data(gLibId);

    if (app_data) {
        return app_data->gPool;
    }

    return NULL;
}

