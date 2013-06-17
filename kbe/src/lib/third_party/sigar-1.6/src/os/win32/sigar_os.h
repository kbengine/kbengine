/*
 * Copyright (c) 2004-2009 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2009-2010 VMware, Inc.
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

#ifndef SIGAR_OS_H
#define SIGAR_OS_H

#if _MSC_VER <= 1200
#define SIGAR_USING_MSC6 /* Visual Studio version 6 */
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winreg.h>
#include <winperf.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stddef.h>
#include <sys/types.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <tlhelp32.h>

#include "sigar_util.h"

#define INT64_C(val) val##i64

/* see apr/include/arch/win32/atime.h */
#define EPOCH_DELTA INT64_C(11644473600000000)

#define SIGAR_CMDLINE_MAX 4096

/* XXX: support CP_UTF8 ? */

#define SIGAR_A2W(lpa, lpw, bytes) \
    (lpw[0] = 0, MultiByteToWideChar(CP_ACP, 0, \
                                     lpa, -1, lpw, (bytes/sizeof(WCHAR))))

#define SIGAR_W2A(lpw, lpa, chars) \
    (lpa[0] = '\0', WideCharToMultiByte(CP_ACP, 0, \
                                        lpw, -1, (LPSTR)lpa, chars, \
                                        NULL, NULL))

/* iptypes.h from vc7, not available in vc6 */
/* copy from PSDK if using vc6 */
#include "iptypes.h"

/* from wtsapi32.h not in vs6.0 */
typedef enum {
    WTSInitialProgram,
    WTSApplicationName,
    WTSWorkingDirectory,
    WTSOEMId,
    WTSSessionId,
    WTSUserName,
    WTSWinStationName,
    WTSDomainName,
    WTSConnectState,
    WTSClientBuildNumber,
    WTSClientName,
    WTSClientDirectory,
    WTSClientProductId,
    WTSClientHardwareId,
    WTSClientAddress,
    WTSClientDisplay,
    WTSClientProtocolType,
} WTS_INFO_CLASS;

typedef enum _WTS_CONNECTSTATE_CLASS {
    WTSActive,
    WTSConnected,
    WTSConnectQuery,
    WTSShadow,
    WTSDisconnected,
    WTSIdle,
    WTSListen,
    WTSReset,
    WTSDown,
    WTSInit
} WTS_CONNECTSTATE_CLASS;

#define WTS_PROTOCOL_TYPE_CONSOLE 0
#define WTS_PROTOCOL_TYPE_ICA     1
#define WTS_PROTOCOL_TYPE_RDP     2

typedef struct _WTS_SESSION_INFO {
    DWORD SessionId;
    LPTSTR pWinStationName;
    DWORD State;
} WTS_SESSION_INFO, *PWTS_SESSION_INFO;

typedef struct _WTS_PROCESS_INFO {
    DWORD SessionId;
    DWORD ProcessId;
    LPSTR pProcessName;
    PSID pUserSid;
} WTS_PROCESS_INFO, *PWTS_PROCESS_INFO;

typedef struct _WTS_CLIENT_ADDRESS {
    DWORD AddressFamily;
    BYTE Address[20];
} WTS_CLIENT_ADDRESS, *PWTS_CLIENT_ADDRESS;

/* the WINSTATION_INFO stuff here is undocumented
 * got the howto from google groups:
 * http://redirx.com/?31gy
 */
typedef enum _WINSTATION_INFO_CLASS {
    WinStationInformation = 8
} WINSTATION_INFO_CLASS;

typedef struct _WINSTATION_INFO {
    BYTE Reserved1[72];
    ULONG SessionId;
    BYTE Reserved2[4];
    FILETIME ConnectTime;
    FILETIME DisconnectTime;
    FILETIME LastInputTime;
    FILETIME LoginTime;
    BYTE Reserved3[1096];
    FILETIME CurrentTime;
} WINSTATION_INFO, *PWINSTATION_INFO;

