/*
 * Copyright (c) 2007 Hyperic, Inc.
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

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint SIGAR_JNI(win32_LocaleInfo_getSystemDefaultLCID)
(JNIEnv *env, jclass objcls)
{
    return GetSystemDefaultLCID();
}

JNIEXPORT jstring SIGAR_JNI(win32_LocaleInfo_getAttribute)
(JNIEnv *env, jclass objcls, jint lcid, jint attr)
{
    TCHAR value[8192];
    int retval =
        GetLocaleInfo(lcid,
                      attr,
                      value, sizeof(value) / sizeof(TCHAR));

    if (retval) {
        int len = lstrlen(value);
        return env->NewString((const jchar *)value, len);
    }
    else {
        return NULL;
    }
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
