/*
 * Copyright (c) 2004, 2007-2008 Hyperic, Inc.
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

#ifdef WIN32
#define UNICODE
#define _UNICODE

#include <stdlib.h>

#include "win32bindings.h"
#include "javasigar.h"

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL SIGAR_JNI(win32_RegistryKey_RegCloseKey)
(JNIEnv *, jclass, jlong hkey)
{
    return RegCloseKey((HKEY)hkey);
}

JNIEXPORT jlong JNICALL SIGAR_JNI(win32_RegistryKey_RegCreateKey)
(JNIEnv *env, jclass, jlong hkey, jstring subkey)
{
    HKEY    hkeyResult = NULL;
    LPCTSTR lpSubkey = (LPCTSTR)env->GetStringChars(subkey, NULL);
    
    RegCreateKeyEx((HKEY)hkey, lpSubkey, 0, NULL, 
                   REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
                   NULL, &hkeyResult, NULL);

    env->ReleaseStringChars(subkey, (const jchar *)lpSubkey);

    return (jlong)hkeyResult;
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegDeleteKey)
(JNIEnv *env, jclass, jlong hkey, jstring subkey)
{
    LPCTSTR lpSubkey = (LPCTSTR)env->GetStringChars(subkey, NULL);
    LONG    lResult  = RegDeleteKey((HKEY)hkey, lpSubkey);
    env->ReleaseStringChars(subkey, (const jchar *)lpSubkey);

    return lResult;
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegDeleteValue)
(JNIEnv *env, jclass, jlong hkey, jstring valueName)
{
    LPCTSTR lpValueName = (LPCTSTR)env->GetStringChars(valueName, NULL);
    LONG    lResult     = RegDeleteValue((HKEY)hkey, lpValueName);
    env->ReleaseStringChars(valueName, (const jchar *)lpValueName);

    return lResult;
}

JNIEXPORT jstring SIGAR_JNI(win32_RegistryKey_RegEnumKey)
(JNIEnv *env, jclass, jlong hkey, jint index)
{
    jstring strResult;
    TCHAR   szBuffer[MAX_PATH + 1];
    DWORD len = sizeof(szBuffer) / sizeof(TCHAR);

    if(RegEnumKey((HKEY)hkey, index, szBuffer, 
                  len) == ERROR_SUCCESS)
        strResult = env->NewString((const jchar *)szBuffer, 
                                   lstrlen(szBuffer));
    else
        strResult = NULL;

    return strResult;
}

JNIEXPORT jstring SIGAR_JNI(win32_RegistryKey_RegEnumValueName)
(JNIEnv *env, jclass, jlong hkey, jint index)
{
    jstring strResult;
    TCHAR   szValueName[MAX_PATH + 1];
    DWORD   cbValueName = sizeof(szValueName) / sizeof(TCHAR);

    if(RegEnumValue((HKEY)hkey, index, szValueName, 
                    &cbValueName, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
        strResult = env->NewString((const jchar *)szValueName, 
                                   lstrlen(szValueName));
    else
        strResult = NULL;

    return strResult;
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegFlushKey)
(JNIEnv *env, jclass, long hkey)
{
    return RegFlushKey((HKEY)hkey);
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegLoadKey)
(JNIEnv *env, jclass, jlong hkey, jstring subkey, jstring file)
{
    LPCTSTR lpSubkey = (LPCTSTR)env->GetStringChars(subkey, NULL);
    LPCTSTR lpFile   = (LPCTSTR)env->GetStringChars(file, NULL);

    LONG lResult = RegLoadKey((HKEY)hkey, lpSubkey, lpFile);

    env->ReleaseStringChars(subkey, (const jchar *)lpSubkey);
    env->ReleaseStringChars(file, (const jchar *)lpFile);

    return lResult;
}

/* http://msdn.microsoft.com/en-us/library/aa384129(VS.85).aspx */
#ifndef KEY_WOW64_64KEY
#define KEY_WOW64_64KEY 0x0100
#endif
#ifndef KEY_WOW64_32KEY
#define KEY_WOW64_32KEY 0x0200
#endif

static DWORD RegOpenOpts[] = {
    0,
    KEY_WOW64_32KEY,
    KEY_WOW64_64KEY
};

