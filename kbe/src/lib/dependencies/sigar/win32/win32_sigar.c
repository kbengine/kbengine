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

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_pdh.h"
#include "sigar_os.h"
#include "sigar_util.h"
#include <shellapi.h>

#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4244)
#pragma warning(disable:4133)
#pragma warning(disable:4307)
#pragma warning(disable:4293)
#pragma warning(disable:4101)

#define USING_WIDE_S(s) (s)->using_wide
#define USING_WIDE()    USING_WIDE_S(sigar)

#define PERFBUF_SIZE 8192

#define PERF_TITLE_PROC       230
#define PERF_TITLE_SYS_KEY   "2"
#define PERF_TITLE_MEM_KEY   "4"
#define PERF_TITLE_PROC_KEY  "230"
#define PERF_TITLE_CPU_KEY   "238"
#define PERF_TITLE_DISK_KEY  "236"

#define PERF_TITLE_CPU_USER    142
#define PERF_TITLE_CPU_IDLE    1746
#define PERF_TITLE_CPU_SYS     144
#define PERF_TITLE_CPU_IRQ     698

typedef enum {
    PERF_IX_CPU_USER,
    PERF_IX_CPU_IDLE,
    PERF_IX_CPU_SYS,
    PERF_IX_CPU_IRQ,
    PERF_IX_CPU_MAX
} perf_cpu_offsets_t;

#define PERF_TITLE_CPUTIME    6
#define PERF_TITLE_PAGE_FAULTS 28
#define PERF_TITLE_MEM_VSIZE  174
#define PERF_TITLE_MEM_SIZE   180
#define PERF_TITLE_THREAD_CNT 680
#define PERF_TITLE_HANDLE_CNT 952
#define PERF_TITLE_PID        784
#define PERF_TITLE_PPID       1410
#define PERF_TITLE_PRIORITY   682
#define PERF_TITLE_START_TIME 684

typedef enum {
    PERF_IX_CPUTIME,
    PERF_IX_PAGE_FAULTS,
    PERF_IX_MEM_VSIZE,
    PERF_IX_MEM_SIZE,
    PERF_IX_THREAD_CNT,
    PERF_IX_HANDLE_CNT,
    PERF_IX_PID,
    PERF_IX_PPID,
    PERF_IX_PRIORITY,
    PERF_IX_START_TIME,
    PERF_IX_MAX
} perf_proc_offsets_t;

typedef enum {
    PERF_IX_DISK_TIME,
    PERF_IX_DISK_READ_TIME,
    PERF_IX_DISK_WRITE_TIME,
    PERF_IX_DISK_READ,
    PERF_IX_DISK_WRITE,
    PERF_IX_DISK_READ_BYTES,
    PERF_IX_DISK_WRITE_BYTES,
    PERF_IX_DISK_QUEUE,
    PERF_IX_DISK_MAX
} perf_disk_offsets_t;

#define PERF_TITLE_DISK_TIME 200 /* % Disk Time */
#define PERF_TITLE_DISK_READ_TIME 202 /* % Disk Read Time */
#define PERF_TITLE_DISK_WRITE_TIME 204 /* % Disk Write Time */
#define PERF_TITLE_DISK_READ  214 /* Disk Reads/sec */
#define PERF_TITLE_DISK_WRITE 216 /* Disk Writes/sec */
#define PERF_TITLE_DISK_READ_BYTES  220 /* Disk Read Bytes/sec */
#define PERF_TITLE_DISK_WRITE_BYTES 222 /* Disk Write Bytes/sec */
#define PERF_TITLE_DISK_QUEUE 198 /* Current Disk Queue Length */

/* 
 * diff is:
 *   ExW      -> ExA
 *   wcounter -> counter
 */
#define MyRegQueryValue() \
    (USING_WIDE() ? \
        RegQueryValueExW(sigar->handle, \
                         wcounter_key, NULL, &type, \
                         sigar->perfbuf, \
                         &bytes) : \
        RegQueryValueExA(sigar->handle, \
                         counter_key, NULL, &type, \
                         sigar->perfbuf, \
                         &bytes))

#define PERF_VAL(ix) \
    perf_offsets[ix] ? \
        *((DWORD *)((BYTE *)counter_block + perf_offsets[ix])) : 0

/* 1/100ns units to milliseconds */
#define NS100_2MSEC(t) ((t) / 10000)

#define PERF_VAL_CPU(ix) \
    NS100_2MSEC(PERF_VAL(ix))

#define MS_LOOPBACK_ADAPTER "Microsoft Loopback Adapter"
#define NETIF_LA "la"

sigar_uint64_t sigar_FileTimeToTime(FILETIME *ft)
{
    sigar_uint64_t time;
    time = ft->dwHighDateTime;
    time = time << 32;
    time |= ft->dwLowDateTime;
    time /= 10;
    time -= EPOCH_DELTA;
    return time;
}

static DWORD perfbuf_init(sigar_t *sigar)
{
    if (!sigar->perfbuf) {
        sigar->perfbuf = malloc(PERFBUF_SIZE);
        sigar->perfbuf_size = PERFBUF_SIZE;
    }

    return sigar->perfbuf_size;
}

static DWORD perfbuf_grow(sigar_t *sigar)
{
    sigar->perfbuf_size += PERFBUF_SIZE;

    sigar->perfbuf =
        realloc(sigar->perfbuf, sigar->perfbuf_size);

    return sigar->perfbuf_size;
}

static char *get_counter_name(char *key)
{
    if (strEQ(key, PERF_TITLE_MEM_KEY)) {
        return "Memory";
    }
    else if (strEQ(key, PERF_TITLE_PROC_KEY)) {
        return "Process";
    }
    else if (strEQ(key, PERF_TITLE_CPU_KEY)) {
        return "Processor";
    }
    else if (strEQ(key, PERF_TITLE_DISK_KEY)) {
        return "LogicalDisk";
    }
    else {
        return key;
    }
}

static PERF_OBJECT_TYPE *get_perf_object_inst(sigar_t *sigar,
                                              char *counter_key,
                                              DWORD inst, DWORD *err)
{
    DWORD retval, type, bytes;
    WCHAR wcounter_key[MAX_PATH+1];
    PERF_DATA_BLOCK *block;
    PERF_OBJECT_TYPE *object;

    *err = SIGAR_OK;

    if (USING_WIDE()) {
        SIGAR_A2W(counter_key, wcounter_key, sizeof(wcounter_key));
    }

    bytes = perfbuf_init(sigar);

    while ((retval = MyRegQueryValue()) != ERROR_SUCCESS) {
        if (retval == ERROR_MORE_DATA) {
            bytes = perfbuf_grow(sigar);
        }
        else {
            *err = retval;
            return NULL;
        }
    }

    block = (PERF_DATA_BLOCK *)sigar->perfbuf;
    if (block->NumObjectTypes == 0) {
        counter_key = get_counter_name(counter_key);
        sigar_strerror_printf(sigar, "No %s counters defined (disabled?)",
                              counter_key);
        *err = -1;
        return NULL;
    }
    object = PdhFirstObject(block);

    /* 
     * only seen on windows 2003 server when pdh.dll
     * functions are in use by the same process.
     * confucius say what the fuck.
     */
    if (inst && (object->NumInstances == PERF_NO_INSTANCES)) {
        int i;

        for (i=0; i<block->NumObjectTypes; i++) {
            if (object->NumInstances != PERF_NO_INSTANCES) {
                return object;
            }
            object = PdhNextObject(object);
        }
        return NULL;
    }
    else {
        return object;
    }
}

#define get_perf_object(sigar, counter_key, err) \
    get_perf_object_inst(sigar, counter_key, 1, err)

static int get_mem_counters(sigar_t *sigar, sigar_swap_t *swap, sigar_mem_t *mem)
{
    int status;
    PERF_OBJECT_TYPE *object =
        get_perf_object_inst(sigar, PERF_TITLE_MEM_KEY, 0, &status);
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    BYTE *data;
    DWORD i;

    if (!object) {
        return status;
    }

    data = (BYTE *)((BYTE *)object + object->DefinitionLength);

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        DWORD offset = counter->CounterOffset;

        switch (counter->CounterNameTitleIndex) {
          case 48: /* "Pages Output/sec" */
            if (swap) swap->page_out = *((DWORD *)(data + offset));
            break;
          case 76: /* "System Cache Resident Bytes" aka file cache */
            if (mem) {
                sigar_uint64_t kern = *((DWORD *)(data + offset));
                mem->actual_free = mem->free + kern;
                mem->actual_used = mem->used - kern;
                return SIGAR_OK;
            }
          case 822: /* "Pages Input/sec" */
            if (swap) swap->page_in = *((DWORD *)(data + offset));
            break;
          default:
            continue;
        }
    }

    return SIGAR_OK;
}

static void get_sysinfo(sigar_t *sigar)
{
    SYSTEM_INFO sysinfo;

    GetSystemInfo(&sysinfo);

    sigar->ncpu = sysinfo.dwNumberOfProcessors;
    sigar->pagesize = sysinfo.dwPageSize;
}

/* for C# bindings */
SIGAR_DECLARE(sigar_t *) sigar_new(void)
{
    sigar_t *sigar;
    if (sigar_open(&sigar) != SIGAR_OK) {
        return NULL;
    }
    return sigar;
}

static sigar_wtsapi_t sigar_wtsapi = {
    "wtsapi32.dll",
    NULL,
    { "WTSEnumerateSessionsA", NULL },
    { "WTSFreeMemory", NULL },
    { "WTSQuerySessionInformationA", NULL },
    { NULL, NULL }
};

static sigar_iphlpapi_t sigar_iphlpapi = {
    "iphlpapi.dll",
    NULL,
    { "GetIpForwardTable", NULL },
    { "GetIpAddrTable", NULL },
    { "GetIfTable", NULL },
    { "GetIfEntry", NULL },
    { "GetNumberOfInterfaces", NULL },
    { "GetTcpTable", NULL },
    { "GetUdpTable", NULL },
    { "AllocateAndGetTcpExTableFromStack", NULL },
    { "AllocateAndGetUdpExTableFromStack", NULL },
    { "GetTcpStatistics", NULL },
    { "GetNetworkParams", NULL },
    { "GetAdaptersInfo", NULL },
    { "GetAdaptersAddresses", NULL },
    { NULL, NULL }
};

static sigar_advapi_t sigar_advapi = {
    "advapi32.dll",
    NULL,
    { "ConvertStringSidToSidA", NULL },
    { "QueryServiceStatusEx", NULL },
    { NULL, NULL }
};

static sigar_ntdll_t sigar_ntdll = {
    "ntdll.dll",
    NULL,
    { "NtQuerySystemInformation", NULL },
    { "NtQueryInformationProcess", NULL },
    { NULL, NULL }
};

static sigar_psapi_t sigar_psapi = {
    "psapi.dll",
    NULL,
    { "EnumProcessModules", NULL },
    { "EnumProcesses", NULL },
    { "GetModuleFileNameExA", NULL },
    { NULL, NULL }
};

static sigar_psapi_t sigar_winsta = {
    "winsta.dll",
    NULL,
    { "WinStationQueryInformationW", NULL },
    { NULL, NULL }
};

static sigar_psapi_t sigar_kernel = {
    "kernel32.dll",
    NULL,
    { "GlobalMemoryStatusEx", NULL },
    { NULL, NULL }
};

static sigar_mpr_t sigar_mpr = {
    "mpr.dll",
    NULL,
    { "WNetGetConnectionA", NULL },
    { NULL, NULL }
};

#define DLLMOD_COPY(name) \
    memcpy(&(sigar->##name), &sigar_##name, sizeof(sigar_##name))

#define DLLMOD_INIT(name, all) \
    sigar_dllmod_init(sigar, (sigar_dll_module_t *)&(sigar->##name), all)

#define DLLMOD_FREE(name) \
    sigar_dllmod_free((sigar_dll_module_t *)&(sigar->##name))

static void sigar_dllmod_free(sigar_dll_module_t *module)
{
    if (module->handle) {
		if(INVALID_HANDLE_VALUE != module->handle)
			FreeLibrary(module->handle);
        module->handle = NULL;
    }
}

static int sigar_dllmod_init(sigar_t *sigar,
                             sigar_dll_module_t *module,
                             int all)
{
    sigar_dll_func_t *funcs = &module->funcs[0];
    int is_debug = SIGAR_LOG_IS_DEBUG(sigar);
    int rc, success;

    if (module->handle == INVALID_HANDLE_VALUE) {
        return ENOENT; /* XXX better rc */
    }

    if (module->handle) {
        return SIGAR_OK;
    }
    
    module->handle = LoadLibrary(module->name);
    if (!(success = (module->handle ? TRUE : FALSE))) {
        rc = GetLastError();
        /* dont try again */
        module->handle = INVALID_HANDLE_VALUE;
    }
    
    if (is_debug) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "LoadLibrary(%s): %s",
                         module->name,
                         success ?
                         "OK" :
                         sigar_strerror(sigar, rc));
    }

    if (!success) {
        return rc;
    }

    while (funcs->name) {
        funcs->func = GetProcAddress(module->handle, funcs->name);

        if (!(success = (funcs->func ? TRUE : FALSE))) {
            rc = GetLastError();
        }

        if (is_debug) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "GetProcAddress(%s:%s): %s",
                             module->name, funcs->name,
                             success ?
                             "OK" :
                             sigar_strerror(sigar, rc));
        }

        if (all && !success) {
            return rc;
        }

        funcs++;
    }

    return SIGAR_OK;
}

int sigar_wsa_init(sigar_t *sigar)
{
    if (sigar->ws_version == 0) {
        WSADATA data;

        if (WSAStartup(MAKEWORD(2, 0), &data)) {
            sigar->ws_error = WSAGetLastError();
            WSACleanup();
            return sigar->ws_error;
        }

        sigar->ws_version = data.wVersion;
    }

    return SIGAR_OK;
}

static int sigar_enable_privilege(char *name)
{
    int status;
    HANDLE handle;
    TOKEN_PRIVILEGES tok;

    SIGAR_ZERO(&tok);

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,
                          &handle))
    {
        return GetLastError();
    }

    if (LookupPrivilegeValue(NULL, name,
                             &tok.Privileges[0].Luid))
    {
        tok.PrivilegeCount = 1;
        tok.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (AdjustTokenPrivileges(handle, FALSE, &tok, 0, NULL, 0)) {
            status = SIGAR_OK;
        }
        else {
            status = GetLastError();
        }
    }
    else {
        status = GetLastError();
    }

    CloseHandle(handle);

    return status;
}

