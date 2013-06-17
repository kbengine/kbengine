/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VMCONTROL_WRAPPER_H
#define VMCONTROL_WRAPPER_H

#if defined(WIN32) || defined(__linux__)

#define VMCONTROL_WRAPPER_SUPPORTED

/* types defined by vmcontrol.h */
typedef struct VMControlServer VMControlServer;

typedef struct VMControlVM VMControlVM;

typedef char Bool;

typedef struct {
    const char *hostname;
    int port;
    const char *username;
    char *password;
} VMControlConnectParams;

/* function pointers to api defined by vmcontrol.h */
typedef struct {
    void *handle;
    void (*xVMControl_ConnectParamsDestroy)(VMControlConnectParams *);
    VMControlConnectParams * (*xVMControl_ConnectParamsNew)(const char *, int, const char *, const char *);
    Bool (*xVMControl_Init)(void);
    Bool (*xVMControl_MKSSaveScreenshot)(VMControlVM *, const char *, const char *);
    Bool (*xVMControl_ServerConnectEx)(VMControlServer *, VMControlConnectParams *);
    void (*xVMControl_ServerDestroy)(VMControlServer *);
    void (*xVMControl_ServerDisconnect)(VMControlServer *);
    char ** (*xVMControl_ServerEnumerate)(VMControlServer *);
    char * (*xVMControl_ServerExec)(VMControlServer *, const char *);
    int (*xVMControl_ServerGetLastError)(VMControlServer *, char **);
    char * (*xVMControl_ServerGetResource)(VMControlServer *, char *);
    Bool (*xVMControl_ServerIsConnected)(VMControlServer *);
    Bool (*xVMControl_ServerIsRegistered)(VMControlServer *, const char *, Bool *);
    VMControlServer * (*xVMControl_ServerNewEx)(void);
    Bool (*xVMControl_VMConnectEx)(VMControlVM *, VMControlConnectParams *, const char *, Bool);
    Bool (*xVMControl_VMCreateSnapshot)(VMControlVM *, const char *, const char *, Bool, Bool);
    void (*xVMControl_VMDestroy)(VMControlVM *);
    Bool (*xVMControl_VMDeviceConnect)(VMControlVM *, const char *);
    Bool (*xVMControl_VMDeviceDisconnect)(VMControlVM *, const char *);
    Bool (*xVMControl_VMDeviceIsConnected)(VMControlVM *, const char *, Bool *);
    void (*xVMControl_VMDisconnect)(VMControlVM *);
    Bool (*xVMControl_VMGetCapabilities)(VMControlVM *, unsigned int *);
    char * (*xVMControl_VMGetConfig)(VMControlVM *, char *);
    char * (*xVMControl_VMGetConfigFileName)(VMControlVM *);
    Bool (*xVMControl_VMGetExecutionState)(VMControlVM *, int *);
    char * (*xVMControl_VMGetGuestInfo)(VMControlVM *, char *);
    Bool (*xVMControl_VMGetHeartbeat)(VMControlVM *, unsigned int *);
    Bool (*xVMControl_VMGetId)(VMControlVM *, unsigned int *);
    int (*xVMControl_VMGetLastError)(VMControlVM *, char **);
    Bool (*xVMControl_VMGetPid)(VMControlVM *, unsigned int * );
    Bool (*xVMControl_VMGetProductInfo)(VMControlVM *, int, int *);
    Bool (*xVMControl_VMGetRemoteConnections)(VMControlVM *, unsigned int *);
    char * (*xVMControl_VMGetResource)(VMControlVM *, char *);
    Bool (*xVMControl_VMGetRunAsUser)(VMControlVM *, char **);
    Bool (*xVMControl_VMGetUptime)(VMControlVM *, unsigned int *);
    Bool (*xVMControl_VMHasSnapshot)(VMControlVM *, Bool *);
    char (*xVMControl_VMInit)(void);
    Bool (*xVMControl_VMIsConnected)(VMControlVM *);
    VMControlVM * (*xVMControl_VMNewEx)(void);
    Bool (*xVMControl_VMRemoveAllSnapshots)(VMControlVM *);
    Bool (*xVMControl_VMRevertToSnapshot)(VMControlVM *);
    Bool (*xVMControl_VMSetConfig)(VMControlVM *, char *, char *);
    Bool (*xVMControl_VMSetGuestInfo)(VMControlVM *, char *, char *);
    Bool (*xVMControl_VMStart)(VMControlVM *, int);
    Bool (*xVMControl_VMStopOrReset)(VMControlVM *, Bool, int);
    Bool (*xVMControl_VMSuspendToDisk)(VMControlVM *, int);
    Bool (*xVMControl_VMToolsLastActive)(VMControlVM *, int *);
} vmcontrol_wrapper_api_t;

int vmcontrol_wrapper_api_init(const char *lib);

int vmcontrol_wrapper_api_shutdown(void);

vmcontrol_wrapper_api_t *vmcontrol_wrapper_api_get(void);

#define VMControl_ConnectParamsDestroy \
  vmcontrol_wrapper_api_get()->xVMControl_ConnectParamsDestroy

