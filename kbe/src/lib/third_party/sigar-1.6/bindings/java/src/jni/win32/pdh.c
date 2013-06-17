/*
 * Copyright (c) 2004-2007 Hyperic, Inc.
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

#include <pdh.h>
#include <pdhmsg.h>

#include "win32bindings.h"
#include "javasigar.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PDH_ACCESS_DENIED
#define PDH_ACCESS_DENIED ((DWORD)0xC0000BDBL)
#endif

/**
 * Hack around not being able to format error codes using
 * FORMAT_MESSAGE_FROM_HMODULE.  We only define the error
 * codes that could be possibly returned.
 */
static char *get_error_message(PDH_STATUS status) {
    switch (status) {
    case PDH_CSTATUS_NO_MACHINE:
        return "The computer is unavailable";
    case PDH_CSTATUS_NO_OBJECT:
        return "The specified object could not be found on the computer";
    case PDH_INVALID_ARGUMENT:
        return "A required argument is invalid";
    case PDH_MEMORY_ALLOCATION_FAILURE:
        return "A required temporary buffer could not be allocated";
    case PDH_INVALID_HANDLE:
        return "The query handle is not valid";
    case PDH_NO_DATA:
        return "The query does not currently have any counters";
    case PDH_CSTATUS_BAD_COUNTERNAME:
        return "The counter name path string could not be parsed or "
            "interpreted";
    case PDH_CSTATUS_NO_COUNTER:
        return "The specified counter was not found";
    case PDH_CSTATUS_NO_COUNTERNAME:
        return "An empty counter name path string was passed in";
    case PDH_FUNCTION_NOT_FOUND:
        return "The calculation function for this counter could not "
            "be determined";
    case PDH_ACCESS_DENIED:
        return "Access denied";
    default:
        return "Unknown error";
    }
}

JNIEXPORT jint SIGAR_JNI(win32_Pdh_validate)
(JNIEnv *env, jclass cur, jstring jpath)
{
    PDH_STATUS status;
    jboolean is_copy;
    LPCTSTR path = JENV->GetStringChars(env, jpath, &is_copy);

    status = PdhValidatePath(path);
    if (is_copy) {
        JENV->ReleaseStringChars(env, jpath, path);
    }

    return status;
}

JNIEXPORT void SIGAR_JNI(win32_Pdh_pdhConnectMachine)
(JNIEnv *env, jobject cur, jstring jhost)
{
    PDH_STATUS status;
    LPCTSTR host = JENV->GetStringChars(env, jhost, NULL);

    status = PdhConnectMachine(host);
    JENV->ReleaseStringChars(env, jhost, host);

    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
    }
}

JNIEXPORT jlong JNICALL SIGAR_JNI(win32_Pdh_pdhOpenQuery)
(JNIEnv *env, jobject cur)
{
    HQUERY     h_query;
    PDH_STATUS status;

    status = PdhOpenQuery(NULL, 0, &h_query);
    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return 0;
    }
    return (jlong)h_query;
}

JNIEXPORT void SIGAR_JNI(win32_Pdh_pdhCloseQuery)
(JNIEnv *env, jclass cur, jlong query)
{
    HQUERY     h_query    = (HQUERY)query;
    PDH_STATUS status;

    // Close the query and the log file.
    status = PdhCloseQuery(h_query);

    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return;
    }
}

JNIEXPORT jlong SIGAR_JNI(win32_Pdh_pdhAddCounter)
(JNIEnv *env, jclass cur, jlong query, jstring cp)
{
    HCOUNTER   h_counter;
    HQUERY     h_query = (HQUERY)query;
    PDH_STATUS status;
    LPCTSTR    counter_path = JENV->GetStringChars(env, cp, NULL);

    /* Add the counter that created the data in the log file. */
    status = PdhAddCounter(h_query, counter_path, 0, &h_counter);

    if (status == PDH_CSTATUS_NO_COUNTER) {
        /* if given counter does not exist,
         * try the same name w/ "/sec" appended
         */
        TCHAR counter_sec[MAX_PATH];
        lstrcpy(counter_sec, counter_path);
        lstrcat(counter_sec, _T("/sec"));
        status = PdhAddCounter(h_query, counter_sec, 0, &h_counter);
    }

    JENV->ReleaseStringChars(env, cp, counter_path);

    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return 0;
    }

    return (jlong)h_counter;
}

