/*
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

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define _WIN32_DCOM

#include <windows.h>
#include <objbase.h>
#include <comdef.h>
#include <wbemidl.h>
#include "sigar.h"

#pragma comment(lib, "wbemuuid.lib")

#ifndef SIGAR_CMDLINE_MAX
#define SIGAR_CMDLINE_MAX 4096
#endif

class WMI {

  public:
    WMI();
    ~WMI();
    HRESULT Open(LPCTSTR machine=NULL, LPCTSTR user=NULL, LPCTSTR pass=NULL);
    void Close();
    HRESULT GetProcStringProperty(DWORD pid, TCHAR *name, TCHAR *value, DWORD len);
    HRESULT GetProcExecutablePath(DWORD pid, TCHAR *value);
    HRESULT GetProcCommandLine(DWORD pid, TCHAR *value);
    int GetLastError();

  private:
    IWbemServices *wbem;
    HRESULT result;
    BSTR GetProcQuery(DWORD pid);
};

WMI::WMI()
{
    wbem = NULL;
    result = S_OK;
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
}

WMI::~WMI()
{
    Close();
    CoUninitialize();
}

/* XXX must be a better way to map HRESULT */
int WMI::GetLastError()
{
    switch (result) {
      case S_OK:
        return ERROR_SUCCESS;
      case WBEM_E_NOT_FOUND:
        return ERROR_NOT_FOUND;
      case WBEM_E_ACCESS_DENIED:
        return ERROR_ACCESS_DENIED;
      case WBEM_E_NOT_SUPPORTED:
        return SIGAR_ENOTIMPL;
      default:
        return ERROR_INVALID_FUNCTION;
    }
}

HRESULT WMI::Open(LPCTSTR machine, LPCTSTR user, LPCTSTR pass)
{
    IWbemLocator *locator;
    wchar_t path[MAX_PATH];

    if (wbem) {
        result = S_OK;
        return result;
    }

    result =
        CoInitializeSecurity(NULL,                        //Security Descriptor
                             -1,                          //COM authentication
                             NULL,                        //Authentication services
                             NULL,                        //Reserved
                             RPC_C_AUTHN_LEVEL_DEFAULT,   //Default authentication
                             RPC_C_IMP_LEVEL_IMPERSONATE, //Default Impersonation
                             NULL,                        //Authentication info
                             EOAC_NONE,                   //Additional capabilities
                             NULL);                       //Reserved

    result = CoCreateInstance(CLSID_WbemLocator,
                              NULL, /* IUnknown */
                              CLSCTX_INPROC_SERVER,
                              IID_IWbemLocator,
                              (LPVOID *)&locator);

    if (FAILED(result)) {
        return result;
    }

    if (machine == NULL) {
        machine = L".";
    }

    wsprintf(path, L"\\\\%S\\ROOT\\CIMV2", machine);

    result = locator->ConnectServer(bstr_t(path), //Object path of WMI namespace
                                    bstr_t(user), //User name. NULL = current user
                                    bstr_t(pass), //User password. NULL = current
                                    NULL,         //Locale. NULL indicates current
                                    0,            //Security flags
                                    NULL,         //Authority (e.g. Kerberos)
                                    NULL,         //Context object
                                    &wbem);       //pointer to IWbemServices proxy

    locator->Release();

    return result;
}

void WMI::Close()
{
    if (wbem) {
        wbem->Release();
        wbem = NULL;
        result = S_OK;
    }
}

BSTR WMI::GetProcQuery(DWORD pid)
{
    wchar_t query[56];
    wsprintf(query, L"Win32_Process.Handle=%d", pid);
    return bstr_t(query);
}

HRESULT WMI::GetProcStringProperty(DWORD pid, TCHAR *name, TCHAR *value, DWORD len)
{
    IWbemClassObject *obj;
    VARIANT var;

    result = wbem->GetObject(GetProcQuery(pid), 0, 0, &obj, 0);

    if (FAILED(result)) {
        return result;
    }

    result = obj->Get(name, 0, &var, 0, 0);

    if (SUCCEEDED(result)) {
        if (var.vt == VT_NULL) {
            result = E_INVALIDARG;
        }
        else {
            lstrcpyn(value, var.bstrVal, len);
        }
        VariantClear(&var);
    }

    obj->Release();

    return result;
}

HRESULT WMI::GetProcExecutablePath(DWORD pid, TCHAR *value)
{
    return GetProcStringProperty(pid, L"ExecutablePath", value, MAX_PATH);
}

HRESULT WMI::GetProcCommandLine(DWORD pid, TCHAR *value)
{
    return GetProcStringProperty(pid, L"CommandLine", value, SIGAR_CMDLINE_MAX);
}

/* in peb.c */
extern "C" int sigar_parse_proc_args(sigar_t *sigar, WCHAR *buf,
                                     sigar_proc_args_t *procargs);

extern "C" int sigar_proc_args_wmi_get(sigar_t *sigar, sigar_pid_t pid,
                                       sigar_proc_args_t *procargs)
{
    int status;
    TCHAR buf[SIGAR_CMDLINE_MAX];
    WMI *wmi = new WMI();

    if (FAILED(wmi->Open())) {
        return wmi->GetLastError();
    }

    if (FAILED(wmi->GetProcCommandLine((DWORD)pid, buf))) {
        status = wmi->GetLastError();
    }
    else {
        status = sigar_parse_proc_args(sigar, buf, procargs);
    }

    wmi->Close();
    delete wmi;

    return status;
}

extern "C" int sigar_proc_exe_wmi_get(sigar_t *sigar, sigar_pid_t pid,
                                      sigar_proc_exe_t *procexe)
{
    int status;
    TCHAR buf[MAX_PATH+1];
    WMI *wmi = new WMI();

    if (FAILED(wmi->Open())) {
        return wmi->GetLastError();
    }

    procexe->name[0] = '\0';

    if (FAILED(wmi->GetProcExecutablePath((DWORD)pid, buf))) {
        status = wmi->GetLastError();
    }
    else {
        status = SIGAR_OK;
        /* SIGAR_W2A(buf, procexe->name, sizeof(procexe->name)); */
        WideCharToMultiByte(CP_ACP, 0, buf, -1,
                            (LPSTR)procexe->name, sizeof(procexe->name),
                            NULL, NULL);
    }

    wmi->Close();
    delete wmi;

    return status;
}