#define VMControl_ConnectParamsNew \
  vmcontrol_wrapper_api_get()->xVMControl_ConnectParamsNew

#define VMControl_Init \
  vmcontrol_wrapper_api_get()->xVMControl_Init

#define VMControl_MKSSaveScreenshot \
  vmcontrol_wrapper_api_get()->xVMControl_MKSSaveScreenshot

#define VMControl_ServerConnectEx \
  vmcontrol_wrapper_api_get()->xVMControl_ServerConnectEx

#define VMControl_ServerDestroy \
  vmcontrol_wrapper_api_get()->xVMControl_ServerDestroy

#define VMControl_ServerDisconnect \
  vmcontrol_wrapper_api_get()->xVMControl_ServerDisconnect

#define VMControl_ServerEnumerate \
  vmcontrol_wrapper_api_get()->xVMControl_ServerEnumerate

#define VMControl_ServerExec \
  vmcontrol_wrapper_api_get()->xVMControl_ServerExec

#define VMControl_ServerGetLastError \
  vmcontrol_wrapper_api_get()->xVMControl_ServerGetLastError

#define VMControl_ServerGetResource \
  vmcontrol_wrapper_api_get()->xVMControl_ServerGetResource

#define VMControl_ServerIsConnected \
  vmcontrol_wrapper_api_get()->xVMControl_ServerIsConnected

#define VMControl_ServerIsRegistered \
  vmcontrol_wrapper_api_get()->xVMControl_ServerIsRegistered

#define VMControl_ServerNewEx \
  vmcontrol_wrapper_api_get()->xVMControl_ServerNewEx

#define VMControl_VMConnectEx \
  vmcontrol_wrapper_api_get()->xVMControl_VMConnectEx

#define VMControl_VMCreateSnapshot \
  vmcontrol_wrapper_api_get()->xVMControl_VMCreateSnapshot

#define VMControl_VMDestroy \
  vmcontrol_wrapper_api_get()->xVMControl_VMDestroy

#define VMControl_VMDeviceConnect \
  vmcontrol_wrapper_api_get()->xVMControl_VMDeviceConnect

#define VMControl_VMDeviceDisconnect \
  vmcontrol_wrapper_api_get()->xVMControl_VMDeviceDisconnect

#define VMControl_VMDeviceIsConnected \
  vmcontrol_wrapper_api_get()->xVMControl_VMDeviceIsConnected

#define VMControl_VMDisconnect \
  vmcontrol_wrapper_api_get()->xVMControl_VMDisconnect

#define VMControl_VMGetCapabilities \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetCapabilities

#define VMControl_VMGetConfig \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetConfig

#define VMControl_VMGetConfigFileName \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetConfigFileName

#define VMControl_VMGetExecutionState \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetExecutionState

#define VMControl_VMGetGuestInfo \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetGuestInfo

#define VMControl_VMGetHeartbeat \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetHeartbeat

#define VMControl_VMGetId \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetId

#define VMControl_VMGetLastError \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetLastError

#define VMControl_VMGetPid \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetPid

#define VMControl_VMGetProductInfo \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetProductInfo

#define VMControl_VMGetRemoteConnections \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetRemoteConnections

#define VMControl_VMGetResource \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetResource

#define VMControl_VMGetRunAsUser \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetRunAsUser

#define VMControl_VMGetUptime \
  vmcontrol_wrapper_api_get()->xVMControl_VMGetUptime

#define VMControl_VMHasSnapshot \
  vmcontrol_wrapper_api_get()->xVMControl_VMHasSnapshot

#define VMControl_VMInit \
  vmcontrol_wrapper_api_get()->xVMControl_VMInit

#define VMControl_VMIsConnected \
  vmcontrol_wrapper_api_get()->xVMControl_VMIsConnected

#define VMControl_VMNewEx \
  vmcontrol_wrapper_api_get()->xVMControl_VMNewEx

#define VMControl_VMRemoveAllSnapshots \
  vmcontrol_wrapper_api_get()->xVMControl_VMRemoveAllSnapshots

#define VMControl_VMRevertToSnapshot \
  vmcontrol_wrapper_api_get()->xVMControl_VMRevertToSnapshot

#define VMControl_VMSetConfig \
  vmcontrol_wrapper_api_get()->xVMControl_VMSetConfig

#define VMControl_VMSetGuestInfo \
  vmcontrol_wrapper_api_get()->xVMControl_VMSetGuestInfo

#define VMControl_VMStart \
  vmcontrol_wrapper_api_get()->xVMControl_VMStart

#define VMControl_VMStopOrReset \
  vmcontrol_wrapper_api_get()->xVMControl_VMStopOrReset

#define VMControl_VMSuspendToDisk \
  vmcontrol_wrapper_api_get()->xVMControl_VMSuspendToDisk

#define VMControl_VMToolsLastActive \
  vmcontrol_wrapper_api_get()->xVMControl_VMToolsLastActive

#endif

#endif /* VMCONTROL_WRAPPER_H */