JNIEXPORT jlong SIGAR_JNI(win32_RegistryKey_RegOpenKey)
(JNIEnv *env, jclass, jlong hkey, jstring subkey)
{
    HKEY    hkeyResult = NULL;
    jsize len = env->GetStringLength(subkey);
    LPTSTR lpSubkey = (LPTSTR)env->GetStringChars(subkey, NULL);
    LPTSTR copy;
    int i;

    /* required under IBM/WebSphere 4.0 for certain keys */
    if (lpSubkey[len] != '\0') {
        copy = _wcsdup(lpSubkey);
        copy[len] = '\0';
    }
    else {
        copy = lpSubkey;
    }

    /* try both flags so 32-bit apps can access registry entries
       of 64-bit apps and vice-versa */
    for (i=0; i<sizeof(RegOpenOpts)/sizeof(RegOpenOpts[0]); i++) {
        DWORD status =
            RegOpenKeyEx((HKEY)hkey, copy,
                         0, KEY_READ|RegOpenOpts[i],
                         &hkeyResult);
        if (status == ERROR_SUCCESS) {
            break;
        }
    }

    env->ReleaseStringChars(subkey, (const jchar *)lpSubkey);
    if (copy != lpSubkey) {
        free(copy);
    }
    return (jlong)hkeyResult;
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegQueryIntValue)
(JNIEnv *env, jclass, jlong hkey, jstring valueName)
{
    DWORD   dwResult;
    DWORD   dwType;
    LPBYTE  lpValue;
    DWORD   cbValue;
    LPTSTR copy;
    jsize len = env->GetStringLength(valueName);
    LPTSTR lpValueName = (LPTSTR)env->GetStringChars(valueName, NULL);
    LONG    lErr;
    /* required under IBM/WebSphere 4.0 for certain keys */
    if (lpValueName[len] != '\0') {
        copy = _wcsdup(lpValueName);
        copy[len] = '\0';
    }
    else {
        copy = lpValueName;
    }

    lErr = RegQueryValueEx((HKEY)hkey, copy, 
                           NULL, (LPDWORD)&dwType, 
                           NULL, &cbValue);
    if(lErr == ERROR_SUCCESS) {
        lpValue = (LPBYTE)HeapAlloc(GetProcessHeap(), 
                                    HEAP_ZERO_MEMORY, cbValue);

        if(RegQueryValueEx((HKEY)hkey, copy, NULL, 
                           NULL, lpValue, &cbValue) == ERROR_SUCCESS)
        {
            switch(dwType) {
            case REG_DWORD:
                dwResult = *(LPDWORD)lpValue;
                break;
            case REG_SZ:
                dwResult = _ttol((LPCTSTR)lpValue);
                break;
            default:
                lErr = ERROR_SUCCESS - 1; // Make an error
            }
        }

        HeapFree(GetProcessHeap(), 0, lpValue);
    }
    else
        // Make an error out of not seeing a REG_DWORD
        lErr = ERROR_SUCCESS - 1;

    env->ReleaseStringChars(valueName, (const jchar *)lpValueName);
    if (copy != lpValueName) {
        free(copy);
    }
    
    if(lErr != ERROR_SUCCESS)
    {
        jclass cls = 
            env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, NULL);
    }

    return dwResult;
}

JNIEXPORT jstring SIGAR_JNI(win32_RegistryKey_RegQueryStringValue)
(JNIEnv *env, jclass, jlong hkey, jstring name)
{
    jstring strResult;
    DWORD   dwType;
    LPBYTE  lpValue;
    DWORD   cbValue;
    jsize len = env->GetStringLength(name);
    LPTSTR lpValueName = (LPTSTR)env->GetStringChars(name, NULL);
    LPTSTR copy;
    LONG    lErr;
    /* required under IBM/WebSphere 4.0 for certain keys */
    if (lpValueName[len] != '\0') {
        copy = _wcsdup(lpValueName);
        copy[len] = '\0';
    }
    else {
        copy = lpValueName;
    }

    lErr = RegQueryValueEx((HKEY)hkey, 
                           copy, NULL, 
                           (LPDWORD)&dwType, NULL, &cbValue);
    if(lErr == ERROR_SUCCESS)
    {
        lpValue = (LPBYTE)HeapAlloc(GetProcessHeap(), 
                                    HEAP_ZERO_MEMORY, cbValue);

        if(RegQueryValueEx((HKEY)hkey, copy, NULL, NULL, 
                           lpValue, &cbValue) == ERROR_SUCCESS)
        {
            switch(dwType) {
            case REG_DWORD:
                TCHAR   szBuf[20];
                _ltot(*(LPDWORD)lpValue, szBuf, 10);
                strResult = env->NewString((const jchar *)szBuf, 
                                           lstrlen(szBuf));
                break;
            case REG_SZ:
            case REG_EXPAND_SZ: {
                DWORD len;
                LPTSTR dest = NULL;
                len = ExpandEnvironmentStrings((LPCTSTR)lpValue, dest, 0);
                dest = (LPTSTR)malloc(len * sizeof(TCHAR));
                ExpandEnvironmentStrings((LPCTSTR)lpValue, dest, len);
                strResult = env->NewString((const jchar *)dest, len);
                free(dest);
                break;
            }
            default:
                lErr = ERROR_SUCCESS - 1; // Make an error
            }
        }

        HeapFree(GetProcessHeap(), 0, lpValue);
    }

    env->ReleaseStringChars(name, (const jchar *)lpValueName);
    if (copy != lpValueName) {
        free(copy);
    }
    
    if(lErr == ERROR_SUCCESS)
        return strResult;
    else
    {
        jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, "");
        return NULL;
    }
}

