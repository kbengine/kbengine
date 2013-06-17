/*
 * Copyright (c) 2004-2006, 2008 Hyperic, Inc.
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

#if defined(WIN32) && !defined(SIGAR_NO_ATL)
#define UNICODE
#define _UNICODE
#define _WIN32_DCOM

#define INITGUID

#include <objbase.h>  // COM Interface header file. 
#include <iadmw.h>    // COM Interface header file. 
#include <iiscnfg.h>  // MD_ & IIS_MD_ #defines header file.
#include <ks.h>
extern const CLSID CLSID_StdGlobalInterfaceTable;
#include <atlBase.h>  // ATL support header file.

#include "win32bindings.h"
#include "javasigar.h"

#ifdef __cplusplus
extern "C" {
#endif

static jfieldID ptr_field = 0;
static jfieldID IMeta_field = 0;

JNIEXPORT void SIGAR_JNI(win32_MetaBase_MetaBaseClose)
(JNIEnv *env, jobject cur)
{
    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    METADATA_HANDLE MyHandle; 
    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);
    (*pIMeta)->CloseKey(MyHandle);
}

JNIEXPORT void SIGAR_JNI(win32_MetaBase_MetaBaseRelease)
(JNIEnv *env, jobject cur)
{
    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    pIMeta->Release();
    delete pIMeta;
    env->SetIntField(cur, ptr_field, 0);
    CoUninitialize();
}

JNIEXPORT jstring SIGAR_JNI(win32_MetaBase_MetaBaseEnumKey)
(JNIEnv *env, jobject cur, jint index)
{ 
    HRESULT hRes = 0;
    METADATA_HANDLE MyHandle; 
    DWORD dwBufLen = 8096; 
    DWORD dwReqBufLen = 0; 
    TCHAR pbBuffer[METADATA_MAX_NAME_LEN]; 

    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);

    hRes = (*pIMeta)->EnumKeys(MyHandle, TEXT(""), pbBuffer, index);  
    if (SUCCEEDED(hRes)) { 
        jstring strResult;
        // Store the data identifiers in an array for future use. 
        // Note: declare a suitable DWORD array for names and add 
        // array bounds checking. 
        strResult = env->NewString((const jchar *)pbBuffer, lstrlen(pbBuffer));
        return strResult;
    } else if (hRes == HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS)) {
        return NULL;
    }

    jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
    env->ThrowNew(cls, "MetaBaseEnumKey");
    return NULL;
}

JNIEXPORT jlong SIGAR_JNI(win32_MetaBase_MetaBaseInit)
(JNIEnv *env, jobject cur)
{
    CComPtr <IMSAdminBase> *pIMeta;
    HRESULT hRes = 0;
    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hRes)) {
        jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, "MetaBaseInit");
        return 0;
    }

    pIMeta = new (CComPtr <IMSAdminBase>);
    hRes = CoCreateInstance(CLSID_MSAdminBase, NULL, CLSCTX_ALL, 
                            IID_IMSAdminBase, (void **)pIMeta);  
    if (FAILED(hRes)) {
        jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, "MetaBaseInit:  Can't CoCreateInstance");
        return 0;
    }
    
    jclass cls = env->GetObjectClass(cur);
    ptr_field = env->GetFieldID(cls, "m_handle", "I");
    IMeta_field = env->GetFieldID(cls, "pIMeta", "J");
    return (jlong)pIMeta;
}

static void MetaBaseOpenSubKey(JNIEnv *env, jobject cur, 
                               jstring path, METADATA_HANDLE MyHandle)
{
    HRESULT hRes = 0;
    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    // Get a handle to the Web service. 
    LPCTSTR lpSubkey = (LPCTSTR)env->GetStringChars(path, NULL);
    hRes = (*pIMeta)->OpenKey(MyHandle, lpSubkey, 
                              METADATA_PERMISSION_READ, 20, &MyHandle); 
    env->ReleaseStringChars(path, (const jchar *)lpSubkey);

    if (FAILED(hRes)) {
        jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
        env->ThrowNew(cls, "Can't open Sub Key");
        return;
    }
    env->SetIntField(cur, ptr_field, MyHandle);
}

JNIEXPORT void SIGAR_JNI(win32_MetaBase_MetaBaseOpenSubKey)
(JNIEnv *env, jobject cur, jstring path)
{
    METADATA_HANDLE MyHandle; 
    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);
    MetaBaseOpenSubKey(env, cur, path, MyHandle);
}

JNIEXPORT void SIGAR_JNI(win32_MetaBase_MetaBaseOpenSubKeyAbs)
(JNIEnv *env, jobject cur, jstring path)
{
    MetaBaseOpenSubKey(env, cur, path, METADATA_MASTER_ROOT_HANDLE);
}

JNIEXPORT jint SIGAR_JNI(win32_MetaBase_MetaBaseGetIntValue)
(JNIEnv *env, jobject cur, jint key)
{
    HRESULT hRes = 0;
    METADATA_HANDLE MyHandle; 
    METADATA_RECORD MyRecord; 
    DWORD dwBufLen = 8096; 
    DWORD dwReqBufLen = 0; 
    TCHAR pbBuffer[8096]; 

    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);

    // Initialize the input structure - 
    // the values specify what kind of data to enumerate. 
    MyRecord.dwMDIdentifier = key;
    MyRecord.dwMDAttributes = 0; 
    MyRecord.dwMDUserType = IIS_MD_UT_SERVER; 
    MyRecord.dwMDDataType = DWORD_METADATA; 
    MyRecord.dwMDDataLen = dwBufLen; 
    MyRecord.pbMDData = (unsigned char *)pbBuffer; 
 
    // Enumerate the data of the first virtual Web server, 
    // checking to ensure that the data returned does not 
    // overflow the buffer. 
 
    hRes = (*pIMeta)->GetData(MyHandle, TEXT(""), &MyRecord, &dwReqBufLen); 
    if (SUCCEEDED(hRes)) { 
        int ret = (int)MyRecord.pbMDData;
        return ret;
    }
    jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
    env->ThrowNew(cls, "No such Int value");
    return 0;
}

JNIEXPORT jstring SIGAR_JNI(win32_MetaBase_MetaBaseGetStringValue)
(JNIEnv *env, jobject cur, jint key)
{
    int i;
    HRESULT hRes = 0;
    METADATA_HANDLE MyHandle; 
    METADATA_RECORD MyRecord; 
    DWORD dwBufLen = 8096; 
    DWORD dwReqBufLen = 0; 
    TCHAR pbBuffer[8096]; 
    DWORD data_types[] = {
        STRING_METADATA,
        EXPANDSZ_METADATA /* e.g. MD_LOGFILEDIRECTORY */
    };

    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);

    for (i=0; i<sizeof(data_types)/sizeof(data_types[0]); i++) {
        DWORD dtype = data_types[i];
        // Initialize the input structure - 
        // the values specify what kind of data to enumerate. 
        MyRecord.dwMDIdentifier = key;
        MyRecord.dwMDAttributes = 0; 
        MyRecord.dwMDUserType = IIS_MD_UT_SERVER; 
        MyRecord.dwMDDataType = dtype;
        MyRecord.dwMDDataLen = dwBufLen; 
        MyRecord.pbMDData = (unsigned char *)pbBuffer; 
  
        hRes =
            (*pIMeta)->GetData(MyHandle, TEXT(""), &MyRecord, &dwReqBufLen); 

        if (SUCCEEDED(hRes)) { 
            jstring strResult;
            // Store the data identifiers in an array for future use. 
            // Note: declare a suitable DWORD array for names and add 
            // array bounds checking. 
            strResult = env->NewString((const jchar *)MyRecord.pbMDData,
                                       lstrlen(pbBuffer));
            return strResult;
        }
    }
    jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
    env->ThrowNew(cls, "No Such string value");
    return NULL;
}

