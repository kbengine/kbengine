/*
 * Copyright (c) 2004, 2006, 2008 Hyperic, Inc.
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

#include <jni.h>
#include "sigar.h"

#define JENV (*env)

#define SIGAR_PACKAGE "org/hyperic/sigar/"

#define SIGAR_JNI(m) JNICALL Java_org_hyperic_sigar_##m

#define SIGAR_JNIx(m) JNICALL Java_org_hyperic_sigar_Sigar_##m

#define SIGAR_FIND_CLASS(name) \
    JENV->FindClass(env, SIGAR_PACKAGE name)

#define SIGAR_CLASS_SIG(name) \
    "L" SIGAR_PACKAGE name ";"

/* CHeck EXception */
#define SIGAR_CHEX if (JENV->ExceptionCheck(env)) return NULL

typedef struct {
    JNIEnv *env;
    jobject obj;
    jmethodID id;
} jsigar_list_t;

#ifdef __cplusplus
extern "C" {
#endif

int jsigar_list_init(JNIEnv *env, jsigar_list_t *obj);

int jsigar_list_add(void *data, char *value, int len);

sigar_t *jsigar_get_sigar(JNIEnv *env, jobject sigar_obj);

#ifdef __cplusplus
}
#endif