JNIEXPORT void SIGAR_JNI(win32_Pdh_pdhRemoveCounter)
(JNIEnv *env, jclass cur, jlong counter)
{
    HCOUNTER   h_counter = (HCOUNTER)counter;
    PDH_STATUS status;
    
    status = PdhRemoveCounter(h_counter);

    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return;
    }
}

JNIEXPORT jdouble SIGAR_JNI(win32_Pdh_pdhGetValue)
(JNIEnv *env, jclass cur, jlong query, jlong counter, jboolean fmt)
{
    HCOUNTER              h_counter      = (HCOUNTER)counter;
    HQUERY                h_query        = (HQUERY)query;
    PDH_STATUS            status;
    PDH_RAW_COUNTER raw_value;
    PDH_FMT_COUNTERVALUE fmt_value;
    DWORD type;

    status = PdhCollectQueryData(h_query);
   
    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return 0;
    }

    if (fmt) {
        /* may require 2 counters, see msdn docs */
        int i=0;
        for (i=0; i<2; i++) {
            status = PdhGetFormattedCounterValue(h_counter,
                                                 PDH_FMT_DOUBLE,
                                                 (LPDWORD)NULL,
                                                 &fmt_value);
            if (status == ERROR_SUCCESS) {
                break;
            }

            PdhCollectQueryData(h_query);
        }
    }
    else {
        status = PdhGetRawCounterValue(h_counter, &type, &raw_value);
    }

    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return 0;
    }

    if (fmt) {
        return fmt_value.doubleValue;
    }
    else {
        return (jdouble)raw_value.FirstValue;
    }
}

JNIEXPORT jstring SIGAR_JNI(win32_Pdh_pdhGetDescription)
(JNIEnv *env, jclass cur, jlong counter)
{
    HCOUNTER h_counter = (HCOUNTER)counter;
    PDH_COUNTER_INFO *info = NULL;
    jstring retval = NULL;
    DWORD size = 0;
    PDH_STATUS status;

    status = PdhGetCounterInfo(h_counter, TRUE, &size, NULL);
    if (status != PDH_MORE_DATA) {
        win32_throw_exception(env, get_error_message(status));
        return NULL;
    }

    info = malloc(size);

    status = PdhGetCounterInfo(h_counter, 1, &size, info);
    if (status == ERROR_SUCCESS) {
        if (info->szExplainText) {
            retval = JENV->NewString(env, info->szExplainText,
                                     lstrlen(info->szExplainText));
        }
    }
    else {
        win32_throw_exception(env, get_error_message(status));
    }

    free(info);
    return retval;
}

JNIEXPORT jlong SIGAR_JNI(win32_Pdh_pdhGetCounterType)
(JNIEnv *env, jclass cur, jlong counter)
{
    HCOUNTER h_counter = (HCOUNTER)counter;
    PDH_COUNTER_INFO info;
    DWORD size = sizeof(info);
    PDH_STATUS status;

    status = PdhGetCounterInfo(h_counter, FALSE, &size, &info);
    if (status != ERROR_SUCCESS) {
        win32_throw_exception(env, get_error_message(status));
        return -1;
    }

    return info.dwType;
}