JNIEXPORT jobjectArray SIGAR_JNI(win32_MetaBase_MetaBaseGetMultiStringValue)
(JNIEnv *env, jobject cur, jint key)
{
    HRESULT hRes = 0;
    METADATA_HANDLE MyHandle; 
    METADATA_RECORD MyRecord; 
    DWORD dwBufLen = 8096; 
    DWORD dwReqBufLen = 0; 
    TCHAR pbBuffer[8096]; 

    CComPtr <IMSAdminBase> *pIMeta;
    pIMeta = (CComPtr <IMSAdminBase> *)env->GetLongField(cur, IMeta_field);

    MyHandle = (METADATA_HANDLE)env->GetIntField(cur, ptr_field);

    // Initialize the input structure - 
    // the values specify what kind of data to enumerate. 
    MyRecord.dwMDIdentifier = key;
    MyRecord.dwMDAttributes = 0; 
    MyRecord.dwMDUserType = IIS_MD_UT_SERVER; 
    MyRecord.dwMDDataType = MULTISZ_METADATA; 
    MyRecord.dwMDDataLen = dwBufLen; 
    MyRecord.pbMDData = (unsigned char *)pbBuffer; 
  
    hRes = (*pIMeta)->GetData(MyHandle, TEXT(""), &MyRecord, &dwReqBufLen); 
    if (SUCCEEDED(hRes)) {
        TCHAR *szThisInstance = NULL;
        jobjectArray ret = NULL;
        int i;
        // Start at -1 to account for the \0 at the end of the
        // list.
        int count = -1;

        // Count # of objects in dwInstanceList
        for (i = 0; i < MyRecord.dwMDDataLen; i+=2) {
            if ((TCHAR)MyRecord.pbMDData[i] == '\0') {
                count++;
            }
        }

        ret = (jobjectArray)env->
            NewObjectArray(count,
                           env->FindClass("java/lang/String"),
                           env->NewString((const jchar *)"", 1));
        if (env->ExceptionCheck()) {
            return NULL;
        }

        // Walk the return instance list, creating an array
        for (szThisInstance = (TCHAR *)MyRecord.pbMDData, i = 0;
             *szThisInstance != 0;
             szThisInstance += lstrlen(szThisInstance) + 1, i++) 
        {
            env->SetObjectArrayElement(ret,i,env->NewString(
                      (const jchar *)(LPCTSTR)szThisInstance,
                      lstrlen(szThisInstance)));
            if (env->ExceptionCheck()) {
                return NULL;
            }
        }
        return ret;
    }

    jclass cls = env->FindClass(WIN32_PACKAGE "Win32Exception");
    env->ThrowNew(cls, "No Such string value");
    return NULL;
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