int sigar_os_open(sigar_t **sigar_ptr)
{
    LONG result;
    HINSTANCE h;
    OSVERSIONINFO version;
    int i;
    sigar_t *sigar;

    *sigar_ptr = sigar = malloc(sizeof(*sigar));
    sigar->machine = ""; /* local machine */
    sigar->using_wide = 0; /*XXX*/

    sigar->perfbuf = NULL;
    sigar->perfbuf_size = 0;

    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx(&version);

    /*
     * 4 == NT 4.0
     * 5 == 2000, XP, 2003 Server
     */
    sigar->winnt = (version.dwMajorVersion == 4);

    if (USING_WIDE_S(sigar)) {
        WCHAR wmachine[MAX_PATH+1];

        SIGAR_A2W(sigar->machine, wmachine, sizeof(wmachine));

        result = RegConnectRegistryW(wmachine,
                                     HKEY_PERFORMANCE_DATA,
                                     &sigar->handle);
    }
    else {
        result = RegConnectRegistryA(sigar->machine,
                                     HKEY_PERFORMANCE_DATA,
                                     &sigar->handle);
    }

    get_sysinfo(sigar);

    DLLMOD_COPY(wtsapi);
    DLLMOD_COPY(iphlpapi);
    DLLMOD_COPY(advapi);
    DLLMOD_COPY(ntdll);
    DLLMOD_COPY(psapi);
    DLLMOD_COPY(winsta);
    DLLMOD_COPY(kernel);
    DLLMOD_COPY(mpr);

    sigar->log_level = -1; /* else below segfaults */
    /* XXX init early for use by javasigar.c */
    sigar_dllmod_init(sigar,
                      (sigar_dll_module_t *)&sigar->advapi,
                      FALSE);

    sigar->netif_mib_rows = NULL;
    sigar->netif_addr_rows = NULL;
    sigar->netif_adapters = NULL;
    sigar->netif_names = NULL;
    sigar->pinfo.pid = -1;
    sigar->ws_version = 0;
    sigar->lcpu = -1;

    /* increase process visibility */
    sigar_enable_privilege(SE_DEBUG_NAME);

    return result;
}

void dllmod_init_ntdll(sigar_t *sigar)
{
    DLLMOD_INIT(ntdll, FALSE);
}

int sigar_os_close(sigar_t *sigar)
{
    int retval;

    DLLMOD_FREE(wtsapi);
    DLLMOD_FREE(iphlpapi);
    DLLMOD_FREE(advapi);
    DLLMOD_FREE(ntdll);
    DLLMOD_FREE(psapi);
    DLLMOD_FREE(winsta);
    DLLMOD_FREE(kernel);
    DLLMOD_FREE(mpr);

    if (sigar->perfbuf) {
        free(sigar->perfbuf);
    }

    retval = RegCloseKey(sigar->handle);

    if (sigar->ws_version != 0) {
        WSACleanup();
    }

    if (sigar->netif_mib_rows) {
        sigar_cache_destroy(sigar->netif_mib_rows);
    }

    if (sigar->netif_addr_rows) {
        sigar_cache_destroy(sigar->netif_addr_rows);
    }

    if (sigar->netif_adapters) {
        sigar_cache_destroy(sigar->netif_adapters);
    }

    if (sigar->netif_names) {
        sigar_cache_destroy(sigar->netif_names);
    }

    free(sigar);

    return retval;
}

char *sigar_os_error_string(sigar_t *sigar, int err)
{
    switch (err) {
      case SIGAR_NO_SUCH_PROCESS:
        return "No such process";
        break;
    }
    return NULL;
}

#define sigar_GlobalMemoryStatusEx \
    sigar->kernel.memory_status.func

SIGAR_DECLARE(int) sigar_mem_get(sigar_t *sigar, sigar_mem_t *mem)
{
	sigar->kernel.memory_status.func = &GlobalMemoryStatusEx;
    DLLMOD_INIT(kernel, TRUE);

    if (sigar_GlobalMemoryStatusEx) {
        MEMORYSTATUSEX memstat;

        memstat.dwLength = sizeof(memstat);

        if (!sigar_GlobalMemoryStatusEx(&memstat)) {
            return GetLastError();
        }

        mem->total = memstat.ullTotalPhys;
        mem->free  = memstat.ullAvailPhys;
    }
    else {
        MEMORYSTATUS memstat;
        GlobalMemoryStatus(&memstat);
        mem->total = memstat.dwTotalPhys;
        mem->free  = memstat.dwAvailPhys;
    }

    mem->used = mem->total - mem->free;

    mem->actual_free = mem->free;
    mem->actual_used = mem->used;
    /* set actual_{free,used} */
    get_mem_counters(sigar, NULL, mem);

    sigar_mem_calc_ram(sigar, mem);

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_swap_get(sigar_t *sigar, sigar_swap_t *swap)
{
    int status;
    DLLMOD_INIT(kernel, TRUE);

    if (sigar_GlobalMemoryStatusEx) {
        MEMORYSTATUSEX memstat;

        memstat.dwLength = sizeof(memstat);

        if (!sigar_GlobalMemoryStatusEx(&memstat)) {
            return GetLastError();
        }

        swap->total = memstat.ullTotalPageFile;
        swap->free  = memstat.ullAvailPageFile;
    }
    else {
        MEMORYSTATUS memstat;
        GlobalMemoryStatus(&memstat);
        swap->total = memstat.dwTotalPageFile;
        swap->free  = memstat.dwAvailPageFile;
    }

    swap->used = swap->total - swap->free;

    if (get_mem_counters(sigar, swap, NULL) != SIGAR_OK) {
        swap->page_in = SIGAR_FIELD_NOTIMPL;
        swap->page_out = SIGAR_FIELD_NOTIMPL;
    }

    return SIGAR_OK;
}

static PERF_INSTANCE_DEFINITION *get_cpu_instance(sigar_t *sigar,
                                                  DWORD *perf_offsets,
                                                  DWORD *num, DWORD *err)
{
    PERF_OBJECT_TYPE *object = get_perf_object(sigar, PERF_TITLE_CPU_KEY, err);
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    DWORD i;

    if (!object) {
        return NULL;
    }

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        DWORD offset = counter->CounterOffset;

        switch (counter->CounterNameTitleIndex) {
          case PERF_TITLE_CPU_SYS:
            perf_offsets[PERF_IX_CPU_SYS] = offset;
            break;
          case PERF_TITLE_CPU_USER:
            perf_offsets[PERF_IX_CPU_USER] = offset;
            break;
          case PERF_TITLE_CPU_IDLE:
            perf_offsets[PERF_IX_CPU_IDLE] = offset;
            break;
          case PERF_TITLE_CPU_IRQ:
            perf_offsets[PERF_IX_CPU_IRQ] = offset;
            break;
        }
    }

    if (num) {
        *num = object->NumInstances;
    }

    return PdhFirstInstance(object);
}

#define SPPI_MAX 128 /* XXX unhardcode; should move off this api anyhow */

#define sigar_NtQuerySystemInformation \
   sigar->ntdll.query_sys_info.func

static int get_idle_cpu(sigar_t *sigar, sigar_cpu_t *cpu,
                        DWORD idx,
                        PERF_COUNTER_BLOCK *counter_block,
                        DWORD *perf_offsets)
{
    cpu->idle = 0;

    if (perf_offsets[PERF_IX_CPU_IDLE]) {
        cpu->idle = PERF_VAL_CPU(PERF_IX_CPU_IDLE);
    }
    else {
        /* windows NT and 2000 do not have an Idle counter */
        DLLMOD_INIT(ntdll, FALSE);
        if (sigar_NtQuerySystemInformation) {
            DWORD retval, num;
            SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info[SPPI_MAX];
            /* into the lungs of hell */
            sigar_NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                           &info, sizeof(info), &retval);

            if (!retval) {
                return GetLastError();
            }
            num = retval/sizeof(info[0]);

            if (idx == -1) {
                int i;
                for (i=0; i<num; i++) {
                    cpu->idle += NS100_2MSEC(info[i].IdleTime.QuadPart);
                }
            }
            else if (idx < num) {
                cpu->idle = NS100_2MSEC(info[idx].IdleTime.QuadPart);
            }
            else {
                return ERROR_INVALID_DATA;
            }
        }
        else {
            return ERROR_INVALID_FUNCTION;
        }
    }

    return SIGAR_OK;
}

static int sigar_cpu_perflib_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    int status;
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_BLOCK *counter_block;
    DWORD perf_offsets[PERF_IX_CPU_MAX], err;

    SIGAR_ZERO(cpu);
    memset(&perf_offsets, 0, sizeof(perf_offsets));

    inst = get_cpu_instance(sigar, (DWORD*)&perf_offsets, 0, &err);

    if (!inst) {
        return err;
    }

    /* first instance is total, rest are per-cpu */
    counter_block = PdhGetCounterBlock(inst);

    cpu->sys  = PERF_VAL_CPU(PERF_IX_CPU_SYS);
    cpu->user = PERF_VAL_CPU(PERF_IX_CPU_USER);
    status = get_idle_cpu(sigar, cpu, -1, counter_block, perf_offsets);
    cpu->irq = PERF_VAL_CPU(PERF_IX_CPU_IRQ);
    cpu->nice = 0; /* no nice here */
    cpu->wait = 0; /*N/A?*/
    cpu->total = cpu->sys + cpu->user + cpu->idle + cpu->wait + cpu->irq;

    if (status != SIGAR_OK) {
        sigar_log_printf(sigar, SIGAR_LOG_WARN,
                         "unable to determine idle cpu time: %s",
                         sigar_strerror(sigar, status));
    }

    return SIGAR_OK;
}

static int sigar_cpu_ntsys_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    DWORD retval, num;
    int i;
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info[SPPI_MAX];
    /* into the lungs of hell */
    sigar_NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                   &info, sizeof(info), &retval);

    if (!retval) {
        return GetLastError();
    }
    num = retval/sizeof(info[0]);
    SIGAR_ZERO(cpu);

    for (i=0; i<num; i++) {
        cpu->idle += NS100_2MSEC(info[i].IdleTime.QuadPart);
        cpu->user += NS100_2MSEC(info[i].UserTime.QuadPart);
        cpu->sys  += NS100_2MSEC(info[i].KernelTime.QuadPart -
                                 info[i].IdleTime.QuadPart);
        cpu->irq  += NS100_2MSEC(info[i].InterruptTime.QuadPart);
        cpu->total += cpu->idle + cpu->user + cpu->sys;
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_cpu_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    DLLMOD_INIT(ntdll, FALSE);
    if (sigar_NtQuerySystemInformation) {
        return sigar_cpu_ntsys_get(sigar, cpu);
    }
    else {
        return sigar_cpu_perflib_get(sigar, cpu);
    }
}

static int sigar_cpu_list_perflib_get(sigar_t *sigar,
                                      sigar_cpu_list_t *cpulist)
{
    int status, i, j;
    PERF_INSTANCE_DEFINITION *inst;
    DWORD perf_offsets[PERF_IX_CPU_MAX], num, err;
    int core_rollup = sigar_cpu_core_rollup(sigar);

    memset(&perf_offsets, 0, sizeof(perf_offsets));

    /* first instance is total, rest are per-cpu */
    inst = get_cpu_instance(sigar, (DWORD*)&perf_offsets, &num, &err);

    if (!inst) {
        return err;
    }

    if (!sigar->winnt) {
        /* skip Processor _Total instance (NT doesnt have one) */
        --num;
        inst = PdhNextInstance(inst);
    }

    sigar_cpu_list_create(cpulist);

    /* verify there's a counter for each logical cpu */
    if (core_rollup && (sigar->ncpu != num)) {
        core_rollup = 0;
    }

    for (i=0; i<num; i++) {
        PERF_COUNTER_BLOCK *counter_block;
        sigar_cpu_t *cpu;

        if (core_rollup && (i % sigar->lcpu)) {
            /* merge times of logical processors */
            cpu = &cpulist->data[cpulist->number-1];
        }
        else {
            SIGAR_CPU_LIST_GROW(cpulist);
            cpu = &cpulist->data[cpulist->number++];
            SIGAR_ZERO(cpu);
        }

        counter_block = PdhGetCounterBlock(inst);

        cpu->sys  += PERF_VAL_CPU(PERF_IX_CPU_SYS);
        cpu->user += PERF_VAL_CPU(PERF_IX_CPU_USER);
        cpu->irq  += PERF_VAL_CPU(PERF_IX_CPU_IRQ);
        get_idle_cpu(sigar, cpu, i, counter_block, perf_offsets);
        cpu->nice = cpu->wait = 0; /*N/A*/

        /*XXX adding up too much here if xeon, but not using this atm*/
        cpu->total += cpu->sys + cpu->user + cpu->idle + cpu->irq;

        inst = PdhNextInstance(inst);
    }

    return SIGAR_OK;
}