JNIEXPORT jobjectArray SIGAR_JNI(win32_Pdh_pdhGetInstances)
(JNIEnv *env, jclass cur, jstring cp)
{
    PDH_STATUS   status              = ERROR_SUCCESS;
    DWORD        counter_list_size   = 0;
    DWORD        instance_list_size  = 8096;
    LPTSTR       instance_list_buf   = 
        (LPTSTR)malloc ((instance_list_size * sizeof (TCHAR)));
    LPTSTR       cur_object          = NULL;
    LPCTSTR      counter_path        = 
        JENV->GetStringChars(env, cp, 0);
    jobjectArray array = NULL;

    status = PdhEnumObjectItems(NULL, NULL, counter_path, NULL,
                                &counter_list_size, instance_list_buf,
                                &instance_list_size, PERF_DETAIL_WIZARD,
                                FALSE);
    
    if (status == PDH_MORE_DATA && instance_list_size > 0) {
        // Allocate the buffers and try the call again.
        if (instance_list_buf != NULL) 
            free(instance_list_buf);
        
        instance_list_buf = (LPTSTR)malloc((instance_list_size * 
                                            sizeof (TCHAR)));
        counter_list_size = 0;
        status  = PdhEnumObjectItems (NULL, NULL, counter_path,
                                      NULL, &counter_list_size,
                                      instance_list_buf,
                                      &instance_list_size,
                                      PERF_DETAIL_WIZARD, FALSE);
    }

    JENV->ReleaseStringChars(env, cp, counter_path);

    // Still may get PDH_ERROR_MORE data after the first reallocation,
    // but that is OK for just browsing the instances
    if (status == ERROR_SUCCESS || status == PDH_MORE_DATA) {
        int i, count;
        
        for (cur_object = instance_list_buf, count = 0;
             *cur_object != 0;
             cur_object += lstrlen(cur_object) + 1, count++);
            
        array = JENV->NewObjectArray(env, count,
                                     JENV->FindClass(env, 
                                                     "java/lang/String"),
                                     JENV->NewStringUTF(env, ""));
        if (JENV->ExceptionCheck(env)) {
            free(instance_list_buf);
            return NULL;
        }

        /* Walk the return instance list, creating an array */
        for (cur_object = instance_list_buf, i = 0;
             *cur_object != 0;
             i++) 
        {
            int len = lstrlen(cur_object);
            jstring s =
                JENV->NewString(env, (const jchar *)cur_object, len);
            JENV->SetObjectArrayElement(env, array, i, s);
            if (JENV->ExceptionCheck(env)) {
                free(instance_list_buf);
                return NULL;
            }
            cur_object += len + 1;
        }
    } else {
        if (instance_list_buf != NULL) 
            free(instance_list_buf);
        
        // An error occured
        win32_throw_exception(env, get_error_message(status));
        return NULL;
    }

    if (instance_list_buf != NULL) 
        free(instance_list_buf);

    return array;
}

JNIEXPORT jobjectArray SIGAR_JNI(win32_Pdh_pdhGetKeys)
(JNIEnv *env, jclass cur, jstring cp)
{
    PDH_STATUS  status              = ERROR_SUCCESS;
    DWORD       counter_list_size   = 8096;
    DWORD       instance_list_size  = 0;
    LPTSTR      instance_list_buf   = 
        (LPTSTR)malloc (counter_list_size * sizeof(TCHAR));
    LPTSTR      cur_object          = NULL;
    LPCTSTR     counter_path        = JENV->GetStringChars(env, cp, NULL);
    jobjectArray array              = NULL;

    status = PdhEnumObjectItems(NULL, NULL, counter_path,
                                instance_list_buf, &counter_list_size,
                                NULL, &instance_list_size,
                                PERF_DETAIL_WIZARD, FALSE); 
        
    if (status == PDH_MORE_DATA) {
        /* Allocate the buffers and try the call again. */
        if (instance_list_buf != NULL)
            free(instance_list_buf);

        instance_list_buf = (LPTSTR)malloc(counter_list_size *
                                           sizeof(TCHAR));
        instance_list_size = 0;
        status = PdhEnumObjectItems (NULL, NULL, counter_path,
                                     instance_list_buf,
                                     &counter_list_size, NULL,
                                     &instance_list_size,
                                     PERF_DETAIL_WIZARD, 0);
    }

    JENV->ReleaseStringChars(env, cp, counter_path);

    if (status == ERROR_SUCCESS || status == PDH_MORE_DATA) {
        int i, count;

        for (cur_object = instance_list_buf, count = 0;
             *cur_object != 0;
             cur_object += lstrlen(cur_object) + 1, count++);

        array = JENV->NewObjectArray(env, count,
                                     JENV->FindClass(env, 
                                                     "java/lang/String"),
                                     JENV->NewStringUTF(env, ""));
        if (JENV->ExceptionCheck(env)) {
            free(instance_list_buf);
            return NULL;
        }

        /* Walk the return instance list, creating an array */
        for (cur_object = instance_list_buf, i = 0;
             *cur_object != 0;
             i++) 
        {
            int len = lstrlen(cur_object);
            jstring s =
                JENV->NewString(env, (const jchar *)cur_object, len);
            JENV->SetObjectArrayElement(env, array, i, s);
            if (JENV->ExceptionCheck(env)) {
                free(instance_list_buf);
                return NULL;
            }
            cur_object += len + 1;
        }
    } else {
        // An error occured
        if (instance_list_buf != NULL) 
            free(instance_list_buf);
        
        // An error occured
        win32_throw_exception(env, get_error_message(status));
        return NULL;
    }
    
    if (instance_list_buf != NULL) 
            free(instance_list_buf);

    return array;
}

