/*
 * Copyright (c) 2004-2005, 2008 Hyperic, Inc.
 * Copyright (c) 2010 VMware, Inc.
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

#include "win32bindings.h"
#include "javasigar.h"
#include "sigar.h"
#include "sigar_private.h"
#include "sigar_os.h"
#include <shellapi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STRING_SIG "Ljava/lang/String;"
#define ASTRING_SIG "[" STRING_SIG

#define SERVICE_SetStringField(field, str) \
    id = env->GetFieldID(cls, field, STRING_SIG); \
    value = env->NewString((const jchar *)str, lstrlen(str)); \
    env->SetObjectField(obj, id, value)

#define SERVICE_SetIntField(field, val) \
    id = env->GetFieldID(cls, field, "I"); \
    env->SetIntField(obj, id, val)

typedef DWORD (CALLBACK *ChangeServiceConfig2_func_t)(SC_HANDLE,
                                                      DWORD, LPVOID);

typedef DWORD (CALLBACK *QueryServiceConfig2_func_t)(SC_HANDLE,
                                                     DWORD,
                                                     LPSERVICE_DESCRIPTION,
                                                     DWORD,
                                                     LPDWORD);

JNIEXPORT jboolean SIGAR_JNI(win32_Service_ChangeServiceDescription)
(JNIEnv *env, jclass, jlong handle, jstring description)
{
    jboolean result = FALSE;
    SERVICE_DESCRIPTION desc;
    HINSTANCE lib;
    ChangeServiceConfig2_func_t change_config;
    
    if ((lib = LoadLibrary(L"advapi32"))) {
        change_config = (ChangeServiceConfig2_func_t)
            GetProcAddress(lib, "ChangeServiceConfig2W");

        if (change_config) {
            desc.lpDescription =
                (LPTSTR)env->GetStringChars(description, NULL);

            result = change_config((SC_HANDLE)handle, 
                                   SERVICE_CONFIG_DESCRIPTION, &desc);
            env->ReleaseStringChars(description, 
                                    (const jchar *)desc.lpDescription);
        }

        FreeLibrary(lib);
    }

    return result;
}

JNIEXPORT jboolean SIGAR_JNI(win32_Service_CloseServiceHandle)
(JNIEnv *, jclass, jlong handle)
{
    return CloseServiceHandle((SC_HANDLE)handle);
}

JNIEXPORT void SIGAR_JNI(win32_Service_ControlService)
(JNIEnv *env, jclass, jlong handle, jint control)
{
    BOOL retval;
    SERVICE_STATUS status;

    if (control == 0) {
        retval = StartService((SC_HANDLE)handle, 0, NULL);
    }
    else {
        retval = ControlService((SC_HANDLE)handle, control, &status);
    }

    if (!retval) {
        win32_throw_last_error(env);
    }
}

JNIEXPORT jlong SIGAR_JNI(win32_Service_CreateService)
(JNIEnv *env, jclass, jlong handle,
 jstring j_name, jstring j_display, jint type,
 jint startType, jint errorControl, jstring j_path,
 jobjectArray dependencies,
 jstring j_startName, 
 jstring j_password)
{
    TCHAR buffer[8192];
    LPCTSTR depend = NULL;
    jlong   result;
    LPCTSTR startName;
    LPCTSTR name     = (LPCTSTR)env->GetStringChars(j_name, NULL);
    LPCTSTR display  = (LPCTSTR)env->GetStringChars(j_display, NULL);
    LPCTSTR path     = (LPCTSTR)env->GetStringChars(j_path, NULL);
    LPCTSTR password = (LPCTSTR)env->GetStringChars(j_password, NULL);

    if (j_startName != NULL) {
        startName = (LPCTSTR)env->GetStringChars(j_startName, NULL);
    }
    else {
        startName = NULL;
    }

    if (dependencies != NULL) {
        // Build a buffer of a double null terminated array of 
        // null terminated service names
        LPTSTR ptr = buffer;
        jsize alen = env->GetArrayLength(dependencies);
        depend = buffer;

        for (int i=0; i<alen; i++) {
            jstring str = (jstring)env->GetObjectArrayElement(dependencies, i);
            LPCTSTR chars = (LPCTSTR)env->GetStringChars(str, NULL);
            size_t len = lstrlen(chars);

            // If we're going to overrun the buffer then break out of the loop
            if ((ptr + len + 1) >=
                (buffer + sizeof(buffer) / sizeof(TCHAR)))
            {
                break;
            }

            lstrcpy(ptr, chars);
            env->ReleaseStringChars(str, (const jchar *)chars);

            // Move the buffer to the byte beyond the current string 
            // null terminator
            ptr = ptr + len + 1;
        }

        *ptr = 0;  // Double null terminate the string
    }

    // Create the Service
    result = (jlong)CreateService((SC_HANDLE)handle, name,
                                  display, SERVICE_ALL_ACCESS,
                                  type,
                                  startType, errorControl, path, 
                                  NULL, NULL, depend, startName,
                                  password);

    if (result == 0) {
        win32_throw_last_error(env);
    }

    if (startName != NULL) {
        env->ReleaseStringChars(j_startName, (const jchar *)startName);
    }
    env->ReleaseStringChars(j_password, (const jchar *)password);
    env->ReleaseStringChars(j_path, (const jchar *)path);
    env->ReleaseStringChars(j_display, (const jchar *)display);
    env->ReleaseStringChars(j_name, (const jchar *)name);

    return result;
}

JNIEXPORT void SIGAR_JNI(win32_Service_DeleteService)
(JNIEnv *env, jclass, jlong handle)
{
    if (!DeleteService((SC_HANDLE)handle)) {
        win32_throw_last_error(env);
    }
}

JNIEXPORT jlong SIGAR_JNI(win32_Service_OpenSCManager)
(JNIEnv *env, jclass, jstring jmachine, jint access)
{
    LPCTSTR machine = (LPCTSTR)env->GetStringChars(jmachine, NULL);
    jlong result = (jlong)OpenSCManager(machine, NULL, access);

    if (!result) {
        win32_throw_last_error(env);
    }

    env->ReleaseStringChars(jmachine, (const jchar *)machine);

    return result;
}

JNIEXPORT jlong SIGAR_JNI(win32_Service_OpenService)
(JNIEnv *env, jclass, jlong handle,
 jstring jservice, jint access)
{
    LPCTSTR service = (LPCTSTR)env->GetStringChars(jservice, NULL);
    jlong result =
        (jlong)OpenService((SC_HANDLE)handle, service, access);

    if (!result) {
        win32_throw_last_error(env);
    }

    env->ReleaseStringChars(jservice, (const jchar *)service);

    return result;
}

JNIEXPORT jint
SIGAR_JNI(win32_Service_QueryServiceStatus)
(JNIEnv *, jclass, jlong handle)
{
    SERVICE_STATUS status;
    int result;

    if (QueryServiceStatus((SC_HANDLE)handle, &status) == TRUE) {
        result = status.dwCurrentState;
    }
    else {
        result = -1;
    }

    return result;
}

static int jsigar_add_service(sigar_services_walker_t *walker, char *name)
{
    return jsigar_list_add(walker->data, name, -1);
}

JNIEXPORT jobject SIGAR_JNI(win32_Service_getServiceNames)
(JNIEnv *env, jclass, jobject sigar_obj, jstring jptql)
{
    DWORD status;
    jsigar_list_t obj;
    sigar_t *sigar = NULL;
    char *ptql = NULL;
    sigar_ptql_error_t error;
    sigar_services_walker_t walker;
    jboolean is_copy;

    if (sigar_obj) {
        if (!(sigar = jsigar_get_sigar(env, sigar_obj))) {
            return NULL;
        }
    }
    if (jptql) {
        ptql = (char *)env->GetStringUTFChars(jptql, &is_copy);
    }

    walker.sigar = sigar;
    walker.flags = SERVICE_STATE_ALL;
    walker.data = &obj;
    walker.add_service = jsigar_add_service;

    jsigar_list_init(env, &obj);

    status = sigar_services_query(ptql, &error, &walker);

    if (ptql && is_copy) {
        env->ReleaseStringUTFChars(jptql, ptql);
    }

    if (status != SIGAR_OK) {
        env->DeleteLocalRef(obj.obj);
        if (status == SIGAR_PTQL_MALFORMED_QUERY) {
            win32_throw_exception(env, error.message);
        }
        else {
            win32_throw_error(env, status);
        }
        return NULL;
    }

    return obj.obj;
}

/*
 * convert:
 *   "RPCSS\0Tcpip\0IPSec\0\0"
 * to:
 *   ["RPCSS", "Tcpip", "IPSec"]
 */