static int sigar_cpu_list_ntsys_get(sigar_t *sigar,
                                    sigar_cpu_list_t *cpulist)
{
    DWORD retval, num;
    int status, i, j;
    int core_rollup = sigar_cpu_core_rollup(sigar);

    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION info[SPPI_MAX];
    /* into the lungs of hell */
    sigar_NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                          &info, sizeof(info), &retval);

    if (!retval) {
        return GetLastError();
    }
    num = retval/sizeof(info[0]);

    sigar_cpu_list_create(cpulist);

    /* verify there's a counter for each logical cpu */
    if (core_rollup && (sigar->ncpu != num)) {
        core_rollup = 0;
    }

    for (i=0; i<num; i++) {
        sigar_cpu_t *cpu;
        sigar_uint64_t idle, user, sys;

        if (core_rollup && (i % sigar->lcpu)) {
            /* merge times of logical processors */
            cpu = &cpulist->data[cpulist->number-1];
        }
        else {
            SIGAR_CPU_LIST_GROW(cpulist);
            cpu = &cpulist->data[cpulist->number++];
            SIGAR_ZERO(cpu);
        }

        idle = NS100_2MSEC(info[i].IdleTime.QuadPart);
        user = NS100_2MSEC(info[i].UserTime.QuadPart);
        sys  = NS100_2MSEC(info[i].KernelTime.QuadPart -
                           info[i].IdleTime.QuadPart);
        cpu->idle += idle;
        cpu->user += user;
        cpu->sys  += sys;
        cpu->nice = cpu->wait = 0; /*N/A*/
        cpu->total += idle + user + sys;
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_cpu_list_get(sigar_t *sigar,
                                      sigar_cpu_list_t *cpulist)
{
    DLLMOD_INIT(ntdll, FALSE);
    if (sigar_NtQuerySystemInformation) {
        return sigar_cpu_list_ntsys_get(sigar, cpulist);
    }
    else {
        return sigar_cpu_list_perflib_get(sigar, cpulist);
    }
}

#define PERF_TITLE_UPTIME_KEY 674 /* System Up Time */

SIGAR_DECLARE(int) sigar_uptime_get(sigar_t *sigar,
                                    sigar_uptime_t *uptime)
{
    int status;
    PERF_OBJECT_TYPE *object =
        get_perf_object_inst(sigar, PERF_TITLE_SYS_KEY, 0, &status);
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    BYTE *data;
    DWORD i;

    if (!object) {
        return status;
    }

    data = (BYTE *)((BYTE *)object + object->DefinitionLength);

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        if (counter->CounterNameTitleIndex == PERF_TITLE_UPTIME_KEY) {
            DWORD offset = counter->CounterOffset;
            LONGLONG time = object->PerfTime.QuadPart;
            LONGLONG freq = object->PerfFreq.QuadPart;
            LONGLONG counter = *((LONGLONG *)(data + offset));
            uptime->uptime = (time - counter) / freq;
            return SIGAR_OK;
        }
    }

    /* http://msdn.microsoft.com/en-us/library/ms724408.aspx */
    return GetTickCount() / 1000;
}

/*
 * there is no api for this info.
 * closest i've seen is enumerating the entire process table
 * and calculating an average based on process times.
 */
SIGAR_DECLARE(int) sigar_loadavg_get(sigar_t *sigar,
                                     sigar_loadavg_t *loadavg)
{
    return SIGAR_ENOTIMPL;
}

#define get_process_object(sigar, err) \
    get_perf_object(sigar, PERF_TITLE_PROC_KEY, err)

static int sigar_proc_list_get_perf(sigar_t *sigar,
                                    sigar_proc_list_t *proclist)
{

    PERF_OBJECT_TYPE *object;
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    DWORD i, err;
    DWORD perf_offsets[PERF_IX_MAX];

    perf_offsets[PERF_IX_PID] = 0;

    object = get_process_object(sigar, &err);

    if (!object) {
        return err;
    }

    /*
     * note we assume here:
     *  block->NumObjectTypes == 1
     *  object->ObjectNameTitleIndex == PERF_TITLE_PROC
     *
     * which should always be the case.
     */

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        DWORD offset = counter->CounterOffset;

        switch (counter->CounterNameTitleIndex) {
          case PERF_TITLE_PID:
            perf_offsets[PERF_IX_PID] = offset;
            break;
        }
    }

    for (i=0, inst = PdhFirstInstance(object);
         i<object->NumInstances;
         i++, inst = PdhNextInstance(inst))
    {
        PERF_COUNTER_BLOCK *counter_block = PdhGetCounterBlock(inst);
        DWORD pid = PERF_VAL(PERF_IX_PID);

        if (pid == 0) {
            continue; /* dont include the system Idle process */
        }

        SIGAR_PROC_LIST_GROW(proclist);

        proclist->data[proclist->number++] = pid;
    }

    return SIGAR_OK;
}

#define sigar_EnumProcesses \
    sigar->psapi.enum_processes.func

int sigar_os_proc_list_get(sigar_t *sigar,
                           sigar_proc_list_t *proclist)
{
    DLLMOD_INIT(psapi, FALSE);

    if (sigar_EnumProcesses) {
        DWORD retval, *pids;
        DWORD size = 0, i;

        do {
            /* re-use the perfbuf */
            if (size == 0) {
                size = perfbuf_init(sigar);
            }
            else {
                size = perfbuf_grow(sigar);
            }

            if (!sigar_EnumProcesses((DWORD *)sigar->perfbuf,
                                     sigar->perfbuf_size,
                                     &retval))
            {
                return GetLastError();
            }
        } while (retval == sigar->perfbuf_size); //unlikely

        pids = (DWORD *)sigar->perfbuf;

        size = retval / sizeof(DWORD);

        for (i=0; i<size; i++) {
            DWORD pid = pids[i];
            if (pid == 0) {
                continue; /* dont include the system Idle process */
            }
            SIGAR_PROC_LIST_GROW(proclist);
            proclist->data[proclist->number++] = pid;
        }

        return SIGAR_OK;
    }
    else {
        return sigar_proc_list_get_perf(sigar, proclist);
    }
}

#define PROCESS_DAC (PROCESS_QUERY_INFORMATION|PROCESS_VM_READ)

static HANDLE open_process(sigar_pid_t pid)
{
    return OpenProcess(PROCESS_DAC, 0, (DWORD)pid);
}

/*
 * Pretty good explanation of counters:
 * http://www.semack.net/wiki/default.asp?db=SemackNetWiki&o=VirtualMemory
 */
SIGAR_DECLARE(int) sigar_proc_mem_get(sigar_t *sigar, sigar_pid_t pid,
                                      sigar_proc_mem_t *procmem)
{
    int status = get_proc_info(sigar, pid);
    sigar_win32_pinfo_t *pinfo = &sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    procmem->size     = pinfo->size;     /* "Virtual Bytes" */
    procmem->resident = pinfo->resident; /* "Working Set" */
    procmem->share    = SIGAR_FIELD_NOTIMPL;
    procmem->page_faults  = pinfo->page_faults;
    procmem->minor_faults = SIGAR_FIELD_NOTIMPL;
    procmem->major_faults = SIGAR_FIELD_NOTIMPL;

    return SIGAR_OK;
}

#define TOKEN_DAC (STANDARD_RIGHTS_READ | READ_CONTROL | TOKEN_QUERY)

SIGAR_DECLARE(int)
sigar_proc_cred_name_get(sigar_t *sigar, sigar_pid_t pid,
                         sigar_proc_cred_name_t *proccredname)
{
    HANDLE proc, token;
    DWORD len;
    int success;
    TOKEN_USER *user = NULL;
    TOKEN_PRIMARY_GROUP *group = NULL;
    SID_NAME_USE type;
    char domain[SIGAR_CRED_NAME_MAX];

    /* XXX cache lookup */

    if (!(proc = open_process(pid))) {
        return GetLastError();
    }
    
    if (!OpenProcessToken(proc, TOKEN_DAC, &token)) {
        CloseHandle(proc);
        return GetLastError();
    }

    CloseHandle(proc);

    success =
        !GetTokenInformation(token, TokenUser, NULL, 0, &len) &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (user = malloc(len)) &&
        GetTokenInformation(token, TokenUser, user, len, &len);

    if (success) {
        DWORD domain_len = sizeof(domain);
        DWORD user_len = sizeof(proccredname->user);

        success = LookupAccountSid(NULL, user->User.Sid,
                                   proccredname->user, &user_len,
                                   domain, &domain_len, &type);
    }

    if (user != NULL) {
        free(user);
    }
    if (!success) {
        CloseHandle(token);
        return GetLastError();
    }

    success =
        !GetTokenInformation(token, TokenPrimaryGroup, NULL, 0, &len) &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER) &&
        (group = malloc(len)) &&
        GetTokenInformation(token, TokenPrimaryGroup, group, len, &len);

    if (success) {
        DWORD domain_len = sizeof(domain);
        DWORD group_len = sizeof(proccredname->group);

        success = LookupAccountSid(NULL, group->PrimaryGroup,
                                   proccredname->group, &group_len,
                                   domain, &domain_len, &type);
    }

    if (group != NULL) {
        free(group);
    }

    CloseHandle(token);

    if (!success) {
        return GetLastError();
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_proc_cred_get(sigar_t *sigar, sigar_pid_t pid,
                                       sigar_proc_cred_t *proccred)
{
    return SIGAR_ENOTIMPL;
}

#define FILETIME2MSEC(ft) \
    NS100_2MSEC(((ft.dwHighDateTime << 32) | ft.dwLowDateTime))

sigar_int64_t sigar_time_now_millis(void)
{
    SYSTEMTIME st;
    FILETIME time;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &time);

    return sigar_FileTimeToTime(&time) / 1000;
}

SIGAR_DECLARE(int) sigar_proc_time_get(sigar_t *sigar, sigar_pid_t pid,
                                       sigar_proc_time_t *proctime)
{
    HANDLE proc = open_process(pid);
    FILETIME start_time, exit_time, system_time, user_time;
    int status = ERROR_SUCCESS;

    if (!proc) {
        return GetLastError();
    }

    if (!GetProcessTimes(proc,
                         &start_time, &exit_time,
                         &system_time, &user_time))
    {
        status = GetLastError();
    }

    CloseHandle(proc);

    if (status != ERROR_SUCCESS) {
        return status;
    }

    if (start_time.dwHighDateTime) {
        proctime->start_time =
            sigar_FileTimeToTime(&start_time) / 1000;
    }
    else {
        proctime->start_time = 0;
    }

    proctime->user = FILETIME2MSEC(user_time);
    proctime->sys  = FILETIME2MSEC(system_time);
    proctime->total = proctime->user + proctime->sys;

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_proc_state_get(sigar_t *sigar, sigar_pid_t pid,
					sigar_proc_state_t *procstate)
{
    int status = get_proc_info(sigar, pid);
    sigar_win32_pinfo_t *pinfo = &sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    memcpy(procstate->name, pinfo->name, sizeof(procstate->name));
    procstate->state = pinfo->state;
    procstate->ppid = pinfo->ppid;
    procstate->priority = pinfo->priority;
    procstate->nice = SIGAR_FIELD_NOTIMPL;
    procstate->tty =  SIGAR_FIELD_NOTIMPL;
    procstate->threads = pinfo->threads;
    procstate->processor = SIGAR_FIELD_NOTIMPL;

    return SIGAR_OK;
}

static int get_proc_info(sigar_t *sigar, sigar_pid_t pid)
{
    PERF_OBJECT_TYPE *object;
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    DWORD i, err;
    DWORD perf_offsets[PERF_IX_MAX];
    sigar_win32_pinfo_t *pinfo = &sigar->pinfo;
    time_t timenow = time(NULL);

    if (pinfo->pid == pid) {
        if ((timenow - pinfo->mtime) < SIGAR_LAST_PROC_EXPIRE) {
            return SIGAR_OK;
        }
    }

    memset(&perf_offsets, 0, sizeof(perf_offsets));

    object = get_process_object(sigar, &err);

    if (object == NULL) {
        return err;
    }

    pinfo->pid = pid;
    pinfo->mtime = timenow;

    /*
     * note we assume here:
     *  block->NumObjectTypes == 1
     *  object->ObjectNameTitleIndex == PERF_TITLE_PROC
     *
     * which should always be the case.
     */

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        DWORD offset = counter->CounterOffset;

        switch (counter->CounterNameTitleIndex) {
          case PERF_TITLE_CPUTIME:
            perf_offsets[PERF_IX_CPUTIME] = offset;
            break;
          case PERF_TITLE_PAGE_FAULTS:
            perf_offsets[PERF_IX_PAGE_FAULTS] = offset;
            break;
          case PERF_TITLE_MEM_VSIZE:
            perf_offsets[PERF_IX_MEM_VSIZE] = offset;
            break;
          case PERF_TITLE_MEM_SIZE:
            perf_offsets[PERF_IX_MEM_SIZE] = offset;
            break;
          case PERF_TITLE_THREAD_CNT:
            perf_offsets[PERF_IX_THREAD_CNT] = offset;
            break;
          case PERF_TITLE_HANDLE_CNT:
            perf_offsets[PERF_IX_HANDLE_CNT] = offset;
            break;
          case PERF_TITLE_PID:
            perf_offsets[PERF_IX_PID] = offset;
            break;
          case PERF_TITLE_PPID:
            perf_offsets[PERF_IX_PPID] = offset;
            break;
          case PERF_TITLE_PRIORITY:
            perf_offsets[PERF_IX_PRIORITY] = offset;
            break;
          case PERF_TITLE_START_TIME:
            perf_offsets[PERF_IX_START_TIME] = offset;
            break;
        }
    }

    for (i=0, inst = PdhFirstInstance(object);
         i<object->NumInstances;
         i++, inst = PdhNextInstance(inst))
    {
        PERF_COUNTER_BLOCK *counter_block = PdhGetCounterBlock(inst);
        sigar_pid_t this_pid = PERF_VAL(PERF_IX_PID);

        if (this_pid != pid) {
            continue;
        }

        pinfo->state = 'R'; /* XXX? */
        SIGAR_W2A(PdhInstanceName(inst),
                  pinfo->name, sizeof(pinfo->name));

        pinfo->size     = PERF_VAL(PERF_IX_MEM_VSIZE);
        pinfo->resident = PERF_VAL(PERF_IX_MEM_SIZE);
        pinfo->ppid     = PERF_VAL(PERF_IX_PPID);
        pinfo->priority = PERF_VAL(PERF_IX_PRIORITY);
        pinfo->handles  = PERF_VAL(PERF_IX_HANDLE_CNT);
        pinfo->threads  = PERF_VAL(PERF_IX_THREAD_CNT);
        pinfo->page_faults = PERF_VAL(PERF_IX_PAGE_FAULTS);

        return SIGAR_OK;
    }

    return SIGAR_NO_SUCH_PROCESS;
}