JNIEXPORT void SIGAR_JNI(win32_RegistryKey_RegQueryMultiStringValue)
(JNIEnv *env, jclass, jlong hkey, jstring jname, jobject obj)
{
    LONG rc;
    DWORD type, size;
    LPBYTE value;
    LPTSTR name =
        (LPTSTR)env->GetStringChars(jname, NULL);
    jclass cls =
        env->GetObjectClass(obj);
    jmethodID id =
        env->GetMethodID(cls, "add",
                         "(Ljava/lang/Object;)"
                         "Z");

    rc = RegQueryValueEx((HKEY)hkey, 
                         name, NULL, 
                         &type, NULL, &size);

    if (type != REG_MULTI_SZ) {
        rc = ERROR_SUCCESS - 1;
    }

    if (rc == ERROR_SUCCESS) {
        value =
            (LPBYTE)HeapAlloc(GetProcessHeap(), 
                              HEAP_ZERO_MEMORY, size);

        if (RegQueryValueEx((HKEY)hkey, name, NULL, NULL, 
                            value, &size) == ERROR_SUCCESS)
        {
            PTSTR ptr = (PTSTR)value;

            while (*ptr) {
                int len = _tcslen(ptr);
                jstring jval =
                    env->NewString((const jchar *)ptr, len);

                env->CallBooleanMethod(obj, id, jval);
                if (env->ExceptionCheck()) {
                    HeapFree(GetProcessHeap(), 0, value);
                    return;
                }
                ptr += len + 1;
            }
        }

        HeapFree(GetProcessHeap(), 0, value);
    }

    env->ReleaseStringChars(jname, (const jchar *)name);
    
    if (rc != ERROR_SUCCESS) {
        jclass cls =
            env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, "");
    }
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegSetIntValue)
(JNIEnv * env, jclass, jlong hkey, jstring valueName, jint value)
{
    LPCTSTR lpValueName;
    
    if(valueName != NULL)
        lpValueName = (LPCTSTR)env->GetStringChars(valueName, NULL);
    else
        lpValueName = NULL;

    int iResult = RegSetValueEx((HKEY)hkey, lpValueName, 0, 
                                REG_DWORD, (LPBYTE)&value, sizeof(value));

    if(valueName != NULL)
        env->ReleaseStringChars(valueName, (const jchar *)lpValueName);

    return iResult;
}

JNIEXPORT jint SIGAR_JNI(win32_RegistryKey_RegSetStringValue)
(JNIEnv *env, jclass, jlong hkey, jstring name, jstring value)
{
    LPCTSTR lpValueName;

    if(name != NULL)
        lpValueName = (LPCTSTR)env->GetStringChars(name, NULL);
    else
        lpValueName = NULL;

    LPCTSTR lpValue = (LPCTSTR)env->GetStringChars(value, NULL);

    int iResult = RegSetValueEx((HKEY)hkey, lpValueName, 0, 
                                REG_SZ, (LPBYTE)lpValue, 
                                (lstrlen(lpValue) + 1) * sizeof(TCHAR));
    
    if(name != NULL)
        env->ReleaseStringChars(name, (const jchar *)lpValueName);
    env->ReleaseStringChars(value, (const jchar *)lpValue);

    return iResult;
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
