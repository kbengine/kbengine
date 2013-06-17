/*
 * Copyright (c) 2004-2006 Hyperic, Inc.
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

#ifndef WIN32_BINDINGS_H
#define WIN32_BINDINGS_H

/* Exclude rarely-used stuff from windows headers */
#define WIN32_LEAN_AND_MEAN

/* Windows Header Files */
#include <windows.h>

#include <tchar.h>

/* Include java jni headers */
#include <jni.h>

#define WIN32_PACKAGE "org/hyperic/sigar/win32/"

#define WIN32_FIND_CLASS(name) \
    JENV->FindClass(env, WIN32_PACKAGE name)

#define WIN32_ALLOC_OBJECT(name) \
    JENV->AllocObject(env, WIN32_FIND_CLASS(name))

#define SetStringField(env, obj, fieldID, val) \
    JENV->SetObjectField(env, obj, fieldID, JENV->NewStringUTF(env, val))

#ifdef __cplusplus
extern "C" {
#endif
    void win32_throw_exception(JNIEnv *env, char *msg);

    void win32_throw_error(JNIEnv *env, LONG err);

    void win32_throw_last_error(JNIEnv *env);
#ifdef __cplusplus
}
#endif
#endif