static int sigar_remote_proc_args_get(sigar_t *sigar, sigar_pid_t pid,
                                      sigar_proc_args_t *procargs)
{
    int status;
    char cmdline[SIGAR_CMDLINE_MAX], *ptr = cmdline, *arg;
    HANDLE proc = open_process(pid);

    if (proc) {
        status = sigar_proc_args_peb_get(sigar, proc, procargs);

        CloseHandle(proc);

        if (status == SIGAR_OK) {
            return status;
        }
    }

    /* likely we are 32-bit, pid process is 64-bit */
    status = sigar_proc_args_wmi_get(sigar, pid, procargs);
    if (status == ERROR_NOT_FOUND) {
        status = SIGAR_NO_SUCH_PROCESS;
    }
    return status;
}

int sigar_os_proc_args_get(sigar_t *sigar, sigar_pid_t pid,
                          sigar_proc_args_t *procargs)
{
    if (pid == sigar->pid) {
        return sigar_parse_proc_args(sigar, NULL, procargs);
    }
    else {
        return sigar_remote_proc_args_get(sigar, pid, procargs);
    }
}

static int sigar_proc_env_parse(UCHAR *ptr, sigar_proc_env_t *procenv,
                                int multi)
{
    while (*ptr) {
        char *val;
        int klen, vlen, status;
        char key[128]; /* XXX is there a max key size? */

        if (*ptr == '=') {
            ptr += strlen(ptr)+1;
            continue;
        }

        val = strchr(ptr, '=');

        if (val == NULL) {
            break; /*XXX*/
        }

        klen = val - ptr;
        SIGAR_SSTRCPY(key, ptr);
        key[klen] = '\0';
        ++val;

        vlen = strlen(val);

        status = procenv->env_getter(procenv->data,
                                     key, klen, val, vlen);

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            return status;
        }

        if (!multi) {
            break; /* caller only provided 1 key=val pair */
        }

        ptr += klen + 1 + vlen + 1;
    }

    return SIGAR_OK;
}

static int sigar_local_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                                    sigar_proc_env_t *procenv)
{
    UCHAR *env = (UCHAR*)GetEnvironmentStrings();

    sigar_proc_env_parse(env, procenv, TRUE);

    FreeEnvironmentStrings(env);

    return SIGAR_OK;
}

static int sigar_remote_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                                     sigar_proc_env_t *procenv)
{
    int status;
    HANDLE proc = open_process(pid);
    WCHAR env[4096];

    if (!proc) {
        return GetLastError();
    }

    status = sigar_proc_env_peb_get(sigar, proc, env, sizeof(env));

    CloseHandle(proc);

    if (status == SIGAR_OK) {
        LPBYTE ptr = (LPBYTE)env;
        DWORD size = sizeof(env);
        UCHAR ent[4096];

        while ((size > 0) && (*ptr != L'\0')) {
            DWORD len = (wcslen((LPWSTR)ptr) + 1) * sizeof(WCHAR);
            /* multi=FALSE so no need to: memset(ent, '\0', sizeof(ent)) */
            SIGAR_W2A((WCHAR *)ptr, ent, sizeof(ent));
            if (sigar_proc_env_parse(ent, procenv, FALSE) != SIGAR_OK) {
                break;
            }
            size -= len;
            ptr += len;
        }
    }

    return status;
}

SIGAR_DECLARE(int) sigar_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                                      sigar_proc_env_t *procenv)
{
    if (pid == sigar->pid) {
        if (procenv->type == SIGAR_PROC_ENV_KEY) {
            char value[32767]; /* max size from msdn docs */
            DWORD retval = 
                GetEnvironmentVariable(procenv->key, value, sizeof(value));

            if (retval == 0) {
                if (GetLastError() == ERROR_ENVVAR_NOT_FOUND) {
                    return SIGAR_OK;
                }
                return GetLastError();
            }
            else if (retval > sizeof(value)) {
                /* XXX shouldnt happen */
                return GetLastError();
            }

            procenv->env_getter(procenv->data,
                                procenv->key, procenv->klen,
                                value, retval);
            return SIGAR_OK;
        }
        else {
            return sigar_local_proc_env_get(sigar, pid, procenv);
        }
    }
    else {
        return sigar_remote_proc_env_get(sigar, pid, procenv);
    }
}

SIGAR_DECLARE(int) sigar_proc_fd_get(sigar_t *sigar, sigar_pid_t pid,
                                     sigar_proc_fd_t *procfd)
{
    int status;
    sigar_win32_pinfo_t *pinfo = &sigar->pinfo;

    pinfo->pid = -1; /* force update */
    if ((status = get_proc_info(sigar, pid)) != SIGAR_OK) {
        return status;
    }

    procfd->total = pinfo->handles;

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_proc_exe_get(sigar_t *sigar, sigar_pid_t pid,
                                      sigar_proc_exe_t *procexe)
{
    int status = SIGAR_OK;
    HANDLE proc = open_process(pid);

    if (!proc) {
        return GetLastError();
    }

    status = sigar_proc_exe_peb_get(sigar, proc, procexe);
    if (procexe->name[0] == '\0') {
        /* likely we are 32-bit, pid process is 64-bit */
        /* procexe->cwd[0] = XXX where else can we try? */
        status = sigar_proc_exe_wmi_get(sigar, pid, procexe);
        if (status == ERROR_NOT_FOUND) {
            status = SIGAR_NO_SUCH_PROCESS;
        }
    }

    if (procexe->cwd[0] != '\0') {
        /* strip trailing '\' */
        int len = strlen(procexe->cwd);
        if (procexe->cwd[len-1] == '\\') {
            procexe->cwd[len-1] = '\0';
        }
        /* uppercase driver letter */
        procexe->cwd[0] = toupper(procexe->cwd[0]);
        /* e.g. C:\ */
        strncpy(procexe->root, procexe->cwd, 3);
        procexe->root[3] = '\0';
    }
    else {
        procexe->root[0] = '\0';
    }

    if (procexe->name[0] != '\0') {
        /* uppercase driver letter */
        procexe->name[0] = toupper(procexe->name[0]);
    }

    CloseHandle(proc);

    return status;
}

#define sigar_EnumProcessModules \
    sigar->psapi.enum_modules.func

#define sigar_GetModuleFileNameEx \
    sigar->psapi.get_module_name.func

SIGAR_DECLARE(int) sigar_proc_modules_get(sigar_t *sigar, sigar_pid_t pid,
                                          sigar_proc_modules_t *procmods)
{
    HANDLE proc; 
    HMODULE modules[1024];
    DWORD size = 0;
    unsigned int i;

    if (DLLMOD_INIT(psapi, TRUE) != SIGAR_OK) {
        return SIGAR_ENOTIMPL;
    }

    if (!(proc = open_process(pid))) {
        return GetLastError();
    }

    if (!sigar_EnumProcessModules(proc, modules, sizeof(modules), &size)) {
        CloseHandle(proc);
        return GetLastError();
    }

    for (i=0; i<(size/sizeof(HMODULE)); i++) {
        int status;
        char name[MAX_PATH];

        if (!sigar_GetModuleFileNameEx(proc, modules[i],
                                       name, sizeof(name)))
        {
            continue;
        }

        status = procmods->module_getter(procmods->data,
                                         name, strlen(name));

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            break;
        }
    }

    CloseHandle(proc);

    return SIGAR_OK;
}

#define FT2INT64(ft) \
  ((__int64)((__int64)(ft).dwHighDateTime << 32 | \
             (__int64)(ft).dwLowDateTime))

SIGAR_DECLARE(int) sigar_thread_cpu_get(sigar_t *sigar,
                                        sigar_uint64_t id,
                                        sigar_thread_cpu_t *cpu)
{
    FILETIME start, exit, sys, user;
    DWORD retval;

    if (id != 0) {
        return SIGAR_ENOTIMPL;
    }

    retval = GetThreadTimes(GetCurrentThread(),
                            &start, &exit, &sys, &user);

    if (retval == 0) {
        return GetLastError();
    }

    cpu->user  = FT2INT64(user) * 100;
    cpu->sys   = FT2INT64(sys)  * 100;
    cpu->total = (FT2INT64(user) + FT2INT64(sys)) * 100;

    return SIGAR_OK;
}

int sigar_os_fs_type_get(sigar_file_system_t *fsp)
{
    return fsp->type;
}

#ifndef FILE_READ_ONLY_VOLUME 
#define FILE_READ_ONLY_VOLUME 0x00080000
#endif
#ifndef FILE_NAMED_STREAMS
#define FILE_NAMED_STREAMS 0x00040000
#endif
#ifndef FILE_SEQUENTIAL_WRITE_ONCE
#define FILE_SEQUENTIAL_WRITE_ONCE 0x00100000
#endif
#ifndef FILE_SUPPORTS_TRANSACTIONS
#define FILE_SUPPORTS_TRANSACTIONS 0x00200000
#endif

static void get_fs_options(char *opts, int osize, long flags)
{
    *opts = '\0';
    if (flags & FILE_READ_ONLY_VOLUME)        strncat(opts, "ro", osize);
    else                                      strncat(opts, "rw", osize);
#if 0 /*XXX*/
    if (flags & FILE_CASE_PRESERVED_NAMES)    strncat(opts, ",casepn", osize);
    if (flags & FILE_CASE_SENSITIVE_SEARCH)   strncat(opts, ",casess", osize);
    if (flags & FILE_FILE_COMPRESSION)        strncat(opts, ",fcomp", osize);
    if (flags & FILE_NAMED_STREAMS)           strncat(opts, ",streams", osize);
    if (flags & FILE_PERSISTENT_ACLS)         strncat(opts, ",acls", osize);
    if (flags & FILE_SEQUENTIAL_WRITE_ONCE)   strncat(opts, ",wronce", osize);
    if (flags & FILE_SUPPORTS_ENCRYPTION)     strncat(opts, ",efs", osize);
    if (flags & FILE_SUPPORTS_OBJECT_IDS)     strncat(opts, ",oids", osize);
    if (flags & FILE_SUPPORTS_REPARSE_POINTS) strncat(opts, ",reparse", osize);
    if (flags & FILE_SUPPORTS_SPARSE_FILES)   strncat(opts, ",sparse", osize);
    if (flags & FILE_SUPPORTS_TRANSACTIONS)   strncat(opts, ",trans", osize);
    if (flags & FILE_UNICODE_ON_DISK)         strncat(opts, ",unicode", osize);
    if (flags & FILE_VOLUME_IS_COMPRESSED)    strncat(opts, ",vcomp", osize);
    if (flags & FILE_VOLUME_QUOTAS)           strncat(opts, ",quota", osize);
#endif
}

#define sigar_WNetGetConnection \
    sigar->mpr.get_net_connection.func

SIGAR_DECLARE(int) sigar_file_system_list_get(sigar_t *sigar,
                                              sigar_file_system_list_t *fslist)
{
    char name[256];
    char *ptr = name;
    /* XXX: hmm, Find{First,Next}Volume not available in my sdk */
    DWORD len = GetLogicalDriveStringsA(sizeof(name), name);

    DLLMOD_INIT(mpr, TRUE);

    if (len == 0) {
        return GetLastError();
    }

    sigar_file_system_list_create(fslist);

    while (*ptr) {
        sigar_file_system_t *fsp;
        DWORD flags, serialnum=0;
        char fsname[1024];
        UINT drive_type = GetDriveType(ptr);
        int type;

        switch (drive_type) {
          case DRIVE_FIXED:
            type = SIGAR_FSTYPE_LOCAL_DISK;
            break;
          case DRIVE_REMOTE:
            type = SIGAR_FSTYPE_NETWORK;
            break;
          case DRIVE_CDROM:
            type = SIGAR_FSTYPE_CDROM;
            break;
          case DRIVE_RAMDISK:
            type = SIGAR_FSTYPE_RAM_DISK;
            break;
          case DRIVE_REMOVABLE:
            /* skip floppy, usb, etc. drives */
            ptr += strlen(ptr)+1;
            continue;
          default:
            type = SIGAR_FSTYPE_NONE;
            break;
        }

        fsname[0] = '\0';

        GetVolumeInformation(ptr, NULL, 0, &serialnum, NULL,
                             &flags, fsname, sizeof(fsname));

        if (!serialnum && (drive_type == DRIVE_FIXED)) {
            ptr += strlen(ptr)+1;
            continue; /* ignore unformatted partitions */
        }

        SIGAR_FILE_SYSTEM_LIST_GROW(fslist);

        fsp = &fslist->data[fslist->number++];

        fsp->type = type;
        SIGAR_SSTRCPY(fsp->dir_name, ptr);
        SIGAR_SSTRCPY(fsp->dev_name, ptr);

        if ((drive_type == DRIVE_REMOTE) && sigar_WNetGetConnection) {
            DWORD len = sizeof(fsp->dev_name);
            char drive[3] = {'\0', ':', '\0'}; /* e.g. "X:" w/o trailing "\" */
            drive[0] = fsp->dir_name[0];
            sigar_WNetGetConnection(drive, fsp->dev_name, &len);
            /* ignoring failure, leaving dev_name as dir_name */
        }

        /* we set fsp->type, just looking up sigar.c:fstype_names[type] */
        sigar_fs_type_get(fsp);

        if (*fsname == '\0') {
            SIGAR_SSTRCPY(fsp->sys_type_name, fsp->type_name);
        }
        else {
            SIGAR_SSTRCPY(fsp->sys_type_name, fsname); /* CDFS, NTFS, etc */
        }

        get_fs_options(fsp->options, sizeof(fsp->options)-1, flags);

        ptr += strlen(ptr)+1;
    }

    return SIGAR_OK;
}