/* end wtsapi32.h */

#ifdef SIGAR_USING_MSC6

/* from winbase.h not in vs6.0 */
typedef struct {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    DWORDLONG ullTotalPhys;
    DWORDLONG ullAvailPhys;
    DWORDLONG ullTotalPageFile;
    DWORDLONG ullAvailPageFile;
    DWORDLONG ullTotalVirtual;
    DWORDLONG ullAvailVirtual;
    DWORDLONG ullAvailExtendedVirtual;
} MEMORYSTATUSEX;

/* service manager stuff not in vs6.0 */
typedef struct _SERVICE_STATUS_PROCESS {
    DWORD dwServiceType;
    DWORD dwCurrentState;
    DWORD dwControlsAccepted;
    DWORD dwWin32ExitCode;
    DWORD dwServiceSpecificExitCode;
    DWORD dwCheckPoint;
    DWORD dwWaitHint;
    DWORD dwProcessId;
    DWORD dwServiceFlags;
} SERVICE_STATUS_PROCESS;

typedef enum {
    SC_STATUS_PROCESS_INFO = 0
} SC_STATUS_TYPE;

#ifndef ERROR_DATATYPE_MISMATCH
#define ERROR_DATATYPE_MISMATCH 1629L
#endif

#endif /* _MSC_VER */

#include <iprtrmib.h>

/* undocumented structures */
typedef struct {
    DWORD   dwState;
    DWORD   dwLocalAddr;
    DWORD   dwLocalPort;
    DWORD   dwRemoteAddr;
    DWORD   dwRemotePort;
    DWORD   dwProcessId;
} MIB_TCPEXROW, *PMIB_TCPEXROW;

typedef struct {
    DWORD dwNumEntries;
    MIB_TCPEXROW table[ANY_SIZE];
} MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;

typedef struct {
    DWORD dwLocalAddr;
    DWORD dwLocalPort;
    DWORD dwProcessId;
} MIB_UDPEXROW, *PMIB_UDPEXROW;

typedef struct {
    DWORD dwNumEntries;
    MIB_UDPEXROW table[ANY_SIZE];
} MIB_UDPEXTABLE, *PMIB_UDPEXTABLE;

/* end undocumented structures */

