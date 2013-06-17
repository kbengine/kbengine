/*
 * Copyright (c) 2004, 2006-2008 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2009 VMware, Inc.
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

/*
 * functions for getting info from the Process Environment Block
 */
#define UNICODE
#define _UNICODE

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_os.h"
#include <shellapi.h>

void dllmod_init_ntdll(sigar_t *sigar);

#define sigar_NtQueryInformationProcess \
    sigar->ntdll.query_proc_info.func

static int sigar_pbi_get(sigar_t *sigar, HANDLE proc, PEB *peb)
{
    int status;
    PROCESS_BASIC_INFORMATION pbi;
    DWORD size=sizeof(pbi);

    dllmod_init_ntdll(sigar);

    if (!sigar_NtQueryInformationProcess) {
        return SIGAR_ENOTIMPL;
    }

    SIGAR_ZERO(&pbi);
    status =
        sigar_NtQueryInformationProcess(proc,
                                        ProcessBasicInformation,
                                        &pbi,
                                        size, NULL);
    if (status != ERROR_SUCCESS) {
        return status;
    }

    if (!pbi.PebBaseAddress) {
        /* likely we are 32-bit, pid process is 64-bit */
        return ERROR_DATATYPE_MISMATCH;
    }

    size = sizeof(*peb);

    if (ReadProcessMemory(proc, pbi.PebBaseAddress, peb, size, NULL)) {
        return SIGAR_OK;
    }
    else {
        return GetLastError();
    }
}

static int sigar_rtl_get(sigar_t *sigar, HANDLE proc,
                         RTL_USER_PROCESS_PARAMETERS *rtl)
{
    PEB peb;
    int status = sigar_pbi_get(sigar, proc, &peb);
    DWORD size=sizeof(*rtl);

    if (status != SIGAR_OK) {
        return status;
    }

    if (ReadProcessMemory(proc, peb.ProcessParameters, rtl, size, NULL)) {
        return SIGAR_OK;
    }
    else {
        return GetLastError();
    }
}

#define rtl_bufsize(buf, uc) \
    ((sizeof(buf) < uc.Length) ? sizeof(buf) : uc.Length)

int sigar_proc_exe_peb_get(sigar_t *sigar, HANDLE proc,
                           sigar_proc_exe_t *procexe)
{
    int status;
    WCHAR buf[MAX_PATH+1];
    RTL_USER_PROCESS_PARAMETERS rtl;
    DWORD size;

    procexe->name[0] = '\0';
    procexe->cwd[0] = '\0';

    if ((status = sigar_rtl_get(sigar, proc, &rtl)) != SIGAR_OK) {
        return status;
    }

    size = rtl_bufsize(buf, rtl.ImagePathName);
    memset(buf, '\0', sizeof(buf));

    if ((size > 0) &&
        ReadProcessMemory(proc, rtl.ImagePathName.Buffer, buf, size, NULL))
    {
        SIGAR_W2A(buf, procexe->name, sizeof(procexe->name));
    }

    size = rtl_bufsize(buf, rtl.CurrentDirectoryName);
    memset(buf, '\0', sizeof(buf));

    if ((size > 0) &&
        ReadProcessMemory(proc, rtl.CurrentDirectoryName.Buffer, buf, size, NULL))
    {
        SIGAR_W2A(buf, procexe->cwd, sizeof(procexe->cwd));
    }

    return SIGAR_OK;
}

int sigar_parse_proc_args(sigar_t *sigar, WCHAR *buf,
                          sigar_proc_args_t *procargs)
{
    char arg[SIGAR_CMDLINE_MAX];
    LPWSTR *args;
    int num, i;

    if (!buf) {
        buf = GetCommandLine();
    }

    args = CommandLineToArgvW(buf, &num);

    if (args == NULL) {
        return SIGAR_OK;
    }

    for (i=0; i<num; i++) {
        SIGAR_W2A(args[i], arg, SIGAR_CMDLINE_MAX);
        SIGAR_PROC_ARGS_GROW(procargs);
        procargs->data[procargs->number++] = sigar_strdup(arg);
    }

    GlobalFree(args);

    return SIGAR_OK;
}

int sigar_proc_args_peb_get(sigar_t *sigar, HANDLE proc,
                            sigar_proc_args_t *procargs)
{
    int status;
    WCHAR buf[SIGAR_CMDLINE_MAX];
    RTL_USER_PROCESS_PARAMETERS rtl;
    DWORD size;

    if ((status = sigar_rtl_get(sigar, proc, &rtl)) != SIGAR_OK) {
        return status;
    }

    size = rtl_bufsize(buf, rtl.CommandLine);
    if (size <= 0) {
        return ERROR_DATATYPE_MISMATCH; /* fallback to wmi */
    }
    memset(buf, '\0', sizeof(buf));

    if (ReadProcessMemory(proc, rtl.CommandLine.Buffer, buf, size, NULL)) {
        return sigar_parse_proc_args(sigar, buf, procargs);
    }
    else {
        return GetLastError();
    }
}

int sigar_proc_env_peb_get(sigar_t *sigar, HANDLE proc,
                           WCHAR *buf, DWORD size)
{
    int status;
    RTL_USER_PROCESS_PARAMETERS rtl;
    MEMORY_BASIC_INFORMATION info;

    if ((status = sigar_rtl_get(sigar, proc, &rtl)) != SIGAR_OK) {
        return status;
    }

    memset(buf, '\0', size);
    /* -2 to ensure \0\0 terminator */
    size -= 2;

    if (VirtualQueryEx(proc, rtl.Environment, &info, sizeof(info))) {
        if (size > info.RegionSize) {
            /* ReadProcessMemory beyond region would fail */
            size = info.RegionSize;
        }
    }

    if (ReadProcessMemory(proc, rtl.Environment, buf, size, NULL)) {
        return SIGAR_OK;
    }
    else {
        return GetLastError();
    }
}