static PERF_INSTANCE_DEFINITION *get_disk_instance(sigar_t *sigar,
                                                   DWORD *perf_offsets,
                                                   DWORD *num, DWORD *err)
{
    PERF_OBJECT_TYPE *object =
        get_perf_object(sigar, PERF_TITLE_DISK_KEY, err);
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    DWORD i, found=0;

    if (!object) {
        return NULL;
    }

    for (i=0, counter = PdhFirstCounter(object);
         i<object->NumCounters;
         i++, counter = PdhNextCounter(counter))
    {
        DWORD offset = counter->CounterOffset;

        switch (counter->CounterNameTitleIndex) {
          case PERF_TITLE_DISK_TIME:
            perf_offsets[PERF_IX_DISK_TIME] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_READ_TIME:
            perf_offsets[PERF_IX_DISK_READ_TIME] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_WRITE_TIME:
            perf_offsets[PERF_IX_DISK_WRITE_TIME] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_READ:
            perf_offsets[PERF_IX_DISK_READ] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_WRITE:
            perf_offsets[PERF_IX_DISK_WRITE] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_READ_BYTES:
            perf_offsets[PERF_IX_DISK_READ_BYTES] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_WRITE_BYTES:
            perf_offsets[PERF_IX_DISK_WRITE_BYTES] = offset;
            found = 1;
            break;
          case PERF_TITLE_DISK_QUEUE:
            perf_offsets[PERF_IX_DISK_QUEUE] = offset;
            found = 1;
            break;
        }
    }

    if (!found) {
        *err = ENOENT;
        return NULL;
    }

    if (num) {
        *num = object->NumInstances;
    }

    return PdhFirstInstance(object);
}

SIGAR_DECLARE(int) sigar_disk_usage_get(sigar_t *sigar,
                                        const char *dirname,
                                        sigar_disk_usage_t *disk)
{
    DWORD i, err;
    PERF_OBJECT_TYPE *object =
        get_perf_object(sigar, PERF_TITLE_DISK_KEY, &err);
    PERF_INSTANCE_DEFINITION *inst;
    PERF_COUNTER_DEFINITION *counter;
    DWORD perf_offsets[PERF_IX_DISK_MAX];

    SIGAR_DISK_STATS_INIT(disk);

    if (!object) {
        return err;
    }

    memset(&perf_offsets, 0, sizeof(perf_offsets));
    inst = get_disk_instance(sigar, (DWORD*)&perf_offsets, 0, &err);

    if (!inst) {
        return err;
    }

    for (i=0, inst = PdhFirstInstance(object);
         i<object->NumInstances;
         i++, inst = PdhNextInstance(inst))
    {
        char drive[MAX_PATH];
        PERF_COUNTER_BLOCK *counter_block = PdhGetCounterBlock(inst);
        wchar_t *name = (wchar_t *)((BYTE *)inst + inst->NameOffset);

        SIGAR_W2A(name, drive, sizeof(drive));

        if (sigar_isdigit(*name)) {
            char *ptr = strchr(drive, ' '); /* 2000 Server "0 C:" */

            if (ptr) {
                ++ptr;
                SIGAR_SSTRCPY(drive, ptr);
            }
            else {
                /* XXX NT is a number only "0", how to map? */
            }
        }

        if (strnEQ(drive, dirname, 2)) {
            disk->time   = PERF_VAL(PERF_IX_DISK_TIME);
            disk->rtime  = PERF_VAL(PERF_IX_DISK_READ_TIME);
            disk->wtime  = PERF_VAL(PERF_IX_DISK_WRITE_TIME);
            disk->reads  = PERF_VAL(PERF_IX_DISK_READ);
            disk->writes = PERF_VAL(PERF_IX_DISK_WRITE);
            disk->read_bytes  = PERF_VAL(PERF_IX_DISK_READ_BYTES);
            disk->write_bytes = PERF_VAL(PERF_IX_DISK_WRITE_BYTES);
            disk->queue = PERF_VAL(PERF_IX_DISK_QUEUE);
            return SIGAR_OK;
        }
    }

    return ENXIO;
}

SIGAR_DECLARE(int)
sigar_file_system_usage_get(sigar_t *sigar,
                            const char *dirname,
                            sigar_file_system_usage_t *fsusage)
{
    BOOL retval;
    ULARGE_INTEGER avail, total, free;
    int status;

    /* prevent dialog box if A:\ drive is empty */
    UINT errmode = SetErrorMode(SEM_FAILCRITICALERRORS);

    retval = GetDiskFreeSpaceEx(dirname,
                                &avail, &total, &free);

    /* restore previous error mode */
    SetErrorMode(errmode);

    if (!retval) {
        return GetLastError();
    }

    fsusage->total = total.QuadPart / 1024;
    fsusage->free  = free.QuadPart / 1024;
    fsusage->avail = avail.QuadPart / 1024;
    fsusage->used  = fsusage->total - fsusage->free;
    fsusage->use_percent = sigar_file_system_usage_calc_used(sigar, fsusage);

    /* N/A */
    fsusage->files      = SIGAR_FIELD_NOTIMPL;
    fsusage->free_files = SIGAR_FIELD_NOTIMPL;

    status = sigar_disk_usage_get(sigar, dirname, &fsusage->disk);

    return SIGAR_OK;
}

static int sigar_cpu_info_get(sigar_t *sigar, sigar_cpu_info_t *info)
{
    HKEY key, cpu;
    int i = 0;
    char id[MAX_PATH + 1];
    DWORD size = 0, rc;

    RegOpenKey(HKEY_LOCAL_MACHINE,
               "HARDWARE\\DESCRIPTION\\System\\CentralProcessor", &key);

    //just lookup the first id, then assume all cpus are the same.
    rc = RegEnumKey(key, 0, id, sizeof(id));
    if (rc != ERROR_SUCCESS) {
        RegCloseKey(key);
        return rc;
    }
       
    rc = RegOpenKey(key, id, &cpu);
    if (rc != ERROR_SUCCESS) {
        RegCloseKey(key);
        return rc;
    }

    size = sizeof(info->vendor);
    if (RegQueryValueEx(cpu, "VendorIdentifier", NULL, NULL,
                        (LPVOID)&info->vendor, &size) ||
        strEQ(info->vendor, "GenuineIntel"))
    {
        SIGAR_SSTRCPY(info->vendor, "Intel");
    }
    else {
        if (strEQ(info->vendor, "AuthenticAMD")) {
            SIGAR_SSTRCPY(info->vendor, "AMD");
        }
    }

    size = sizeof(info->model);
    if (RegQueryValueEx(cpu, "ProcessorNameString", NULL, NULL,
                        (LPVOID)&info->model, &size))
    {
        size = sizeof(info->model);
        if (RegQueryValueEx(cpu, "Identifier", NULL, NULL,
                            (LPVOID)&info->model, &size))
        {
            SIGAR_SSTRCPY(info->model, "x86");
        }
    }
    else {
        sigar_cpu_model_adjust(sigar, info);
    }

    size = sizeof(info->mhz); // == sizeof(DWORD)
    if (RegQueryValueEx(cpu, "~MHz", NULL, NULL,
                        (LPVOID)&info->mhz, &size))
    {
        info->mhz = -1;
    }

    info->cache_size = -1; //XXX
    RegCloseKey(key);
    RegCloseKey(cpu);

    info->total_cores = sigar->ncpu;
    info->cores_per_socket = sigar->lcpu;
    info->total_sockets = sigar_cpu_socket_count(sigar);

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_cpu_info_list_get(sigar_t *sigar,
                                           sigar_cpu_info_list_t *cpu_infos)
{
    int i, status;
    sigar_cpu_info_t info;
    int core_rollup = sigar_cpu_core_rollup(sigar);

    sigar_cpu_info_list_create(cpu_infos);

    status = sigar_cpu_info_get(sigar, &info);

    if (status != SIGAR_OK) {
        return status;
    }

    for (i=0; i<sigar->ncpu; i++) {
        SIGAR_CPU_INFO_LIST_GROW(cpu_infos);

        if (core_rollup && (i % sigar->lcpu)) {
            continue; /* fold logical processors */
        }

        memcpy(&cpu_infos->data[cpu_infos->number++],
               &info, sizeof(info));
    }

    return SIGAR_OK;
}

#define sigar_GetNetworkParams \
    sigar->iphlpapi.get_net_params.func

#define sigar_GetAdaptersInfo \
    sigar->iphlpapi.get_adapters_info.func

#define sigar_GetAdaptersAddresses \
    sigar->iphlpapi.get_adapters_addrs.func

#define sigar_GetNumberOfInterfaces \
    sigar->iphlpapi.get_num_if.func

static sigar_cache_t *sigar_netif_cache_new(sigar_t *sigar)
{
    DWORD num = 0;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (sigar_GetNumberOfInterfaces) {
        DWORD rc = sigar_GetNumberOfInterfaces(&num);

        if (rc == NO_ERROR) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "GetNumberOfInterfaces=%d",
                             num);
        }
        else {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "GetNumberOfInterfaces failed: %s",
                             sigar_strerror(sigar, rc));
        }
    }

    if (num == 0) {
        num = 10; /* reasonable default */
    }

    return sigar_cache_new(num);
}

static int sigar_get_adapters_info(sigar_t *sigar,
                                   PIP_ADAPTER_INFO *adapter)
{
    ULONG size = sigar->ifconf_len;
    DWORD rc;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetAdaptersInfo) {
        return SIGAR_ENOTIMPL;
    }

    *adapter = (PIP_ADAPTER_INFO)sigar->ifconf_buf;
    rc = sigar_GetAdaptersInfo(*adapter, &size);

    if (rc == ERROR_BUFFER_OVERFLOW) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "GetAdaptersInfo "
                         "realloc ifconf_buf old=%d, new=%d",
                         sigar->ifconf_len, size);
        sigar->ifconf_len = size;
        sigar->ifconf_buf = realloc(sigar->ifconf_buf,
                                    sigar->ifconf_len);

        *adapter = (PIP_ADAPTER_INFO)sigar->ifconf_buf;
        rc = sigar_GetAdaptersInfo(*adapter, &size);
    }

    if (rc != NO_ERROR) {
        return rc;
    }
    else {
        return SIGAR_OK;
    }
}

static int sigar_get_adapter_info(sigar_t *sigar,
                                  DWORD index,
                                  IP_ADAPTER_INFO **adapter)
{
    sigar_cache_entry_t *entry;
    *adapter = NULL;

    if (sigar->netif_adapters) {
        entry = sigar_cache_get(sigar->netif_adapters, index);
        if (entry->value) {
            *adapter = (IP_ADAPTER_INFO *)entry->value;
        }
    }
    else {
        int status;
        IP_ADAPTER_INFO *info;

        sigar->netif_adapters =
            sigar_netif_cache_new(sigar);

        status = sigar_get_adapters_info(sigar, &info);
        if (status != SIGAR_OK) {
            return status;
        }

        while (info) {
            entry = sigar_cache_get(sigar->netif_adapters,
                                    info->Index);
            if (!entry->value) {
                entry->value = malloc(sizeof(*info));
            }
            memcpy(entry->value, info, sizeof(*info));
            if (info->Index == index) {
                *adapter = info;
            }

            info = info->Next;
        }
    }

    if (*adapter) {
        return SIGAR_OK;
    }
    else {
        return ENOENT;
    }
}

static int sigar_get_adapters_addresses(sigar_t *sigar,
                                        PIP_ADAPTER_ADDRESSES *addrs)
{
    ULONG size = sigar->ifconf_len;
    ULONG rc;
    ULONG flags = 
        GAA_FLAG_SKIP_DNS_SERVER|GAA_FLAG_SKIP_MULTICAST;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetAdaptersAddresses) {
        return SIGAR_ENOTIMPL;
    }

    *addrs = (PIP_ADAPTER_ADDRESSES)sigar->ifconf_buf;
    rc = sigar_GetAdaptersAddresses(AF_UNSPEC,
                                    flags,
                                    NULL,
                                    *addrs,
                                    &size);

    if (rc == ERROR_BUFFER_OVERFLOW) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "GetAdaptersAddresses "
                         "realloc ifconf_buf old=%d, new=%d",
                         sigar->ifconf_len, size);
        sigar->ifconf_len = size;
        sigar->ifconf_buf = realloc(sigar->ifconf_buf,
                                    sigar->ifconf_len);

        *addrs = (PIP_ADAPTER_ADDRESSES)sigar->ifconf_buf;
        rc = sigar_GetAdaptersAddresses(AF_UNSPEC,
                                        flags,
                                        NULL,
                                        *addrs,
                                        &size);
    }

    if (rc != ERROR_SUCCESS) {
        return rc;
    }
    else {
        return SIGAR_OK;
    }
}

#define sigar_GetIpAddrTable \
    sigar->iphlpapi.get_ipaddr_table.func