/* no longer in the standard header files */
typedef struct {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

#define SystemProcessorPerformanceInformation 8

/* PEB decls from msdn docs w/ slight mods */
#define ProcessBasicInformation 0

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PEB_LDR_DATA {
    BYTE Reserved1[8];
    PVOID Reserved2[3];
    LIST_ENTRY InMemoryOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct RTL_DRIVE_LETTER_CURDIR {
    USHORT              Flags;
    USHORT              Length;
    ULONG               TimeStamp;
    UNICODE_STRING      DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

/* from: http://source.winehq.org/source/include/winternl.h */
typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG               AllocationSize;
    ULONG               Size;
    ULONG               Flags;
    ULONG               DebugFlags;
    HANDLE              hConsole;
    ULONG               ProcessGroup;
    HANDLE              hStdInput;
    HANDLE              hStdOutput;
    HANDLE              hStdError;
    UNICODE_STRING      CurrentDirectoryName;
    HANDLE              CurrentDirectoryHandle;
    UNICODE_STRING      DllPath;
    UNICODE_STRING      ImagePathName;
    UNICODE_STRING      CommandLine;
    PWSTR               Environment;
    ULONG               dwX;
    ULONG               dwY;
    ULONG               dwXSize;
    ULONG               dwYSize;
    ULONG               dwXCountChars;
    ULONG               dwYCountChars;
    ULONG               dwFillAttribute;
    ULONG               dwFlags;
    ULONG               wShowWindow;
    UNICODE_STRING      WindowTitle;
    UNICODE_STRING      Desktop;
    UNICODE_STRING      ShellInfo;
    UNICODE_STRING      RuntimeInfo;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

/* from msdn docs
typedef struct _RTL_USER_PROCESS_PARAMETERS {
    BYTE Reserved1[16];
    PVOID Reserved2[10];
    UNICODE_STRING ImagePathName;
    UNICODE_STRING CommandLine;
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;
*/

typedef struct _PEB {
    BYTE Reserved1[2];
    BYTE BeingDebugged;
    BYTE Reserved2[1];
    PVOID Reserved3[2];
    PPEB_LDR_DATA Ldr;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    BYTE Reserved4[104];
    PVOID Reserved5[52];
    /*PPS_POST_PROCESS_INIT_ROUTINE*/ PVOID PostProcessInitRoutine;
    BYTE Reserved6[128];
    PVOID Reserved7[1];
    ULONG SessionId;
} PEB, *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION {
    PVOID Reserved1;
    PPEB PebBaseAddress;
    PVOID Reserved2[2];
    /*ULONG_PTR*/ UINT_PTR UniqueProcessId;
    PVOID Reserved3;
} PROCESS_BASIC_INFORMATION;

typedef struct {
    sigar_pid_t pid;
    int ppid;
    int priority;
    time_t mtime;
    sigar_uint64_t size;
    sigar_uint64_t resident;
    char name[SIGAR_PROC_NAME_LEN];
    char state;
    sigar_uint64_t handles;
    sigar_uint64_t threads;
    sigar_uint64_t page_faults;
} sigar_win32_pinfo_t;

typedef struct {
    const char *name;
    HINSTANCE handle;
} sigar_dll_handle_t;

typedef struct {
    const char *name;
    FARPROC func;
} sigar_dll_func_t;

typedef struct {
    const char *name;
    HINSTANCE handle;
    sigar_dll_func_t funcs[12];
} sigar_dll_module_t;

/* wtsapi.dll */
typedef BOOL (CALLBACK *wtsapi_enum_sessions)(HANDLE,
                                              DWORD,
                                              DWORD,
                                              PWTS_SESSION_INFO *,
                                              DWORD *);

typedef void (CALLBACK *wtsapi_free_mem)(PVOID);

typedef BOOL (CALLBACK *wtsapi_query_session)(HANDLE,
                                              DWORD,
                                              WTS_INFO_CLASS,
                                              LPSTR *, DWORD *);
/* iphlpapi.dll */

typedef DWORD (CALLBACK *iphlpapi_get_ipforward_table)(PMIB_IPFORWARDTABLE,
                                                       PULONG,
                                                       BOOL);

typedef DWORD (CALLBACK *iphlpapi_get_ipaddr_table)(PMIB_IPADDRTABLE,
                                                    PULONG,
                                                    BOOL);

typedef DWORD (CALLBACK *iphlpapi_get_if_table)(PMIB_IFTABLE,
                                                PULONG,
                                                BOOL);

typedef DWORD (CALLBACK *iphlpapi_get_if_entry)(PMIB_IFROW);

typedef DWORD (CALLBACK *iphlpapi_get_num_if)(PDWORD);

typedef DWORD (CALLBACK *iphlpapi_get_tcp_table)(PMIB_TCPTABLE,
                                                 PDWORD,
                                                 BOOL);

typedef DWORD (CALLBACK *iphlpapi_get_udp_table)(PMIB_UDPTABLE,
                                                 PDWORD,
                                                 BOOL);

typedef DWORD (CALLBACK *iphlpapi_get_tcpx_table)(PMIB_TCPEXTABLE *,
                                                  BOOL,
                                                  HANDLE,
                                                  DWORD,
                                                  DWORD);

typedef DWORD (CALLBACK *iphlpapi_get_udpx_table)(PMIB_UDPEXTABLE *,
                                                  BOOL,
                                                  HANDLE,
                                                  DWORD,
                                                  DWORD);

typedef DWORD (CALLBACK *iphlpapi_get_tcp_stats)(PMIB_TCPSTATS);

typedef DWORD (CALLBACK *iphlpapi_get_net_params)(PFIXED_INFO,
                                                  PULONG);

typedef DWORD (CALLBACK *iphlpapi_get_adapters_info)(PIP_ADAPTER_INFO,
                                                     PULONG);

typedef ULONG (CALLBACK *iphlpapi_get_adapters_addrs)(ULONG,
                                                      ULONG,
                                                      PVOID,
                                                      PIP_ADAPTER_ADDRESSES,
                                                      PULONG);

/* advapi32.dll */
typedef BOOL (CALLBACK *advapi_convert_string_sid)(LPCSTR,
                                                   PSID *);

typedef BOOL (CALLBACK *advapi_query_service_status)(SC_HANDLE,
                                                     SC_STATUS_TYPE,
                                                     LPBYTE,
                                                     DWORD,
                                                     LPDWORD);

/* ntdll.dll */
typedef DWORD (CALLBACK *ntdll_query_sys_info)(DWORD,
                                               PVOID,
                                               ULONG,
                                               PULONG);

typedef DWORD (CALLBACK *ntdll_query_proc_info)(HANDLE,
                                                DWORD,
                                                PVOID,
                                                ULONG,
                                                PULONG);

/* psapi.dll */
typedef BOOL (CALLBACK *psapi_enum_modules)(HANDLE,
                                            HMODULE *,
                                            DWORD,
                                            LPDWORD);

typedef DWORD (CALLBACK *psapi_get_module_name)(HANDLE,
                                                HMODULE,
                                                LPTSTR,
                                                DWORD);

typedef BOOL (CALLBACK *psapi_enum_processes)(DWORD *,
                                              DWORD,
                                              DWORD *);

/* winsta.dll */
typedef BOOLEAN (CALLBACK *winsta_query_info)(HANDLE,
                                              ULONG,
                                              WINSTATION_INFO_CLASS,
                                              PVOID,
                                              ULONG,
                                              PULONG);

/* kernel32.dll */
typedef BOOL (CALLBACK *kernel_memory_status)(MEMORYSTATUSEX *);

/* mpr.dll */
typedef BOOL (CALLBACK *mpr_get_net_connection)(LPCTSTR,
                                                LPTSTR,
                                                LPDWORD);

#define SIGAR_DLLFUNC(api, name) \
    struct { \
         const char *name; \
         ##api##_##name func; \
    } ##name

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(wtsapi, enum_sessions);
    SIGAR_DLLFUNC(wtsapi, free_mem);
    SIGAR_DLLFUNC(wtsapi, query_session);

    sigar_dll_func_t end;
} sigar_wtsapi_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(iphlpapi, get_ipforward_table);
    SIGAR_DLLFUNC(iphlpapi, get_ipaddr_table);
    SIGAR_DLLFUNC(iphlpapi, get_if_table);
    SIGAR_DLLFUNC(iphlpapi, get_if_entry);
    SIGAR_DLLFUNC(iphlpapi, get_num_if);
    SIGAR_DLLFUNC(iphlpapi, get_tcp_table);
    SIGAR_DLLFUNC(iphlpapi, get_udp_table);
    SIGAR_DLLFUNC(iphlpapi, get_tcpx_table);
    SIGAR_DLLFUNC(iphlpapi, get_udpx_table);
    SIGAR_DLLFUNC(iphlpapi, get_tcp_stats);
    SIGAR_DLLFUNC(iphlpapi, get_net_params);
    SIGAR_DLLFUNC(iphlpapi, get_adapters_info);
    SIGAR_DLLFUNC(iphlpapi, get_adapters_addrs);

    sigar_dll_func_t end;
} sigar_iphlpapi_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(advapi, convert_string_sid);
    SIGAR_DLLFUNC(advapi, query_service_status);

    sigar_dll_func_t end;
} sigar_advapi_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(ntdll, query_sys_info);
    SIGAR_DLLFUNC(ntdll, query_proc_info);

    sigar_dll_func_t end;
} sigar_ntdll_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(psapi, enum_modules);
    SIGAR_DLLFUNC(psapi, enum_processes);
    SIGAR_DLLFUNC(psapi, get_module_name);

    sigar_dll_func_t end;
} sigar_psapi_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(winsta, query_info);

    sigar_dll_func_t end;
} sigar_winsta_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(kernel, memory_status);

    sigar_dll_func_t end;
} sigar_kernel_t;

