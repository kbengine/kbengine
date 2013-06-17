/*
 * Copyright (c) 2004-2006, 2008 Hyperic, Inc.
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

#include <pdh.h>
#include <pdhmsg.h>

#include "sigar.h"
#include "sigar_private.h"
#include "win32bindings.h"
#include "javasigar.h"

/**
 * Set of common utilities for all win32bindings objects
 */

#ifdef __cplusplus
extern "C" {
#endif

void win32_throw_exception(JNIEnv *env, char *msg)
{
    jclass exceptionClass = WIN32_FIND_CLASS("Win32Exception");
    JENV->ThrowNew(env, exceptionClass, msg);
}

void win32_throw_last_error(JNIEnv *env)
{
    win32_throw_error(env, GetLastError());
}

void win32_throw_error(JNIEnv *env, LONG err)
{
    char msg[8192];

    win32_throw_exception(env,
                          sigar_strerror_get(err, msg, sizeof(msg)));
}

#ifdef __cplusplus
}
#endif
#endif /* WIN32 */