static int sigar_get_ipaddr_table(sigar_t *sigar,
                                  PMIB_IPADDRTABLE *ipaddr)
{
    ULONG size = sigar->ifconf_len;
    DWORD rc;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetIpAddrTable) {
        return SIGAR_ENOTIMPL;
    }

    *ipaddr = (PMIB_IPADDRTABLE)sigar->ifconf_buf;
    rc = sigar_GetIpAddrTable(*ipaddr, &size, FALSE);

    if (rc == ERROR_INSUFFICIENT_BUFFER) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "GetIpAddrTable "
                         "realloc ifconf_buf old=%d, new=%d",
                         sigar->ifconf_len, size);
        sigar->ifconf_len = size;
        sigar->ifconf_buf = realloc(sigar->ifconf_buf,
                                    sigar->ifconf_len);

        *ipaddr = (PMIB_IPADDRTABLE)sigar->ifconf_buf;
        rc = sigar_GetIpAddrTable(*ipaddr, &size, FALSE);
    }

    if (rc != NO_ERROR) {
        return rc;
    }
    else {
        return SIGAR_OK;
    }
}

#ifndef MIB_IPADDR_PRIMARY
#define MIB_IPADDR_PRIMARY 0x0001
#endif

static int sigar_get_netif_ipaddr(sigar_t *sigar,
                                  DWORD index,
                                  MIB_IPADDRROW **ipaddr)
{
    sigar_cache_entry_t *entry;
    *ipaddr = NULL;

    if (sigar->netif_addr_rows) {
        entry = sigar_cache_get(sigar->netif_addr_rows, index);
        if (entry->value) {
            *ipaddr = (MIB_IPADDRROW *)entry->value;
        }
    }
    else {
        int status, i;
        MIB_IPADDRTABLE *mib;

        sigar->netif_addr_rows =
            sigar_netif_cache_new(sigar);

        status = sigar_get_ipaddr_table(sigar, &mib);
        if (status != SIGAR_OK) {
            return status;
        }

        for (i=0; i<mib->dwNumEntries; i++) {
            MIB_IPADDRROW *row = &mib->table[i];
            short type;

#ifdef SIGAR_USING_MSC6
            type = row->unused2;
#else
            type = row->wType;
#endif
            if (!(type & MIB_IPADDR_PRIMARY)) {
                continue;
            }

            entry = sigar_cache_get(sigar->netif_addr_rows,
                                    row->dwIndex);
            if (!entry->value) {
                entry->value = malloc(sizeof(*row));
            }
            memcpy(entry->value, row, sizeof(*row));

            if (row->dwIndex == index) {
                *ipaddr = row;
            }
        }
    }

    if (*ipaddr) {
        return SIGAR_OK;
    }
    else {
        return ENOENT;
    }
}

SIGAR_DECLARE(int) sigar_net_info_get(sigar_t *sigar,
                                      sigar_net_info_t *netinfo)
{
    PIP_ADAPTER_INFO adapter;
    FIXED_INFO *info;
    ULONG len = 0;
    IP_ADDR_STRING *ip;
    DWORD rc;

    DLLMOD_INIT(iphlpapi, FALSE);
    
    if (!sigar_GetNetworkParams) {
        return SIGAR_ENOTIMPL;
    }

    SIGAR_ZERO(netinfo);

    rc = sigar_GetNetworkParams(NULL, &len);
    if (rc != ERROR_BUFFER_OVERFLOW) {
        return rc;
    }

    info = malloc(len);
    rc = sigar_GetNetworkParams(info, &len);
    if (rc != NO_ERROR) {
        free(info);
        return rc;
    }

    SIGAR_SSTRCPY(netinfo->host_name, info->HostName);
    SIGAR_SSTRCPY(netinfo->domain_name, info->DomainName);
    SIGAR_SSTRCPY(netinfo->primary_dns,
                  info->DnsServerList.IpAddress.String);

    if ((ip = info->DnsServerList.Next)) {
        SIGAR_SSTRCPY(netinfo->secondary_dns,
                      ip->IpAddress.String);
    }
    
    free(info);

    if (sigar_get_adapters_info(sigar, &adapter) != SIGAR_OK) {
        return SIGAR_OK;
    }

    while (adapter) {
        /* should only be 1 */
        if (adapter->GatewayList.IpAddress.String[0]) {
            SIGAR_SSTRCPY(netinfo->default_gateway,
                          adapter->GatewayList.IpAddress.String);
        }
#if 0
        if (apapters->DhcpEnabled) {
            SIGAR_SSTRCPY(netinfo->dhcp_server,
                          apdaters->DhcpServer.IpAddress.String);
        }
#endif
        adapter = adapter->Next;
    }

    return SIGAR_OK;
}

#define sigar_GetIpForwardTable \
    sigar->iphlpapi.get_ipforward_table.func

SIGAR_DECLARE(int) sigar_net_route_list_get(sigar_t *sigar,
                                            sigar_net_route_list_t *routelist)
{
    PMIB_IPFORWARDTABLE buffer = NULL;
    ULONG bufsize = 0;
    DWORD rc, i;
    MIB_IPFORWARDTABLE *ipt;
    sigar_net_route_t *route;

    DLLMOD_INIT(iphlpapi, FALSE);
    if (!sigar_GetIpForwardTable) {
        return SIGAR_ENOTIMPL;
    }

    rc = sigar_GetIpForwardTable(buffer, &bufsize, FALSE);
    if (rc != ERROR_INSUFFICIENT_BUFFER) {
        return GetLastError();
    }

    buffer = malloc(bufsize);
    rc = sigar_GetIpForwardTable(buffer, &bufsize, FALSE);
    if (rc != NO_ERROR) {
        free(buffer);
        return GetLastError();
    }

    if (!sigar->netif_names) {
        sigar_net_interface_list_get(sigar, NULL);
    }

    sigar_net_route_list_create(routelist);
    routelist->size = routelist->number = 0;

    ipt = buffer;

    for (i=0; i<ipt->dwNumEntries; i++) {
        MIB_IPFORWARDROW *ipr = ipt->table + i;
        sigar_cache_entry_t *entry;

        SIGAR_NET_ROUTE_LIST_GROW(routelist);

        route = &routelist->data[routelist->number++];
        SIGAR_ZERO(route); /* XXX: other fields */

        sigar_net_address_set(route->destination,
                              ipr->dwForwardDest);
        
        sigar_net_address_set(route->mask,
                              ipr->dwForwardMask);
        
        sigar_net_address_set(route->gateway,
                              ipr->dwForwardNextHop);

        route->metric = ipr->dwForwardMetric1;

        route->flags = SIGAR_RTF_UP;
        if ((ipr->dwForwardDest == 0) &&
            (ipr->dwForwardMask == 0))
        {
            route->flags |= SIGAR_RTF_GATEWAY;
        }

        entry = sigar_cache_get(sigar->netif_names, ipr->dwForwardIfIndex);
        if (entry->value) {
            SIGAR_SSTRCPY(route->ifname, (char *)entry->value);
        }
    }

    free(buffer);

    return SIGAR_OK;
}

#define sigar_GetIfTable \
    sigar->iphlpapi.get_if_table.func

#define sigar_GetIfEntry \
    sigar->iphlpapi.get_if_entry.func

static int sigar_get_if_table(sigar_t *sigar, PMIB_IFTABLE *iftable)
{
    ULONG size = sigar->ifconf_len;
    DWORD rc;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetIfTable) {
        return SIGAR_ENOTIMPL;
    }

    *iftable = (PMIB_IFTABLE)sigar->ifconf_buf;
    rc = sigar_GetIfTable(*iftable, &size, FALSE);

    if (rc == ERROR_INSUFFICIENT_BUFFER) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "GetIfTable "
                         "realloc ifconf_buf old=%d, new=%d",
                         sigar->ifconf_len, size);
        sigar->ifconf_len = size;
        sigar->ifconf_buf = realloc(sigar->ifconf_buf,
                                    sigar->ifconf_len);

        *iftable = (PMIB_IFTABLE)sigar->ifconf_buf;
        rc = sigar_GetIfTable(*iftable, &size, FALSE);
    }

    if (rc != NO_ERROR) {
        return rc;
    }
    else {
        return SIGAR_OK;
    }
}

static int get_mib_ifrow(sigar_t *sigar,
                         const char *name,
                         MIB_IFROW **ifrp)
{
    int status, key, cached=0;
    sigar_cache_entry_t *entry;

    if (sigar->netif_mib_rows) {
        cached = 1;
    }
    else {
        status = sigar_net_interface_list_get(sigar, NULL);
        if (status != SIGAR_OK) {
            return status;
        }
    }
    key = netif_hash(name);
    entry = sigar_cache_get(sigar->netif_mib_rows, key);
    if (!entry->value) {
        return ENOENT;
    }

    *ifrp = (MIB_IFROW *)entry->value;
    if (cached) {
        /* refresh */ 
        if ((status = sigar_GetIfEntry(*ifrp)) != NO_ERROR) {
            return status;
        }
    }

    return SIGAR_OK;
}

static int netif_hash(char *s)
{
    int hash = 0;
    while (*s) {
        hash = 31*hash + *s++; 
    }
    return hash;
}

/* Vista and later, wireless network cards are reported as IF_TYPE_IEEE80211 */
#ifndef IF_TYPE_IEEE80211
#define IF_TYPE_IEEE80211 71
#endif