typedef struct {
    sigar_dll_handle_t handle;

    SIGAR_DLLFUNC(mpr, get_net_connection);

    sigar_dll_func_t end;
} sigar_mpr_t;

struct sigar_t {
    SIGAR_T_BASE;
    char *machine;
    int using_wide;
    long pagesize;
    HKEY handle;
    char *perfbuf;
    DWORD perfbuf_size;
    sigar_wtsapi_t wtsapi;
    sigar_iphlpapi_t iphlpapi;
    sigar_advapi_t advapi;
    sigar_ntdll_t ntdll;
    sigar_psapi_t psapi;
    sigar_winsta_t winsta;
    sigar_kernel_t kernel;
    sigar_mpr_t mpr;
    sigar_win32_pinfo_t pinfo;
    sigar_cache_t *netif_adapters;
    sigar_cache_t *netif_mib_rows;
    sigar_cache_t *netif_addr_rows;
    sigar_cache_t *netif_names; /* dwIndex -> net_interface_config.name */

    WORD ws_version;
    int ws_error;
    int ht_enabled;
    int lcpu; //number of logical cpus
    int winnt;
};

#ifdef __cplusplus
extern "C" {
#endif

sigar_uint64_t sigar_FileTimeToTime(FILETIME *ft);

int sigar_wsa_init(sigar_t *sigar);

int sigar_proc_exe_peb_get(sigar_t *sigar, HANDLE proc,
                           sigar_proc_exe_t *procexe);

int sigar_proc_args_peb_get(sigar_t *sigar, HANDLE proc,
                            sigar_proc_args_t *procargs);

int sigar_proc_env_peb_get(sigar_t *sigar, HANDLE proc,
                           WCHAR *env, DWORD envlen);

int sigar_proc_args_wmi_get(sigar_t *sigar, sigar_pid_t pid,
                            sigar_proc_args_t *procargs);

int sigar_proc_exe_wmi_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_exe_t *procexe);