JNIEXPORT jobjectArray SIGAR_JNI(win32_Pdh_pdhGetObjects)
(JNIEnv *env, jclass cur)
{
    PDH_STATUS   status;
    DWORD        list_size       = 8096;
    LPTSTR       list_buf        = (LPTSTR)malloc(list_size * sizeof(TCHAR));
    LPTSTR       cur_object;
    DWORD        i, num_objects  = 0;
    jobjectArray array           = NULL;

    status = PdhEnumObjects(NULL, NULL, list_buf, &list_size,
                            PERF_DETAIL_WIZARD, FALSE);

    if (status == PDH_MORE_DATA) {
        // Re-try call with a larger buffer
        if (list_buf != NULL)
            free(list_buf);

        list_buf = (LPTSTR)malloc(list_size * sizeof(TCHAR));
        status = PdhEnumObjects(NULL, NULL, list_buf, &list_size,
                                PERF_DETAIL_WIZARD, FALSE);
    }

    if (status != ERROR_SUCCESS) {
        if (list_buf != NULL)
            free(list_buf);

        win32_throw_exception(env, get_error_message(status));
        return NULL;
    }

    // Walk the return buffer counting the number of objects
    for (cur_object = list_buf, num_objects = 0;
         *cur_object != 0;
         cur_object += lstrlen(cur_object) + 1, num_objects++);

    array = JENV->NewObjectArray(env, num_objects,
                                 JENV->FindClass(env, 
                                                 "java/lang/String"),
                                 JENV->NewStringUTF(env, ""));
    if (JENV->ExceptionCheck(env)) {
        free(list_buf);
        return NULL;
    }

    for (cur_object = list_buf, i = 0;
         *cur_object != 0;
         i++) 
    {
        int len = lstrlen(cur_object);
        jstring s =
            JENV->NewString(env, (const jchar *)cur_object, len);
        JENV->SetObjectArrayElement(env, array, i, s);
        if (JENV->ExceptionCheck(env)) {
            free(list_buf);
            return NULL;
        }
        cur_object += len + 1;
    }

    if (list_buf != NULL)
        free(list_buf);

    return array;
}

JNIEXPORT jstring SIGAR_JNI(win32_Pdh_pdhLookupPerfName)
(JNIEnv *env, jclass cur, jint index)
{
    TCHAR path[MAX_PATH + 1];
    DWORD len = sizeof(path) / sizeof(TCHAR); /* len is number of TCHAR's, not sizeof(path) */
    PDH_STATUS status =
        PdhLookupPerfNameByIndex(NULL, index, path, &len);

    if (status == ERROR_SUCCESS) {
        return JENV->NewString(env, (const jchar *)path, len);
    }
    else {
        win32_throw_exception(env, get_error_message(status));
        return NULL;
    }
}

JNIEXPORT jint SIGAR_JNI(win32_Pdh_pdhLookupPerfIndex)
(JNIEnv *env, jclass cur, jstring jname)
{
    DWORD index;
    LPCTSTR name =
        JENV->GetStringChars(env, jname, NULL);
    PDH_STATUS status =
        PdhLookupPerfIndexByNameW(NULL, name, &index);

    JENV->ReleaseStringChars(env, jname, name);

    if (status == ERROR_SUCCESS) {
        return index;
    }
    else {
        win32_throw_exception(env, get_error_message(status));
        return -1;
    }
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