SIGAR_DECLARE(int)
sigar_net_interface_list_get(sigar_t *sigar,
                             sigar_net_interface_list_t *iflist)
{
    MIB_IFTABLE *ift;
    int i, status;
    int lo=0, eth=0, la=0;

    if (!sigar->netif_mib_rows) {
        sigar->netif_mib_rows =
            sigar_netif_cache_new(sigar);
    }

    if (!sigar->netif_names) {
        sigar->netif_names =
            sigar_netif_cache_new(sigar);
    }

    if ((status = sigar_get_if_table(sigar, &ift)) != SIGAR_OK) {
        return status;
    }

    if (iflist) {
        iflist->number = 0;
        iflist->size = ift->dwNumEntries;
        iflist->data =
            malloc(sizeof(*(iflist->data)) * iflist->size);
    }

    for (i=0; i<ift->dwNumEntries; i++) {
        char name[16];
        int key;
        MIB_IFROW *ifr = ift->table + i;
        sigar_cache_entry_t *entry;

        if (strEQ(ifr->bDescr, MS_LOOPBACK_ADAPTER)) {
            /* special-case */
            sprintf(name, NETIF_LA "%d", la++);
        }
        else if (ifr->dwType == MIB_IF_TYPE_LOOPBACK) {
            sprintf(name, "lo%d", lo++);
        }
        else if ((ifr->dwType == MIB_IF_TYPE_ETHERNET) ||
                 (ifr->dwType == IF_TYPE_IEEE80211))
        {
            sprintf(name, "eth%d", eth++);
        }
        else {
            continue; /*XXX*/
        }

        if (iflist) {
            iflist->data[iflist->number++] = sigar_strdup(name);
        }

        key = netif_hash(name);
        entry = sigar_cache_get(sigar->netif_mib_rows, key);
        if (!entry->value) {
            entry->value = malloc(sizeof(*ifr));
        }
        memcpy(entry->value, ifr, sizeof(*ifr));

        /* save dwIndex -> name mapping for use by route_list */
        entry = sigar_cache_get(sigar->netif_names, ifr->dwIndex);
        if (!entry->value) {
            entry->value = sigar_strdup(name);
        }
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int)
sigar_net_interface_config_get(sigar_t *sigar,
                               const char *name,
                               sigar_net_interface_config_t *ifconfig)
{
    MIB_IFROW *ifr;
    MIB_IPADDRROW *ipaddr;
    int status;

    if (!name) {
        return sigar_net_interface_config_primary_get(sigar, ifconfig);
    }

    status = get_mib_ifrow(sigar, name, &ifr);
    if (status != SIGAR_OK) {
        return status;
    }

    SIGAR_ZERO(ifconfig);

    SIGAR_SSTRCPY(ifconfig->name, name);

    ifconfig->mtu = ifr->dwMtu;

    sigar_net_address_mac_set(ifconfig->hwaddr,
                              ifr->bPhysAddr,
                              SIGAR_IFHWADDRLEN);

    SIGAR_SSTRCPY(ifconfig->description,
                  ifr->bDescr);

    if (ifr->dwOperStatus & MIB_IF_OPER_STATUS_OPERATIONAL) {
        ifconfig->flags |= SIGAR_IFF_UP|SIGAR_IFF_RUNNING;
    }

    status = sigar_get_netif_ipaddr(sigar,
                                    ifr->dwIndex,
                                    &ipaddr);

    if (status == SIGAR_OK) {
        sigar_net_address_set(ifconfig->address,
                              ipaddr->dwAddr);

        sigar_net_address_set(ifconfig->netmask,
                              ipaddr->dwMask);

        if (ifr->dwType != MIB_IF_TYPE_LOOPBACK) {
            if (ipaddr->dwBCastAddr) {
                long bcast =
                    ipaddr->dwAddr & ipaddr->dwMask;

                bcast |= ~ipaddr->dwMask;
                ifconfig->flags |= SIGAR_IFF_BROADCAST;

                sigar_net_address_set(ifconfig->broadcast,
                                      bcast);
            }
        }
    }

    /* hack for MS_LOOPBACK_ADAPTER */
    if (strnEQ(name, NETIF_LA, sizeof(NETIF_LA)-1)) {
        ifr->dwType = MIB_IF_TYPE_LOOPBACK;
    }

    if (ifr->dwType == MIB_IF_TYPE_LOOPBACK) {
        ifconfig->flags |= SIGAR_IFF_LOOPBACK;

        SIGAR_SSTRCPY(ifconfig->type,
                      SIGAR_NIC_LOOPBACK);
    }
    else {
        if (ipaddr) {
            ifconfig->flags |= SIGAR_IFF_MULTICAST;
        }

        SIGAR_SSTRCPY(ifconfig->type,
                      SIGAR_NIC_ETHERNET);
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int)
sigar_net_interface_stat_get(sigar_t *sigar, const char *name,
                             sigar_net_interface_stat_t *ifstat)
{
    MIB_IFROW *ifr;
    int status;

    status = get_mib_ifrow(sigar, name, &ifr);
    if (status != SIGAR_OK) {
        return status;
    }
    
    ifstat->rx_bytes    = ifr->dwInOctets;
    ifstat->rx_packets  = ifr->dwInUcastPkts + ifr->dwInNUcastPkts; 
    ifstat->rx_errors   = ifr->dwInErrors;
    ifstat->rx_dropped  = ifr->dwInDiscards;
    ifstat->rx_overruns = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_frame    = SIGAR_FIELD_NOTIMPL;

    ifstat->tx_bytes      = ifr->dwOutOctets;
    ifstat->tx_packets    = ifr->dwOutUcastPkts + ifr->dwOutNUcastPkts; 
    ifstat->tx_errors     = ifr->dwOutErrors;
    ifstat->tx_dropped    = ifr->dwOutDiscards;
    ifstat->tx_overruns   = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_collisions = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_carrier    = SIGAR_FIELD_NOTIMPL;

    ifstat->speed         = ifr->dwSpeed;

    return SIGAR_OK;
}

#define IS_TCP_SERVER(state, flags) \
    ((flags & SIGAR_NETCONN_SERVER) && (state == MIB_TCP_STATE_LISTEN))

#define IS_TCP_CLIENT(state, flags) \
    ((flags & SIGAR_NETCONN_CLIENT) && (state != MIB_TCP_STATE_LISTEN))

#define sigar_GetTcpTable \
    sigar->iphlpapi.get_tcp_table.func

static int net_conn_get_tcp(sigar_net_connection_walker_t *walker)
{
    sigar_t *sigar = walker->sigar;
    int flags = walker->flags;
    int status, i;
    DWORD rc, size=0;
    PMIB_TCPTABLE tcp;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetTcpTable) {
        return SIGAR_ENOTIMPL;
    }

    rc = sigar_GetTcpTable(NULL, &size, FALSE);
    if (rc != ERROR_INSUFFICIENT_BUFFER) {
        return GetLastError();
    }
    tcp = malloc(size);
    rc = sigar_GetTcpTable(tcp, &size, FALSE);
    if (rc) {
        free(tcp);
        return GetLastError();
    }

    /* go in reverse to get LISTEN states first */
    for (i = (tcp->dwNumEntries-1); i >= 0; i--) {
        sigar_net_connection_t conn;
        DWORD state = tcp->table[i].dwState;

        if (!(IS_TCP_SERVER(state, flags) ||
              IS_TCP_CLIENT(state, flags)))
        {
            continue;
        }

        conn.local_port  = htons((WORD)tcp->table[i].dwLocalPort);
        conn.remote_port = htons((WORD)tcp->table[i].dwRemotePort);

        conn.type = SIGAR_NETCONN_TCP;

        sigar_net_address_set(conn.local_address,
                              tcp->table[i].dwLocalAddr);

        sigar_net_address_set(conn.remote_address,
                              tcp->table[i].dwRemoteAddr);

        conn.send_queue = conn.receive_queue = SIGAR_FIELD_NOTIMPL;

        switch (state) {
          case MIB_TCP_STATE_CLOSED:
            conn.state = SIGAR_TCP_CLOSE;
            break;
          case MIB_TCP_STATE_LISTEN:
            conn.state = SIGAR_TCP_LISTEN;
            break;
          case MIB_TCP_STATE_SYN_SENT:
            conn.state = SIGAR_TCP_SYN_SENT;
            break;
          case MIB_TCP_STATE_SYN_RCVD:
            conn.state = SIGAR_TCP_SYN_RECV;
            break;
          case MIB_TCP_STATE_ESTAB:
            conn.state = SIGAR_TCP_ESTABLISHED;
            break;
          case MIB_TCP_STATE_FIN_WAIT1:
            conn.state = SIGAR_TCP_FIN_WAIT1;
            break;
          case MIB_TCP_STATE_FIN_WAIT2:
            conn.state = SIGAR_TCP_FIN_WAIT2;
            break;
          case MIB_TCP_STATE_CLOSE_WAIT:
            conn.state = SIGAR_TCP_CLOSE_WAIT;
            break;
          case MIB_TCP_STATE_CLOSING:
            conn.state = SIGAR_TCP_CLOSING;
            break;
          case MIB_TCP_STATE_LAST_ACK:
            conn.state = SIGAR_TCP_LAST_ACK;
            break;
          case MIB_TCP_STATE_TIME_WAIT:
            conn.state = SIGAR_TCP_TIME_WAIT;
            break;
          case MIB_TCP_STATE_DELETE_TCB:
          default:
            conn.state = SIGAR_TCP_UNKNOWN;
            break;
        }

        if (walker->add_connection(walker, &conn) != SIGAR_OK) {
            break;
        }
    }

    free(tcp);
    return SIGAR_OK;
}

#define IS_UDP_SERVER(conn, flags) \
    ((flags & SIGAR_NETCONN_SERVER) && !conn.remote_port)

#define IS_UDP_CLIENT(state, flags) \
    ((flags & SIGAR_NETCONN_CLIENT) && conn.remote_port)

#define sigar_GetUdpTable \
    sigar->iphlpapi.get_udp_table.func

static int net_conn_get_udp(sigar_net_connection_walker_t *walker)
{
    sigar_t *sigar = walker->sigar;
    int flags = walker->flags;
    int status;
    DWORD rc, size=0, i;
    PMIB_UDPTABLE udp;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetUdpTable) {
        return SIGAR_ENOTIMPL;
    }

    rc = sigar_GetUdpTable(NULL, &size, FALSE);
    if (rc != ERROR_INSUFFICIENT_BUFFER) {
        return GetLastError();
    }
    udp = malloc(size);
    rc = sigar_GetUdpTable(udp, &size, FALSE);
    if (rc) {
        free(udp);
        return GetLastError();
    }

    for (i = 0; i < udp->dwNumEntries; i++) {
        sigar_net_connection_t conn;

        if (!(IS_UDP_SERVER(conn, flags) ||
              IS_UDP_CLIENT(conn, flags)))
        {
            continue;
        }

        conn.local_port  = htons((WORD)udp->table[i].dwLocalPort);
        conn.remote_port = 0;

        conn.type = SIGAR_NETCONN_UDP;

        sigar_net_address_set(conn.local_address,
                              udp->table[i].dwLocalAddr);

        sigar_net_address_set(conn.remote_address, 0);

        conn.send_queue = conn.receive_queue = SIGAR_FIELD_NOTIMPL;

        if (walker->add_connection(walker, &conn) != SIGAR_OK) {
            break;
        }
    }

    free(udp);
    return SIGAR_OK;
}

SIGAR_DECLARE(int)
sigar_net_connection_walk(sigar_net_connection_walker_t *walker)
{
    int status;

    if (walker->flags & SIGAR_NETCONN_TCP) {
        status = net_conn_get_tcp(walker);

        if (status != SIGAR_OK) {
            return status;
        }
    }

    if (walker->flags & SIGAR_NETCONN_UDP) {
        status = net_conn_get_udp(walker);

        if (status != SIGAR_OK) {
            return status;
        }
    }

    return SIGAR_OK;
}

#define sigar_GetTcpStatistics \
    sigar->iphlpapi.get_tcp_stats.func

SIGAR_DECLARE(int)
sigar_tcp_get(sigar_t *sigar,
              sigar_tcp_t *tcp)
{
    MIB_TCPSTATS mib;
    int status;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (!sigar_GetTcpStatistics) {
        return SIGAR_ENOTIMPL;
    }

    status = sigar_GetTcpStatistics(&mib);

    if (status != NO_ERROR) {
        return status;
    }

    tcp->active_opens = mib.dwActiveOpens;
    tcp->passive_opens = mib.dwPassiveOpens;
    tcp->attempt_fails = mib.dwAttemptFails;
    tcp->estab_resets = mib.dwEstabResets;
    tcp->curr_estab = mib.dwCurrEstab;
    tcp->in_segs = mib.dwInSegs;
    tcp->out_segs = mib.dwOutSegs;
    tcp->retrans_segs = mib.dwRetransSegs;
    tcp->in_errs = mib.dwInErrs;
    tcp->out_rsts = mib.dwOutRsts;

    return SIGAR_OK;
}