static int to_array(JNIEnv *env, LPTSTR str, jobjectArray array)
{
    TCHAR *ptr = &str[0];
    int offset=0, i=0;

    while (*ptr != 0) {
        int slen = _tcslen(ptr);
        if (array) {
            jstring jstr =
                env->NewString((const jchar *)ptr, slen);
            env->SetObjectArrayElement(array, i, jstr);
            if (env->ExceptionCheck()) {
                return -1;
            }
        }
        offset += slen + 1;
        ptr = &str[offset];
        i++;
    }

    return i;
}

JNIEXPORT jboolean SIGAR_JNI(win32_Service_QueryServiceConfig)
(JNIEnv *env, jclass, jlong handle, jobject obj)
{
    char buffer[8192]; /* 8k is max size from mdsn docs */
    char exe[SIGAR_CMDLINE_MAX], *ptr;
    LPQUERY_SERVICE_CONFIG config = (LPQUERY_SERVICE_CONFIG)buffer;
    DWORD bytes;
    jfieldID id;
    jclass cls = env->GetObjectClass(obj);
    jstring value;
    HINSTANCE lib;
    LPWSTR *argv;
    int argc;
    jclass stringclass =
        env->FindClass("java/lang/String");

    if (!QueryServiceConfig((SC_HANDLE)handle, config,
                            sizeof(buffer), &bytes))
    {
        win32_throw_last_error(env);
        return JNI_FALSE;
    }

    SERVICE_SetIntField("type", config->dwServiceType);

    SERVICE_SetIntField("startType", config->dwStartType);

    SERVICE_SetIntField("errorControl", config->dwErrorControl);

    SERVICE_SetStringField("path", config->lpBinaryPathName);

    if ((argv = CommandLineToArgvW(config->lpBinaryPathName, &argc))) {
        int i;
        jobjectArray jargv =
            env->NewObjectArray(argc, stringclass, 0);
        if (env->ExceptionCheck()) {
            LocalFree(argv);
            return JNI_FALSE;
        }

        for (i=0; i<argc; i++) {
            jstring jstr =
                env->NewString((const jchar *)argv[i], lstrlen(argv[i]));
            env->SetObjectArrayElement(jargv, i, jstr);
            if (env->ExceptionCheck()) {
                LocalFree(argv);
                return JNI_FALSE;
            }
        }

        id = env->GetFieldID(cls, "argv", ASTRING_SIG);

        env->SetObjectField(obj, id, jargv);
        LocalFree(argv);
    }

    SERVICE_SetStringField("loadOrderGroup", config->lpLoadOrderGroup);

    SERVICE_SetIntField("tagId", config->dwTagId);

    if (config->lpDependencies) {
        /* first pass just get num for NewObjectArray */
        jobjectArray dependencies;
        int num = to_array(env, config->lpDependencies, NULL);

        if (num < 0) {
            return JNI_FALSE; /* Exception thrown */
        }

        dependencies = env->NewObjectArray(num, stringclass, 0);
        if (env->ExceptionCheck()) {
            return JNI_FALSE;
        }

        to_array(env, config->lpDependencies, dependencies);
        if (num < 0) {
            return JNI_FALSE; /* Exception thrown */
        }

        id = env->GetFieldID(cls, "dependencies", ASTRING_SIG);

        env->SetObjectField(obj, id, dependencies);
    }

    SERVICE_SetStringField("startName", config->lpServiceStartName);

    SERVICE_SetStringField("displayName", config->lpDisplayName);

    if ((lib = LoadLibrary(L"advapi32"))) {
        LPSERVICE_DESCRIPTION desc = 
            (LPSERVICE_DESCRIPTION)buffer;
        QueryServiceConfig2_func_t query_config =
            (QueryServiceConfig2_func_t)
                GetProcAddress(lib, "QueryServiceConfig2W");

        if (query_config) {
            BOOL retval =
                query_config((SC_HANDLE)handle, 
                             SERVICE_CONFIG_DESCRIPTION,
                             desc, sizeof(buffer), &bytes);
            if (retval && (desc->lpDescription != NULL)) {
                SERVICE_SetStringField("description",
                                       desc->lpDescription);
            }
        }

        FreeLibrary(lib);
    }

    return JNI_TRUE;
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