int sigar_parse_proc_args(sigar_t *sigar, WCHAR *buf,
                          sigar_proc_args_t *procargs);

int sigar_service_pid_get(sigar_t *sigar, char *name, sigar_pid_t *pid);

typedef struct {
    DWORD size;
    DWORD count;
    ENUM_SERVICE_STATUS *services;
    SC_HANDLE handle;
} sigar_services_status_t;

int sigar_services_status_get(sigar_services_status_t *ss, DWORD state);

void sigar_services_status_close(sigar_services_status_t *ss);

typedef struct sigar_services_walker_t sigar_services_walker_t;

struct sigar_services_walker_t {
    sigar_t *sigar;
    int flags;
    void *data; /* user data */
    int (*add_service)(sigar_services_walker_t *walker, char *name);
};

int sigar_services_query(char *ptql,
                         sigar_ptql_error_t *error,
                         sigar_services_walker_t *walker);

char *sigar_service_exe_get(char *path, char *buffer, int basename);

typedef struct {
    WORD product_major;
    WORD product_minor;
    WORD product_build;
    WORD product_revision;
    WORD file_major;
    WORD file_minor;
    WORD file_build;
    WORD file_revision;
} sigar_file_version_t;

int sigar_file_version_get(sigar_file_version_t *version,
                           char *name,
                           sigar_proc_env_t *infocb);

#ifdef __cplusplus
}
#endif

#define SIGAR_NO_SUCH_PROCESS (SIGAR_OS_START_ERROR+1)

#endif /* SIGAR_OS_H */