SIGAR_DECLARE(int)
sigar_nfs_client_v2_get(sigar_t *sigar,
                        sigar_nfs_client_v2_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

SIGAR_DECLARE(int)
sigar_nfs_server_v2_get(sigar_t *sigar,
                        sigar_nfs_server_v2_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

SIGAR_DECLARE(int)
sigar_nfs_client_v3_get(sigar_t *sigar,
                        sigar_nfs_client_v3_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

SIGAR_DECLARE(int)
sigar_nfs_server_v3_get(sigar_t *sigar,
                        sigar_nfs_server_v3_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

#define sigar_GetTcpExTable \
    sigar->iphlpapi.get_tcpx_table.func

#define sigar_GetUdpExTable \
    sigar->iphlpapi.get_udpx_table.func

SIGAR_DECLARE(int) sigar_proc_port_get(sigar_t *sigar,
                                       int protocol,
                                       unsigned long port,
                                       sigar_pid_t *pid)
{
    DWORD rc, i;

    DLLMOD_INIT(iphlpapi, FALSE);

    if (protocol == SIGAR_NETCONN_TCP) {
        PMIB_TCPEXTABLE tcp;

        if (!sigar_GetTcpExTable) {
            return SIGAR_ENOTIMPL;
        }

        rc = sigar_GetTcpExTable(&tcp, FALSE, GetProcessHeap(),
                                 2, 2);

        if (rc) {
            return GetLastError();
        }

        for (i=0; i<tcp->dwNumEntries; i++) {
            if (tcp->table[i].dwState != MIB_TCP_STATE_LISTEN) {
                continue;
            }

            if (htons((WORD)tcp->table[i].dwLocalPort) != port) {
                continue;
            }

            *pid = tcp->table[i].dwProcessId;
            
            return SIGAR_OK;
        }
    }
    else if (protocol == SIGAR_NETCONN_UDP) {
        PMIB_UDPEXTABLE udp;

        if (!sigar_GetUdpExTable) {
            return SIGAR_ENOTIMPL;
        }

        rc = sigar_GetUdpExTable(&udp, FALSE, GetProcessHeap(),
                                 2, 2);

        if (rc) {
            return GetLastError();
        }

        for (i=0; i<udp->dwNumEntries; i++) {
            if (htons((WORD)udp->table[i].dwLocalPort) != port) {
                continue;
            }

            *pid = udp->table[i].dwProcessId;
            
            return SIGAR_OK;
        }
    }
    else {
        return SIGAR_ENOTIMPL;
    }

    return ENOENT;
}

#include <lm.h>

static int sigar_who_net_sessions(sigar_t *sigar,
                                  sigar_who_list_t *wholist)
{
    NET_API_STATUS status;
    LPSESSION_INFO_10 buffer=NULL, ptr;
    DWORD entries=0, total_entries=0;
    DWORD resume_handle=0;
    DWORD i;

    do {
        status = NetSessionEnum(NULL, /* server name */
                                NULL, /* client name */
                                NULL, /* user name */
                                10,   /* level */
                                (LPBYTE*)&buffer,
                                MAX_PREFERRED_LENGTH,
                                &entries,
                                &total_entries,
                                &resume_handle);

        if ((status == NERR_Success) || (status == ERROR_MORE_DATA)) {
            if ((ptr = buffer)) {
                for (i=0; i<entries; i++) {
                    sigar_who_t *who;

                    if (!ptr) {
                        break;
                    }

                    SIGAR_WHO_LIST_GROW(wholist);
                    who = &wholist->data[wholist->number++];

                    who->time = (time(NULL) - ptr->sesi10_time);
                    SIGAR_W2A((LPCWSTR)ptr->sesi10_username,
                              who->user, sizeof(who->user));
                    SIGAR_W2A((LPCWSTR)ptr->sesi10_cname,
                              who->host, sizeof(who->host));
                    SIGAR_SSTRCPY(who->device, "network share");

                    ptr++;
                }
            }
        }
        else {
            break;
        }

        if (buffer) {
            NetApiBufferFree(buffer);
            buffer = NULL;
        }
    } while (status == ERROR_MORE_DATA);

    if (buffer) {
        NetApiBufferFree(buffer);
    }

    return SIGAR_OK;
}

static int get_logon_info(HKEY users,
                          char *username,
                          sigar_who_t *who)
{
    DWORD status, size, type;
    HKEY key;
    char key_name[MAX_PATH];
    char value[256];
    FILETIME wtime;

    who->time = 0;

    sprintf(key_name, "%s\\Volatile Environment", username);
    if (RegOpenKey(users, key_name, &key) != ERROR_SUCCESS) {
        return ENOENT;
    }

    status = RegQueryInfoKey(key,
                             NULL, NULL, NULL, NULL, NULL,
                             NULL, NULL, NULL, NULL, NULL,
                             &wtime);
    
    if (status == ERROR_SUCCESS) {
        FileTimeToLocalFileTime(&wtime, &wtime);
        who->time = sigar_FileTimeToTime(&wtime) / 1000000;
    }

    size = sizeof(value);
    status = RegQueryValueEx(key, "CLIENTNAME",
                             NULL, &type, value, &size);
    if (status == ERROR_SUCCESS) {
        if ((value[0] != '\0') && !strEQ(value, "Console")) {
            SIGAR_SSTRCPY(who->host, value);
        }
    }

    size = sizeof(value);
    status = RegQueryValueEx(key, "SESSIONNAME",
                             NULL, &type, value, &size);
    if (status == ERROR_SUCCESS) {
        SIGAR_SSTRCPY(who->device, value);
    }

    RegCloseKey(key);

    return SIGAR_OK;
}

#define sigar_ConvertStringSidToSid \
    sigar->advapi.convert_string_sid.func

static int sigar_who_registry(sigar_t *sigar,
                              sigar_who_list_t *wholist)
{
    HKEY users;
    DWORD index=0, status;

    if (!sigar_ConvertStringSidToSid) {
        return ENOENT;
    }

    status = RegOpenKey(HKEY_USERS, NULL, &users);
    if (status != ERROR_SUCCESS) {
        return status;
    }

    while (1) {
        char subkey[MAX_PATH];
        char username[SIGAR_CRED_NAME_MAX];
        char domain[SIGAR_CRED_NAME_MAX];
        DWORD subkey_len = sizeof(subkey);
        DWORD username_len = sizeof(username);
        DWORD domain_len = sizeof(domain);
        PSID sid;
        SID_NAME_USE type;

        status = RegEnumKeyEx(users, index, subkey, &subkey_len,
                              NULL, NULL, NULL, NULL);

        if (status != ERROR_SUCCESS) {
            break;
        }

        index++;

        if ((subkey[0] == '.') || strstr(subkey, "_Classes")) {
            continue;
        }

        if (!sigar_ConvertStringSidToSid(subkey, &sid)) {
            continue;
        }

        if (LookupAccountSid(NULL, /* server */
                             sid, 
                             username, &username_len,
                             domain, &domain_len,
                             &type))
        {
            sigar_who_t *who;

            SIGAR_WHO_LIST_GROW(wholist);
            who = &wholist->data[wholist->number++];
            
            SIGAR_SSTRCPY(who->user, username);
            SIGAR_SSTRCPY(who->host, domain);
            SIGAR_SSTRCPY(who->device, "console");

            get_logon_info(users, subkey, who);
        }               

        LocalFree(sid);
    }

    RegCloseKey(users);

    return SIGAR_OK;
}

#define sigar_WTSEnumerateSessions \
    sigar->wtsapi.enum_sessions.func

#define sigar_WTSFreeMemory \
    sigar->wtsapi.free_mem.func

#define sigar_WTSQuerySessionInformation \
    sigar->wtsapi.query_session.func

#define sigar_WinStationQueryInformation \
    sigar->winsta.query_info.func

static int sigar_who_wts(sigar_t *sigar,
                         sigar_who_list_t *wholist)
{
    DWORD count=0, i;
    WTS_SESSION_INFO *sessions = NULL;

    if (DLLMOD_INIT(wtsapi, TRUE) != SIGAR_OK) {
        sigar_log(sigar, SIGAR_LOG_DEBUG,
                  "Terminal Services api functions not available");
        return ENOENT;
    }

    DLLMOD_INIT(winsta, FALSE);

    if (!sigar_WTSEnumerateSessions(0, 0, 1, &sessions, &count)) {
        return GetLastError();
    }

    for (i=0; i<count; i++) {
        DWORD bytes;
        LPTSTR buffer;
        DWORD sessionId = sessions[i].SessionId;
        WINSTATION_INFO station_info;
        sigar_who_t *who;

        if (sessions[i].State != WTSActive) {
            continue;
        }

        buffer = NULL;
        bytes = 0;
        if (sigar_WTSQuerySessionInformation(0,
                                             sessionId,
                                             WTSClientProtocolType,
                                             &buffer,
                                             &bytes))
        {
            int isConsole = 
                (*buffer == WTS_PROTOCOL_TYPE_CONSOLE);

            sigar_WTSFreeMemory(buffer);

            if (isConsole) {
                continue;
            }
        }

        SIGAR_WHO_LIST_GROW(wholist);
        who = &wholist->data[wholist->number++];

        SIGAR_SSTRCPY(who->device, sessions[i].pWinStationName);

        buffer = NULL;
        bytes = 0;
        if (sigar_WTSQuerySessionInformation(0,
                                             sessionId,
                                             WTSClientAddress,
                                             &buffer,
                                             &bytes))
        {
            PWTS_CLIENT_ADDRESS client =
                (PWTS_CLIENT_ADDRESS)buffer;

            sprintf(who->host, "%u.%u.%u.%u",
                    client->Address[2],
                    client->Address[3],
                    client->Address[4],
                    client->Address[5]);

            sigar_WTSFreeMemory(buffer);
        }
        else {
            SIGAR_SSTRCPY(who->host, "unknown");
        }

        buffer = NULL;
        bytes = 0;
        if (sigar_WTSQuerySessionInformation(0,
                                             sessionId,
                                             WTSUserName,
                                             &buffer,
                                             &bytes))
        {
            SIGAR_SSTRCPY(who->user, buffer);
            sigar_WTSFreeMemory(buffer);
        }
        else {
            SIGAR_SSTRCPY(who->user, "unknown");
        }

        buffer = NULL;
        bytes = 0;
        if (sigar_WinStationQueryInformation &&
            sigar_WinStationQueryInformation(0,
                                             sessionId,
                                             WinStationInformation,
                                             &station_info,
                                             sizeof(station_info),
                                             &bytes))
        {
            who->time =
                sigar_FileTimeToTime(&station_info.ConnectTime) / 1000000;
        }
        else {
            who->time = 0;
        }
    }

    sigar_WTSFreeMemory(sessions);

    return SIGAR_OK;
}

int sigar_who_list_get_win32(sigar_t *sigar,
                             sigar_who_list_t *wholist)
{
    sigar_who_net_sessions(sigar, wholist);

    sigar_who_registry(sigar, wholist);

    sigar_who_wts(sigar, wholist);

    return SIGAR_OK;
}

/* see: http://msdn2.microsoft.com/en-us/library/ms724833.aspx */
#ifndef VER_NT_WORKSTATION
#define VER_NT_WORKSTATION 0x0000001
#endif

#ifdef SIGAR_USING_MSC6
#define sigar_wProductType wReserved[1]
#else
#define sigar_wProductType wProductType
#endif
#ifdef _M_X64
#define SIGAR_ARCH "x64"
#else
#define SIGAR_ARCH "x86"
#endif

int sigar_os_sys_info_get(sigar_t *sigar,
                          sigar_sys_info_t *sysinfo)
{
    OSVERSIONINFOEX version;
    char *vendor_name, *vendor_version, *code_name=NULL;

    version.dwOSVersionInfoSize = sizeof(version);
    GetVersionEx((OSVERSIONINFO *)&version);

    if (version.dwMajorVersion == 4) {
        vendor_name = "Windows NT";
        vendor_version = "NT";
    }
    else if (version.dwMajorVersion == 5) {
        switch (version.dwMinorVersion) {
          case 0:
            vendor_name = "Windows 2000";
            vendor_version = "2000";
            break;
          case 1:
            vendor_name = "Windows XP";
            vendor_version = "XP";
            code_name = "Whistler";
            break;
          case 2:
            vendor_name = "Windows 2003";
            vendor_version = "2003";
            code_name = "Whistler Server";
            break;
          default:
            vendor_name = "Windows Unknown";
            break;
        }
    }
    else if (version.dwMajorVersion == 6) {
        if (version.sigar_wProductType == VER_NT_WORKSTATION) {
            if (version.dwMinorVersion == 0) {
                vendor_name = "Windows Vista";
                vendor_version = "Vista";
                code_name = "Longhorn";
            }
            else {
                vendor_name = "Windows 7";
                vendor_version = "7";
                code_name = "Vienna";
            }
        }
        else {
            vendor_name = "Windows 2008";
            vendor_version = "2008";
            code_name = "Longhorn Server";
        }
    }

    SIGAR_SSTRCPY(sysinfo->name, "Win32");
    SIGAR_SSTRCPY(sysinfo->vendor, "Microsoft");
    SIGAR_SSTRCPY(sysinfo->vendor_name, vendor_name);
    SIGAR_SSTRCPY(sysinfo->vendor_version, vendor_version);
    if (code_name) {
        SIGAR_SSTRCPY(sysinfo->vendor_code_name, code_name);
    }

    SIGAR_SSTRCPY(sysinfo->arch, SIGAR_ARCH);

    sprintf(sysinfo->version, "%d.%d",
            version.dwMajorVersion,
            version.dwMinorVersion);

    SIGAR_SSTRCPY(sysinfo->patch_level,
                  version.szCSDVersion);

    sprintf(sysinfo->description, "%s %s",
            sysinfo->vendor, sysinfo->vendor_name);

    return SIGAR_OK;
}

#define sigar_QueryServiceStatusEx \
    sigar->advapi.query_service_status.func

int sigar_service_pid_get(sigar_t *sigar, char *name, sigar_pid_t *pid)
{
    DWORD rc = ERROR_SUCCESS, len;
    SC_HANDLE mgr;
    HANDLE svc;
    SERVICE_STATUS_PROCESS status;

    if (!sigar_QueryServiceStatusEx) {
        return SIGAR_ENOTIMPL;
    }

    mgr = OpenSCManager(NULL,
                        SERVICES_ACTIVE_DATABASE,
                        SC_MANAGER_ALL_ACCESS);

    if (!mgr) {
        return GetLastError();
    }

    if (!(svc = OpenService(mgr, name, SERVICE_ALL_ACCESS))) {
        CloseServiceHandle(mgr);
        return GetLastError();
    }

    if (sigar_QueryServiceStatusEx(svc,
                                   SC_STATUS_PROCESS_INFO,
                                   (LPBYTE)&status,
                                   sizeof(status), &len))
    {
        *pid = status.dwProcessId;
    }
    else {
        *pid = -1;
        rc = GetLastError();
    }

    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);

    return rc;
}

int sigar_services_status_get(sigar_services_status_t *ss, DWORD state)
{
    DWORD bytes, resume=0;
    BOOL retval;

    if (!ss->handle) {
        ss->handle =
            OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
        if (!ss->handle) {
            return GetLastError();
        }
    }

    retval = EnumServicesStatus(ss->handle,
                                SERVICE_WIN32, state,
                                ss->services, ss->size,
                                &bytes, &ss->count, &resume);
    if (retval == FALSE) {
        DWORD err = GetLastError();
        if (err != ERROR_MORE_DATA) {
            return err;
        }

        ss->services = realloc(ss->services, bytes);
        ss->size = bytes;

        retval = EnumServicesStatus(ss->handle,
                                    SERVICE_WIN32, state,
                                    ss->services, ss->size,
                                    &bytes, &ss->count, &resume);

        if (retval == FALSE) {
            return GetLastError();
        }
    }

    return SIGAR_OK;
}

void sigar_services_status_close(sigar_services_status_t *ss)
{
    if (ss->handle) {
        CloseServiceHandle(ss->handle);
    }
    if (ss->size) {
        free(ss->services);
    }
    SIGAR_ZERO(ss);
}

/* extract exe from QUERY_SERVICE_CONFIG.lpBinaryPathName
 * leaves behind command-line arguments and quotes (if any)
 */
char *sigar_service_exe_get(char *path, char *buffer, int basename)
{
    char *ptr;

    if (path) {
        strncpy(buffer, path, SIGAR_CMDLINE_MAX);
    }
    path = buffer;

    if (*path == '"') {
        ++path;
        if ((ptr = strchr(path, '"'))) {
            *ptr = '\0';
        }
    }
    else {
        ptr = sigar_strcasestr(path, ".exe");

        if (ptr) {
            *(ptr+4) = '\0';
        }
        else {
            if ((ptr = strchr(path, ' '))) {
                *ptr = '\0';
            }
        }
    }

    if (basename && (ptr = strrchr(path, '\\'))) {
        path = ++ptr;
    }

    return path;
}

static char *string_file_info_keys[] = {
    "Comments",
    "CompanyName",
    "FileDescription",
    "FileVersion",
    "InternalName",
    "LegalCopyright",
    "LegalTrademarks",
    "OriginalFilename",
    "ProductName",
    "ProductVersion",
    "PrivateBuild",
    "SpecialBuild",
    NULL
};

int sigar_file_version_get(sigar_file_version_t *version,
                           char *name,
                           sigar_proc_env_t *infocb)
{
    DWORD handle, len;
    LPTSTR data;
    VS_FIXEDFILEINFO *info;
    int status;

    if (!(len = GetFileVersionInfoSize(name, &handle))) {
        return GetLastError();
    }

    if (len == 0) {
        return !SIGAR_OK;
    }
    data = malloc(len);
 
    if (GetFileVersionInfo(name, handle, len, data)) {
        if (VerQueryValue(data, "\\", &info, &len)) {
            version->product_major = HIWORD(info->dwProductVersionMS);
            version->product_minor = LOWORD(info->dwProductVersionMS);
            version->product_build = HIWORD(info->dwProductVersionLS);
            version->product_revision = LOWORD(info->dwProductVersionLS);
            version->file_major = HIWORD(info->dwFileVersionMS);
            version->file_minor = LOWORD(info->dwFileVersionMS);
            version->file_build = HIWORD(info->dwFileVersionLS);
            version->file_revision = LOWORD(info->dwFileVersionLS);
            status = SIGAR_OK;
        }
        else {
            status = GetLastError();
        }
    }
    else {
        status = GetLastError();
    }

    if (infocb && (status == SIGAR_OK)) {
        struct {
            WORD lang;
            WORD code_page;
        } *trans;

        if (VerQueryValue(data, "\\VarFileInfo\\Translation",
                          &trans, &len))
        {
            int i;
            char buf[1024];
            void *ptr;

            for (i=0; string_file_info_keys[i]; i++) {
                char *key = string_file_info_keys[i];
                sprintf(buf, "\\StringFileInfo\\%04x%04x\\%s",
                        trans[0].lang, trans[0].code_page,
                        key);
                if (VerQueryValue(data, buf, &ptr, &len)) {
                    if (len == 0) {
                        continue;
                    }
                    infocb->env_getter(infocb->data,
                                       key, strlen(key),
                                       (char *)ptr, len);
                }
            }
        }
    }

    free(data);
    return status;
}
