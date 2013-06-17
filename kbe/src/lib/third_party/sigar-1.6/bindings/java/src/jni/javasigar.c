/*
 * Copyright (c) 2004-2008 Hyperic, Inc.
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
#include "sigar_fileinfo.h"
#include "sigar_log.h"
#include "sigar_private.h"
#include "sigar_ptql.h"
#include "sigar_util.h"
#include "sigar_os.h"
#include "sigar_format.h"

#include <string.h>

#ifdef WIN32
#include <winsock.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#  ifdef __FreeBSD__
#    include <sys/param.h>
#    if (__FreeBSD_version < 700000)
#      include <sys/time.h>
#    endif
#  else
#    include <sys/time.h>
#  endif
#include <sys/resource.h>
#endif

#include "javasigar_generated.h"
#include "javasigar.h"

#ifdef SIGAR_64BIT
#define SIGAR_POINTER_LONG
#endif

typedef struct {
    jclass classref;
    jfieldID *ids;
} jsigar_field_cache_t;

typedef struct {
    JNIEnv *env;
    jobject logger;
    sigar_t *sigar;
    jsigar_field_cache_t *fields[JSIGAR_FIELDS_MAX];
    int open_status;
    jthrowable not_impl;
} jni_sigar_t;

#define dSIGAR_GET \
    jni_sigar_t *jsigar = sigar_get_jpointer(env, sigar_obj); \
    sigar_t *sigar

#define dSIGAR_VOID \
    dSIGAR_GET; \
    if (!jsigar) return; \
    sigar = jsigar->sigar; \
    jsigar->env = env

#define dSIGAR(val) \
    dSIGAR_GET; \
    if (!jsigar) return val; \
    sigar = jsigar->sigar; \
    jsigar->env = env

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved)
{
#ifdef DMALLOC
    char *options =
        getenv("DMALLOC_OPTIONS");
    if (!options) {
        options = 
            "debug=0x4f47d03,"
            "lockon=20,"
            "log=dmalloc-sigar.log";
    }
    dmalloc_debug_setup(options);
#endif
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *vm, void *reserved)
{
#ifdef DMALLOC
    dmalloc_shutdown();
#endif
}

static void sigar_throw_exception(JNIEnv *env, char *msg)
{
    jclass errorClass = SIGAR_FIND_CLASS("SigarException");

    JENV->ThrowNew(env, errorClass, msg);
}

#define SIGAR_NOTIMPL_EX "SigarNotImplementedException"

static void sigar_throw_notimpl(JNIEnv *env, char *msg)
{
    jclass errorClass = SIGAR_FIND_CLASS(SIGAR_NOTIMPL_EX);

    JENV->ThrowNew(env, errorClass, msg);
}

static void sigar_throw_ptql_malformed(JNIEnv *env, char *msg)
{
    jclass errorClass = SIGAR_FIND_CLASS("ptql/MalformedQueryException");

    JENV->ThrowNew(env, errorClass, msg);
}

static void sigar_throw_error(JNIEnv *env, jni_sigar_t *jsigar, int err)
{
    jclass errorClass;
    int err_type = err;

    /* 
     * support:
     * #define SIGAR_EPERM_KMEM (SIGAR_OS_START_ERROR+EACCES)
     * this allows for os impl specific message
     * (e.g. Failed to open /dev/kmem) but still map to the proper
     * Sigar*Exception
     */
    if (err_type > SIGAR_OS_START_ERROR) {
        err_type -= SIGAR_OS_START_ERROR;
    }

    switch (err_type) {
      case SIGAR_ENOENT:
        errorClass = SIGAR_FIND_CLASS("SigarFileNotFoundException");
        break;

      case SIGAR_EACCES:
        errorClass = SIGAR_FIND_CLASS("SigarPermissionDeniedException");
        break;

      case SIGAR_ENOTIMPL:
        if (jsigar->not_impl == NULL) {
            jfieldID id;
            jthrowable not_impl;

            errorClass = SIGAR_FIND_CLASS(SIGAR_NOTIMPL_EX);

            id = JENV->GetStaticFieldID(env, errorClass,
                                        "INSTANCE",
                                        SIGAR_CLASS_SIG(SIGAR_NOTIMPL_EX));

            not_impl = JENV->GetStaticObjectField(env, errorClass, id);

            jsigar->not_impl = JENV->NewGlobalRef(env, not_impl);
        }

        JENV->Throw(env, jsigar->not_impl);
        return;
      default:
        errorClass = SIGAR_FIND_CLASS("SigarException");
        break;
    }

    JENV->ThrowNew(env, errorClass,
                   sigar_strerror(jsigar->sigar, err));
}

static void *sigar_get_pointer(JNIEnv *env, jobject obj) {
    jfieldID pointer_field;
    jclass cls = JENV->GetObjectClass(env, obj);

#ifdef SIGAR_POINTER_LONG
    pointer_field = JENV->GetFieldID(env, cls, "longSigarWrapper", "J");
    return (void *)JENV->GetLongField(env, obj, pointer_field);
#else
    pointer_field = JENV->GetFieldID(env, cls, "sigarWrapper", "I");
    return (void *)JENV->GetIntField(env, obj, pointer_field);
#endif
}

static jni_sigar_t *sigar_get_jpointer(JNIEnv *env, jobject obj) {
    jni_sigar_t *jsigar =
        (jni_sigar_t *)sigar_get_pointer(env, obj);

    if (!jsigar) {
        sigar_throw_exception(env, "sigar has been closed");
        return NULL;
    }

    if (jsigar->open_status != SIGAR_OK) {
        sigar_throw_error(env, jsigar,
                          jsigar->open_status);
        return NULL;
    }

    return jsigar;
}

static void sigar_set_pointer(JNIEnv *env, jobject obj, const void *ptr) {
    jfieldID pointer_field;
    jclass cls = JENV->GetObjectClass(env, obj);

#ifdef SIGAR_POINTER_LONG
    pointer_field = JENV->GetFieldID(env, cls, "longSigarWrapper", "J");
    JENV->SetLongField(env, obj, pointer_field, (jlong)ptr);
#else
    pointer_field = JENV->GetFieldID(env, cls, "sigarWrapper", "I");
    JENV->SetIntField(env, obj, pointer_field, (int)ptr);
#endif
}

/* for jni/win32 */
sigar_t *jsigar_get_sigar(JNIEnv *env, jobject sigar_obj)
{
    dSIGAR(NULL);
    return jsigar->sigar;
}

int jsigar_list_init(JNIEnv *env, jsigar_list_t *obj)
{
    jclass listclass =
        JENV->FindClass(env, "java/util/ArrayList");
    jmethodID listid =
        JENV->GetMethodID(env, listclass, "<init>", "()V");
    jmethodID addid =
        JENV->GetMethodID(env, listclass, "add",
                          "(Ljava/lang/Object;)"
                          "Z");

    obj->env = env;
    obj->obj = JENV->NewObject(env, listclass, listid);
    obj->id = addid;

    return JENV->ExceptionCheck(env) ? !SIGAR_OK : SIGAR_OK;
}

int jsigar_list_add(void *data, char *value, int len)
{
    jsigar_list_t *obj = (jsigar_list_t *)data;
    JNIEnv *env = obj->env;

    JENV->CallBooleanMethod(env, obj->obj, obj->id,  
                            JENV->NewStringUTF(env, value));

    return JENV->ExceptionCheck(env) ? !SIGAR_OK : SIGAR_OK;
}

JNIEXPORT jstring SIGAR_JNIx(formatSize)
(JNIEnv *env, jclass cls, jlong size)
{
    char buf[56];
    sigar_format_size(size, buf);
    return JENV->NewStringUTF(env, buf);
}

JNIEXPORT jstring SIGAR_JNIx(getNativeVersion)
(JNIEnv *env, jclass cls)
{
    sigar_version_t *version = sigar_version_get();
    return JENV->NewStringUTF(env, version->version);
}

JNIEXPORT jstring SIGAR_JNIx(getNativeBuildDate)
(JNIEnv *env, jclass cls)
{
    sigar_version_t *version = sigar_version_get();
    return JENV->NewStringUTF(env, version->build_date);
}

JNIEXPORT jstring SIGAR_JNIx(getNativeScmRevision)
(JNIEnv *env, jclass cls)
{
    sigar_version_t *version = sigar_version_get();
    return JENV->NewStringUTF(env, version->scm_revision);
}

JNIEXPORT void SIGAR_JNIx(open)
(JNIEnv *env, jobject obj)
{
    jni_sigar_t *jsigar = malloc(sizeof(*jsigar));

    memset(jsigar, '\0', sizeof(*jsigar));

    sigar_set_pointer(env, obj, jsigar);
        
    /* this method is called by the constructor.
     * if != SIGAR_OK save status and throw exception
     * when methods are invoked (see sigar_get_pointer).
     */
    if ((jsigar->open_status = sigar_open(&jsigar->sigar)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, jsigar->open_status);
        return;
    }
}

JNIEXPORT jint SIGAR_JNIx(nativeClose)
(JNIEnv *env, jobject sigar_obj)
{
    jint status;
    int i;
    dSIGAR(0);

    /* only place it is possible this would be something other than
     * SIGAR_OK is on win32 if RegCloseKey fails, which i don't think
     * is possible either.
     */
    status = sigar_close(sigar);

    if (jsigar->logger != NULL) {
        JENV->DeleteGlobalRef(env, jsigar->logger);
    }

    if (jsigar->not_impl != NULL) {
        JENV->DeleteGlobalRef(env, jsigar->not_impl);
    }

    for (i=0; i<JSIGAR_FIELDS_MAX; i++) {
        if (jsigar->fields[i]) {
            JENV->DeleteGlobalRef(env,
                                  jsigar->fields[i]->classref);
            free(jsigar->fields[i]->ids);
            free(jsigar->fields[i]);
        }
    }

    free(jsigar);
    sigar_set_pointer(env, sigar_obj, NULL);

    return status;
}

JNIEXPORT jlong SIGAR_JNIx(getPid)
(JNIEnv *env, jobject sigar_obj)
{
    dSIGAR(0);

    return sigar_pid_get(sigar);
}

JNIEXPORT void SIGAR_JNIx(kill)
(JNIEnv *env, jobject sigar_obj, jlong pid, jint signum)
{
    int status;
    dSIGAR_VOID;

    if ((status = sigar_proc_kill(pid, signum)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
    }
}

JNIEXPORT jint SIGAR_JNIx(getSigNum)
(JNIEnv *env, jclass cls_obj, jstring jname)
{
    jboolean is_copy;
    const char *name;
    int num;

    name = JENV->GetStringUTFChars(env, jname, &is_copy);

    num = sigar_signum_get((char *)name);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jname, name);
    }

    return num;
}

#define SetStringField(env, obj, fieldID, val) \
    SetObjectField(env, obj, fieldID, JENV->NewStringUTF(env, val))

static jstring jnet_address_to_string(JNIEnv *env, sigar_t *sigar, sigar_net_address_t *val) {
    char addr_str[SIGAR_INET6_ADDRSTRLEN];
    sigar_net_address_to_string(sigar, val, addr_str);
    return JENV->NewStringUTF(env, addr_str);
}

#define SetNetAddressField(env, obj, fieldID, val) \
    SetObjectField(env, obj, fieldID, jnet_address_to_string(env, sigar, &val))

#include "javasigar_generated.c"

enum {
    FS_FIELD_DIRNAME,
    FS_FIELD_DEVNAME,
    FS_FIELD_SYS_TYPENAME,
    FS_FIELD_OPTIONS,
    FS_FIELD_TYPE,
    FS_FIELD_TYPENAME,
    FS_FIELD_MAX
};

#define STRING_SIG "Ljava/lang/String;"

JNIEXPORT jobjectArray SIGAR_JNIx(getFileSystemListNative)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    sigar_file_system_list_t fslist;
    jobjectArray fsarray;
    jfieldID ids[FS_FIELD_MAX];
    jclass nfs_cls=NULL, cls = SIGAR_FIND_CLASS("FileSystem");
    dSIGAR(NULL);

    if ((status = sigar_file_system_list_get(sigar, &fslist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    ids[FS_FIELD_DIRNAME] =
        JENV->GetFieldID(env, cls, "dirName", STRING_SIG);

    ids[FS_FIELD_DEVNAME] =
        JENV->GetFieldID(env, cls, "devName", STRING_SIG);

    ids[FS_FIELD_TYPENAME] =
        JENV->GetFieldID(env, cls, "typeName", STRING_SIG);

    ids[FS_FIELD_SYS_TYPENAME] =
        JENV->GetFieldID(env, cls, "sysTypeName", STRING_SIG);

    ids[FS_FIELD_OPTIONS] =
        JENV->GetFieldID(env, cls, "options", STRING_SIG);

    ids[FS_FIELD_TYPE] =
        JENV->GetFieldID(env, cls, "type", "I");

    fsarray = JENV->NewObjectArray(env, fslist.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<fslist.number; i++) {
        sigar_file_system_t *fs = &(fslist.data)[i];
        jobject fsobj;
        jclass obj_cls;

#ifdef WIN32
        obj_cls = cls;
#else
        if ((fs->type == SIGAR_FSTYPE_NETWORK) &&
            (strcmp(fs->sys_type_name, "nfs") == 0) &&
            strstr(fs->dev_name, ":/"))
        {
            if (!nfs_cls) {
                nfs_cls = SIGAR_FIND_CLASS("NfsFileSystem");
            }
            obj_cls = nfs_cls;
        }
        else {
            obj_cls = cls;
        }
#endif

        fsobj = JENV->AllocObject(env, obj_cls);
        SIGAR_CHEX;

        JENV->SetStringField(env, fsobj,
                             ids[FS_FIELD_DIRNAME],
                             fs->dir_name);

        JENV->SetStringField(env, fsobj,
                             ids[FS_FIELD_DEVNAME],
                             fs->dev_name);

        JENV->SetStringField(env, fsobj,
                             ids[FS_FIELD_SYS_TYPENAME],
                             fs->sys_type_name);

        JENV->SetStringField(env, fsobj,
                             ids[FS_FIELD_OPTIONS],
                             fs->options);

        JENV->SetStringField(env, fsobj,
                             ids[FS_FIELD_TYPENAME],
                             fs->type_name);

        JENV->SetIntField(env, fsobj,
                          ids[FS_FIELD_TYPE],
                          fs->type);

        JENV->SetObjectArrayElement(env, fsarray, i, fsobj);
        SIGAR_CHEX;
    }

    sigar_file_system_list_destroy(sigar, &fslist);

    return fsarray;
}

JNIEXPORT jint SIGAR_JNI(RPC_ping)
(JNIEnv *env, jclass cls_obj, jstring jhostname,
 jint protocol, jlong program, jlong version)
{
#ifdef WIN32
    return JNI_FALSE; /*XXX*/
#else
    jboolean is_copy;
    const char *hostname;
    int status;

    if (!jhostname) {
        return 13; /* RPC_UNKNOWNHOST */
    }

    hostname = JENV->GetStringUTFChars(env, jhostname, &is_copy);

    status =
        sigar_rpc_ping((char *)hostname,
                       protocol, program, version);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jhostname, hostname);
    }

    return status;
#endif
}

JNIEXPORT jstring SIGAR_JNI(RPC_strerror)
(JNIEnv *env, jclass cls_obj, jint err)
{
#ifdef WIN32
    return NULL;
#else
    return JENV->NewStringUTF(env, sigar_rpc_strerror(err));
#endif
}

JNIEXPORT jobjectArray SIGAR_JNIx(getCpuInfoList)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    sigar_cpu_info_list_t cpu_infos;
    jobjectArray cpuarray;
    jclass cls = SIGAR_FIND_CLASS("CpuInfo");
    dSIGAR(NULL);

    if ((status = sigar_cpu_info_list_get(sigar, &cpu_infos)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JAVA_SIGAR_INIT_FIELDS_CPUINFO(cls);

    cpuarray = JENV->NewObjectArray(env, cpu_infos.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<cpu_infos.number; i++) {
        jobject info_obj = JENV->AllocObject(env, cls);
        SIGAR_CHEX;
        JAVA_SIGAR_SET_FIELDS_CPUINFO(cls, info_obj,
                                      cpu_infos.data[i]);
        JENV->SetObjectArrayElement(env, cpuarray, i, info_obj);
        SIGAR_CHEX;
    }

    sigar_cpu_info_list_destroy(sigar, &cpu_infos);

    return cpuarray;
}

JNIEXPORT jobjectArray SIGAR_JNIx(getCpuListNative)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    sigar_cpu_list_t cpulist;
    jobjectArray cpuarray;
    jclass cls = SIGAR_FIND_CLASS("Cpu");
    dSIGAR(NULL);

    if ((status = sigar_cpu_list_get(sigar, &cpulist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JAVA_SIGAR_INIT_FIELDS_CPU(cls);

    cpuarray = JENV->NewObjectArray(env, cpulist.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<cpulist.number; i++) {
        jobject info_obj = JENV->AllocObject(env, cls);
        SIGAR_CHEX;
        JAVA_SIGAR_SET_FIELDS_CPU(cls, info_obj,
                                  cpulist.data[i]);
        JENV->SetObjectArrayElement(env, cpuarray, i, info_obj);
        SIGAR_CHEX;
    }

    sigar_cpu_list_destroy(sigar, &cpulist);

    return cpuarray;
}

JNIEXPORT void SIGAR_JNI(CpuPerc_gather)
(JNIEnv *env, jobject jperc, jobject sigar_obj, jobject jprev, jobject jcurr)
{
    sigar_cpu_t prev, curr;
    sigar_cpu_perc_t perc;
    dSIGAR_VOID;

    JAVA_SIGAR_GET_FIELDS_CPU(jprev, prev);
    JAVA_SIGAR_GET_FIELDS_CPU(jcurr, curr);
    sigar_cpu_perc_calculate(&prev, &curr, &perc);
    JAVA_SIGAR_INIT_FIELDS_CPUPERC(JENV->GetObjectClass(env, jperc));
    JAVA_SIGAR_SET_FIELDS_CPUPERC(NULL, jperc, perc);
}

JNIEXPORT jlongArray SIGAR_JNIx(getProcList)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    jlongArray procarray;
    sigar_proc_list_t proclist;
    jlong *pids = NULL;
    dSIGAR(NULL);

    if ((status = sigar_proc_list_get(sigar, &proclist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    procarray = JENV->NewLongArray(env, proclist.number);
    SIGAR_CHEX;

    if (sizeof(jlong) == sizeof(sigar_pid_t)) {
        pids = (jlong *)proclist.data;
    }
    else {
        unsigned int i;
        pids = (jlong *)malloc(sizeof(jlong) * proclist.number);

        for (i=0; i<proclist.number; i++) {
            pids[i] = proclist.data[i];
        }
    }

    JENV->SetLongArrayRegion(env, procarray, 0,
                             proclist.number, pids);

    if (pids != (jlong *)proclist.data) {
        free(pids);
    }

    sigar_proc_list_destroy(sigar, &proclist);

    return procarray;
}

JNIEXPORT jobjectArray SIGAR_JNIx(getProcArgs)
(JNIEnv *env, jobject sigar_obj, jlong pid)
{
    int status;
    unsigned int i;
    sigar_proc_args_t procargs;
    jobjectArray argsarray;
    jclass stringclass = JENV->FindClass(env, "java/lang/String");
    dSIGAR(NULL);

    if ((status = sigar_proc_args_get(sigar, pid, &procargs)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    argsarray = JENV->NewObjectArray(env, procargs.number, stringclass, 0);
    SIGAR_CHEX;

    for (i=0; i<procargs.number; i++) {
        jstring s = JENV->NewStringUTF(env, procargs.data[i]);
        JENV->SetObjectArrayElement(env, argsarray, i, s);
        SIGAR_CHEX;
    }

    sigar_proc_args_destroy(sigar, &procargs);

    return argsarray;
}

typedef struct {
    JNIEnv *env;
    jobject map;
    jmethodID id;
} jni_env_put_t;

static int jni_env_getall(void *data,
                          const char *key, int klen,
                          char *val, int vlen)
{
    jni_env_put_t *put = (jni_env_put_t *)data;
    JNIEnv *env = put->env;

    JENV->CallObjectMethod(env, put->map, put->id,  
                           JENV->NewStringUTF(env, key),
                           JENV->NewStringUTF(env, val));

    return JENV->ExceptionCheck(env) ? !SIGAR_OK : SIGAR_OK;
}

#define MAP_PUT_SIG \
    "(Ljava/lang/Object;Ljava/lang/Object;)" \
    "Ljava/lang/Object;"

JNIEXPORT jobject SIGAR_JNI(ProcEnv_getAll)
(JNIEnv *env, jobject cls, jobject sigar_obj, jlong pid)
{
    int status;
    sigar_proc_env_t procenv;
    jobject hashmap;
    jni_env_put_t put;
    jclass mapclass =
        JENV->FindClass(env, "java/util/HashMap");
    jmethodID mapid =
        JENV->GetMethodID(env, mapclass, "<init>", "()V");
    jmethodID putid =
        JENV->GetMethodID(env, mapclass, "put", MAP_PUT_SIG);

    dSIGAR(NULL);

    hashmap = JENV->NewObject(env, mapclass, mapid);
    SIGAR_CHEX;

    put.env = env;
    put.id = putid;
    put.map = hashmap;

    procenv.type = SIGAR_PROC_ENV_ALL;
    procenv.env_getter = jni_env_getall;
    procenv.data = &put;

    if ((status = sigar_proc_env_get(sigar, pid, &procenv)) != SIGAR_OK) {
        JENV->DeleteLocalRef(env, hashmap);
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    return hashmap;
}

typedef struct {
    JNIEnv *env;
    const char *key;
    int klen;
    jstring val;
} jni_env_get_t;

static int jni_env_getvalue(void *data,
                            const char *key, int klen,
                            char *val, int vlen)
{
    jni_env_get_t *get = (jni_env_get_t *)data;
    JNIEnv *env = get->env;

    if ((get->klen == klen) &&
        (strcmp(get->key, key) == 0))
    {
        get->val = JENV->NewStringUTF(env, val);
        return !SIGAR_OK; /* foundit; stop iterating */
    }

    return SIGAR_OK;
}

JNIEXPORT jstring SIGAR_JNI(ProcEnv_getValue)
(JNIEnv *env, jobject cls, jobject sigar_obj, jlong pid, jstring key)
{
    int status;
    sigar_proc_env_t procenv;
    jni_env_get_t get;
    dSIGAR(NULL);

    get.env = env;
    get.key = JENV->GetStringUTFChars(env, key, 0);
    get.klen = JENV->GetStringUTFLength(env, key);
    get.val = NULL;

    procenv.type = SIGAR_PROC_ENV_KEY;
    procenv.key  = get.key;
    procenv.klen = get.klen;
    procenv.env_getter = jni_env_getvalue;
    procenv.data = &get;

    if ((status = sigar_proc_env_get(sigar, pid, &procenv)) != SIGAR_OK) {
        JENV->ReleaseStringUTFChars(env, key, get.key);
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JENV->ReleaseStringUTFChars(env, key, get.key);

    return get.val;
}

JNIEXPORT jobject SIGAR_JNIx(getProcModulesNative)
(JNIEnv *env, jobject sigar_obj, jlong pid)
{
    int status;
    sigar_proc_modules_t procmods;
    jsigar_list_t obj;

    dSIGAR(NULL);

    if (jsigar_list_init(env, &obj) != SIGAR_OK) {
        return NULL; /* Exception thrown */
    }

    procmods.module_getter = jsigar_list_add;
    procmods.data = &obj;

    if ((status = sigar_proc_modules_get(sigar, pid, &procmods)) != SIGAR_OK) {
        JENV->DeleteLocalRef(env, obj.obj);
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    return obj.obj;
}

JNIEXPORT jdoubleArray SIGAR_JNIx(getLoadAverage)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    jlongArray avgarray;
    sigar_loadavg_t loadavg;
    dSIGAR(NULL);

    if ((status = sigar_loadavg_get(sigar, &loadavg)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    avgarray = JENV->NewDoubleArray(env, 3);
    SIGAR_CHEX;

    JENV->SetDoubleArrayRegion(env, avgarray, 0,
                               3, loadavg.loadavg);

    return avgarray;
}

JNIEXPORT jobjectArray SIGAR_JNIx(getNetRouteList)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    jarray routearray;
    jclass cls = SIGAR_FIND_CLASS("NetRoute");
    sigar_net_route_list_t routelist;
    dSIGAR(NULL);

    if ((status = sigar_net_route_list_get(sigar, &routelist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JAVA_SIGAR_INIT_FIELDS_NETROUTE(cls);

    routearray = JENV->NewObjectArray(env, routelist.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<routelist.number; i++) {
        jobject obj = JENV->AllocObject(env, cls);
        SIGAR_CHEX;
        JAVA_SIGAR_SET_FIELDS_NETROUTE(cls, obj, routelist.data[i]);
        JENV->SetObjectArrayElement(env, routearray, i, obj);
        SIGAR_CHEX;
    }

    sigar_net_route_list_destroy(sigar, &routelist);

    return routearray;
}

JNIEXPORT jstring SIGAR_JNI(NetFlags_getIfFlagsString)
(JNIEnv *env, jclass cls, jlong flags)
{
    char buf[1024];
    sigar_net_interface_flags_to_string(flags, buf);
    return JENV->NewStringUTF(env, buf);
}

JNIEXPORT jobjectArray SIGAR_JNIx(getNetConnectionList)
(JNIEnv *env, jobject sigar_obj, jint flags)
{
    int status;
    unsigned int i;
    jarray connarray;
    jclass cls = SIGAR_FIND_CLASS("NetConnection");
    sigar_net_connection_list_t connlist;
    dSIGAR(NULL);

    status = sigar_net_connection_list_get(sigar, &connlist, flags);

    if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JAVA_SIGAR_INIT_FIELDS_NETCONNECTION(cls);

    connarray = JENV->NewObjectArray(env, connlist.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<connlist.number; i++) {
        jobject obj = JENV->AllocObject(env, cls);
        SIGAR_CHEX;
        JAVA_SIGAR_SET_FIELDS_NETCONNECTION(cls, obj, connlist.data[i]);
        JENV->SetObjectArrayElement(env, connarray, i, obj);
        SIGAR_CHEX;
    }

    sigar_net_connection_list_destroy(sigar, &connlist);

    return connarray;
}

static int jbyteArray_to_sigar_net_address(JNIEnv *env, jbyteArray jaddress,
                                           sigar_net_address_t *address)
{
    jsize len = JENV->GetArrayLength(env, jaddress);

    JENV->GetByteArrayRegion(env, jaddress, 0, len,
                             (jbyte *)&address->addr.in6);

    switch (len) {
      case 4:
        address->family = SIGAR_AF_INET;
        break;
      case 4*4:
        address->family = SIGAR_AF_INET6;
        break;
      default:
        return EINVAL;
    }

    return SIGAR_OK;
}

JNIEXPORT void SIGAR_JNI(NetStat_stat)
(JNIEnv *env, jobject obj, jobject sigar_obj, jint flags,
 jbyteArray jaddress, jlong port)
{
    int status;
    sigar_net_stat_t netstat;
    jclass cls;
    jfieldID id;
    jintArray states;
    jint tcp_states[SIGAR_TCP_UNKNOWN];
    sigar_net_address_t address;
    jboolean has_port = (port != -1);
    dSIGAR_VOID;

    if (has_port) {
        status = jbyteArray_to_sigar_net_address(env, jaddress, &address);
        if (status == SIGAR_OK) {
            status = sigar_net_stat_port_get(sigar, &netstat, flags,
                                             &address, port);
        }
    }
    else {
        status = sigar_net_stat_get(sigar, &netstat, flags);
    }

    if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return;
    }

    cls = JENV->GetObjectClass(env, obj);

    JAVA_SIGAR_INIT_FIELDS_NETSTAT(cls);
    JAVA_SIGAR_SET_FIELDS_NETSTAT(cls, obj, netstat);

    if (sizeof(tcp_states[0]) == sizeof(netstat.tcp_states[0])) {
        memcpy(&tcp_states[0], &netstat.tcp_states[0],
               sizeof(netstat.tcp_states));
    }
    else {
        int i;
        for (i=0; i<SIGAR_TCP_UNKNOWN; i++) {
            tcp_states[i] = netstat.tcp_states[i];
        }
    }

    states = JENV->NewIntArray(env, SIGAR_TCP_UNKNOWN);
    if (JENV->ExceptionCheck(env)) {
        return;
    }

    JENV->SetIntArrayRegion(env, states, 0,
                            SIGAR_TCP_UNKNOWN,
                            tcp_states);

    id = JENV->GetFieldID(env, cls, "tcpStates", "[I");
    JENV->SetObjectField(env, obj, id, states);
}

JNIEXPORT jstring SIGAR_JNIx(getNetListenAddress)
(JNIEnv *env, jobject sigar_obj, jlong port)
{
    int status;
    sigar_net_address_t address;
    dSIGAR(NULL);

    status = sigar_net_listen_address_get(sigar, port, &address);
    if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    return jnet_address_to_string(env, sigar, &address);
}

JNIEXPORT jstring SIGAR_JNIx(getNetServicesName)
(JNIEnv *env, jobject sigar_obj, jint protocol, jlong port)
{
    char *name;
    dSIGAR(NULL);

    if ((name = sigar_net_services_name_get(sigar, protocol, port))) {
        return JENV->NewStringUTF(env, name);
    }
    else {
        return NULL;
    }
}

JNIEXPORT jstring SIGAR_JNI(NetConnection_getTypeString)
(JNIEnv *env, jobject obj)
{
    jclass cls = JENV->GetObjectClass(env, obj);
    jfieldID field = JENV->GetFieldID(env, cls, "type", "I");
    jint type = JENV->GetIntField(env, obj, field);
    return JENV->NewStringUTF(env,
                              sigar_net_connection_type_get(type));
}

JNIEXPORT jstring SIGAR_JNI(NetConnection_getStateString)
(JNIEnv *env, jobject cls, jint state)
{
    return JENV->NewStringUTF(env,
                              sigar_net_connection_state_get(state));
}

JNIEXPORT jobjectArray SIGAR_JNIx(getWhoList)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    sigar_who_list_t wholist;
    jobjectArray whoarray;
    jclass cls = SIGAR_FIND_CLASS("Who");
    dSIGAR(NULL);

    if ((status = sigar_who_list_get(sigar, &wholist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    JAVA_SIGAR_INIT_FIELDS_WHO(cls);

    whoarray = JENV->NewObjectArray(env, wholist.number, cls, 0);
    SIGAR_CHEX;

    for (i=0; i<wholist.number; i++) {
        jobject info_obj = JENV->AllocObject(env, cls);
        SIGAR_CHEX;
        JAVA_SIGAR_SET_FIELDS_WHO(cls, info_obj,
                                  wholist.data[i]);
        JENV->SetObjectArrayElement(env, whoarray, i, info_obj);
        SIGAR_CHEX;
    }

    sigar_who_list_destroy(sigar, &wholist);

    return whoarray;
}

/* XXX perhaps it would be better to duplicate these strings
 * in java land as static final so we dont create a new String
 * everytime.
 */
JNIEXPORT jstring SIGAR_JNI(FileInfo_getTypeString)
(JNIEnv *env, jclass cls, jint type)
{
    return JENV->NewStringUTF(env,
                              sigar_file_attrs_type_string_get(type));
}

JNIEXPORT jstring SIGAR_JNI(FileInfo_getPermissionsString)
(JNIEnv *env, jclass cls, jlong perms)
{
    char str[24];
    return JENV->NewStringUTF(env,
                              sigar_file_attrs_permissions_string_get(perms,
                                                                      str));
}

JNIEXPORT jint SIGAR_JNI(FileInfo_getMode)
(JNIEnv *env, jclass cls, jlong perms)
{
    return sigar_file_attrs_mode_get(perms);
}


/*
 * copy of the generated FileAttrs_gather function
 * but we call the lstat wrapper instead.
 */
JNIEXPORT void SIGAR_JNI(FileInfo_gatherLink)
(JNIEnv *env, jobject obj, jobject sigar_obj, jstring name)
{
    sigar_file_attrs_t s;
    int status;
    jclass cls = JENV->GetObjectClass(env, obj);
    const char *utf;
    dSIGAR_VOID;

    utf = JENV->GetStringUTFChars(env, name, 0);

    status = sigar_link_attrs_get(sigar, utf, &s);

    JENV->ReleaseStringUTFChars(env, name, utf);

    if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return;
    }

    JAVA_SIGAR_INIT_FIELDS_FILEATTRS(cls);

    JAVA_SIGAR_SET_FIELDS_FILEATTRS(cls, obj, s);
}

JNIEXPORT jlong SIGAR_JNIx(getProcPort)
(JNIEnv *env, jobject sigar_obj, jint protocol, jlong port)
{
    int status;
    sigar_pid_t pid;
    dSIGAR(0);

    status = sigar_proc_port_get(sigar, protocol,
                                 (unsigned long)port, &pid);
    if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return -1;
    }

    return pid;
}

JNIEXPORT jobjectArray SIGAR_JNIx(getNetInterfaceList)
(JNIEnv *env, jobject sigar_obj)
{
    int status;
    unsigned int i;
    sigar_net_interface_list_t iflist;
    jobjectArray ifarray;
    jclass stringclass = JENV->FindClass(env, "java/lang/String");
    dSIGAR(NULL);

    if ((status = sigar_net_interface_list_get(sigar, &iflist)) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    ifarray = JENV->NewObjectArray(env, iflist.number, stringclass, 0);
    SIGAR_CHEX;

    for (i=0; i<iflist.number; i++) {
        jstring s = JENV->NewStringUTF(env, iflist.data[i]);
        JENV->SetObjectArrayElement(env, ifarray, i, s);
        SIGAR_CHEX;
    }

    sigar_net_interface_list_destroy(sigar, &iflist);

    return ifarray;
}

JNIEXPORT jstring SIGAR_JNIx(getPasswordNative)
(JNIEnv *env, jclass classinstance, jstring prompt)
{
    const char *prompt_str;
    char *password;

    if (getenv("NO_NATIVE_GETPASS")) {
        sigar_throw_notimpl(env, "disabled with $NO_NATIVE_GETPASS");
        return NULL;
    }

    prompt_str = JENV->GetStringUTFChars(env, prompt, 0);

    password = sigar_password_get(prompt_str);

    JENV->ReleaseStringUTFChars(env, prompt, prompt_str);

    return JENV->NewStringUTF(env, password);
}

JNIEXPORT jstring SIGAR_JNIx(getFQDN)
(JNIEnv *env, jobject sigar_obj)
{
    char fqdn[SIGAR_FQDN_LEN];
    int status;
    dSIGAR(NULL);

    if ((status = sigar_fqdn_get(sigar, fqdn, sizeof(fqdn))) != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    return JENV->NewStringUTF(env, fqdn);
}

typedef struct {
    JNIEnv *env;
    jobject obj;
    jclass cls;
    jmethodID id;    
} jni_ptql_re_data_t;

static int jsigar_ptql_re_impl(void *data,
                               char *haystack, char *needle)
{
    jni_ptql_re_data_t *re = (jni_ptql_re_data_t *)data;
    JNIEnv *env = re->env;

    if (!re->cls) {
        re->cls = JENV->GetObjectClass(env, re->obj);
        re->id =
            JENV->GetStaticMethodID(env, re->cls, "re",
                                    "(Ljava/lang/String;Ljava/lang/String;)"
                                    "Z");
        if (!re->id) {
            return 0;
        }
    }

    return JENV->CallStaticBooleanMethod(env, re->cls, re->id,  
                                         JENV->NewStringUTF(env, haystack),
                                         JENV->NewStringUTF(env, needle));
}

static void re_impl_set(JNIEnv *env, sigar_t *sigar, jobject obj, jni_ptql_re_data_t *re)
{
    re->env = env;
    re->cls = NULL;
    re->obj = obj;
    re->id = NULL;

    sigar_ptql_re_impl_set(sigar, re, jsigar_ptql_re_impl);
}

JNIEXPORT jboolean SIGAR_JNI(ptql_SigarProcessQuery_match)
(JNIEnv *env, jobject obj, jobject sigar_obj, jlong pid)
{
    int status;
    jni_ptql_re_data_t re;
    sigar_ptql_query_t *query =
        (sigar_ptql_query_t *)sigar_get_pointer(env, obj);
    dSIGAR(JNI_FALSE);

    re_impl_set(env, sigar, obj, &re);

    status = sigar_ptql_query_match(sigar, query, pid);

    sigar_ptql_re_impl_set(sigar, NULL, NULL);

    if (status == SIGAR_OK) {
        return JNI_TRUE;
    }
    else {
        return JNI_FALSE;
    }
}

JNIEXPORT void SIGAR_JNI(ptql_SigarProcessQuery_create)
(JNIEnv *env, jobject obj, jstring jptql)
{
    int status;
    jboolean is_copy;
    const char *ptql;
    sigar_ptql_query_t *query;
    sigar_ptql_error_t error;

    ptql = JENV->GetStringUTFChars(env, jptql, &is_copy);
    status = sigar_ptql_query_create(&query, (char *)ptql, &error);
    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jptql, ptql);
    }

    if (status != SIGAR_OK) {
        sigar_throw_ptql_malformed(env, error.message);
    }
    else {
        sigar_set_pointer(env, obj, query);
    }
}

JNIEXPORT void SIGAR_JNI(ptql_SigarProcessQuery_destroy)
(JNIEnv *env, jobject obj)
{
    sigar_ptql_query_t *query =
        (sigar_ptql_query_t *)sigar_get_pointer(env, obj);

    if (query) {
        sigar_ptql_query_destroy(query);
        sigar_set_pointer(env, obj, 0);
    }
}

JNIEXPORT jlong SIGAR_JNI(ptql_SigarProcessQuery_findProcess)
(JNIEnv *env, jobject obj, jobject sigar_obj)
{
    sigar_pid_t pid;
    int status;
    jni_ptql_re_data_t re;
    sigar_ptql_query_t *query =
        (sigar_ptql_query_t *)sigar_get_pointer(env, obj);
    dSIGAR(0);

    re_impl_set(env, sigar, obj, &re);

    status = sigar_ptql_query_find_process(sigar, query, &pid);

    sigar_ptql_re_impl_set(sigar, NULL, NULL);

    if (status < 0) {
        sigar_throw_exception(env, sigar->errbuf);
    }
    else if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
    }

    return pid;
}

JNIEXPORT jlongArray SIGAR_JNI(ptql_SigarProcessQuery_find)
(JNIEnv *env, jobject obj, jobject sigar_obj)
{
    int status;
    jlongArray procarray;
    sigar_proc_list_t proclist;
    jlong *pids = NULL;
    jni_ptql_re_data_t re;
    sigar_ptql_query_t *query =
        (sigar_ptql_query_t *)sigar_get_pointer(env, obj);
    dSIGAR(NULL);

    re_impl_set(env, sigar, obj, &re);

    status = sigar_ptql_query_find(sigar, query, &proclist);

    sigar_ptql_re_impl_set(sigar, NULL, NULL);

    if (status < 0) {
        sigar_throw_exception(env, sigar->errbuf);
        return NULL;
    }
    else if (status != SIGAR_OK) {
        sigar_throw_error(env, jsigar, status);
        return NULL;
    }

    procarray = JENV->NewLongArray(env, proclist.number);
    SIGAR_CHEX;

    if (sizeof(jlong) == sizeof(sigar_pid_t)) {
        pids = (jlong *)proclist.data;
    }
    else {
        unsigned int i;
        pids = (jlong *)malloc(sizeof(jlong) * proclist.number);

        for (i=0; i<proclist.number; i++) {
            pids[i] = proclist.data[i];
        }
    }

    JENV->SetLongArrayRegion(env, procarray, 0,
                             proclist.number, pids);

    if (pids != (jlong *)proclist.data) {
        free(pids);
    }

    sigar_proc_list_destroy(sigar, &proclist);

    return procarray;
}

#include "sigar_getline.h"

JNIEXPORT jboolean SIGAR_JNI(util_Getline_isatty)
(JNIEnv *env, jclass cls)
{
    return sigar_isatty(sigar_fileno(stdin)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jstring SIGAR_JNI(util_Getline_getline)
(JNIEnv *env, jobject sigar_obj, jstring prompt)
{
    const char *prompt_str;
    char *line;
    jboolean is_copy;

    prompt_str = JENV->GetStringUTFChars(env, prompt, &is_copy);

    line = sigar_getline((char *)prompt_str);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, prompt, prompt_str);
    }

    if ((line == NULL) ||
        sigar_getline_eof())
    {
        jclass eof_ex = JENV->FindClass(env, "java/io/EOFException");
        JENV->ThrowNew(env, eof_ex, "");
        return NULL;
    }

    return JENV->NewStringUTF(env, line);
}

JNIEXPORT void SIGAR_JNI(util_Getline_histadd)
(JNIEnv *env, jobject sigar_obj, jstring hist)
{
    const char *hist_str;
    jboolean is_copy;

    hist_str = JENV->GetStringUTFChars(env, hist, &is_copy);

    sigar_getline_histadd((char *)hist_str);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, hist, hist_str);
    }
}

JNIEXPORT void SIGAR_JNI(util_Getline_histinit)
(JNIEnv *env, jobject sigar_obj, jstring hist)
{
    const char *hist_str;
    jboolean is_copy;

    hist_str = JENV->GetStringUTFChars(env, hist, &is_copy);

    sigar_getline_histinit((char *)hist_str);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, hist, hist_str);
    }
}

static struct {
    JNIEnv *env;
    jobject obj;
    jmethodID id;
    jclass clazz;
} jsigar_completer;

static int jsigar_getline_completer(char *buffer, int offset, int *pos)
{
    JNIEnv *env = jsigar_completer.env;
    jstring jbuffer;
    jstring completion;
    const char *line;
    int len, cur;
    jboolean is_copy;

    jbuffer = JENV->NewStringUTF(env, buffer);

    completion = 
        JENV->CallObjectMethod(env, jsigar_completer.obj,
                               jsigar_completer.id, jbuffer);

    if (JENV->ExceptionCheck(env)) {
        JENV->ExceptionDescribe(env);
        return 0;
    }

    if (!completion) {
        return 0;
    }

    line = JENV->GetStringUTFChars(env, completion, &is_copy);
    len = JENV->GetStringUTFLength(env, completion);

    cur = *pos;

    if (len != cur) {
        strcpy(buffer, line);
        *pos = len;
    }

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, completion, line);
    }

    return cur;
}

JNIEXPORT void SIGAR_JNI(util_Getline_setCompleter)
(JNIEnv *env, jclass classinstance, jobject completer)
{
    if (completer == NULL) {
        sigar_getline_completer_set(NULL);
        return;
    }
    
    jsigar_completer.env = env;
    jsigar_completer.obj = completer;
    jsigar_completer.clazz = JENV->GetObjectClass(env, completer);
    jsigar_completer.id =
        JENV->GetMethodID(env, jsigar_completer.clazz,
                          "complete",
                          "(Ljava/lang/String;)Ljava/lang/String;");

    sigar_getline_completer_set(jsigar_getline_completer);
}

JNIEXPORT void SIGAR_JNI(util_Getline_redraw)
(JNIEnv *env, jobject obj)
{
    sigar_getline_redraw();
}

JNIEXPORT void SIGAR_JNI(util_Getline_reset)
(JNIEnv *env, jobject obj)
{
    sigar_getline_reset();
}

static const char *log_methods[] = {
    "fatal", /* SIGAR_LOG_FATAL */
    "error", /* SIGAR_LOG_ERROR */
    "warn",  /* SIGAR_LOG_WARN */
    "info",  /* SIGAR_LOG_INFO */
    "debug", /* SIGAR_LOG_DEBUG */
    /* XXX trace is only in commons-logging??? */
    "debug", /* SIGAR_LOG_TRACE */
};

static void jsigar_log_impl(sigar_t *sigar, void *data,
                            int level, char *message)
{
    jni_sigar_t *jsigar = (jni_sigar_t *)data;
    JNIEnv *env = jsigar->env;
    jobject logger = jsigar->logger;
    jobject message_obj;

    /* XXX should cache method id lookups */
    jmethodID id =
        JENV->GetMethodID(env, JENV->GetObjectClass(env, logger),
                          log_methods[level],
                          "(Ljava/lang/Object;)V");

    if (JENV->ExceptionCheck(env)) {
        JENV->ExceptionDescribe(env);
        return;
    }

    message_obj = (jobject)JENV->NewStringUTF(env, message);

    JENV->CallVoidMethod(env, logger, id, message_obj);
}

JNIEXPORT void SIGAR_JNI(SigarLog_setLogger)
(JNIEnv *env, jclass classinstance, jobject sigar_obj, jobject logger)
{
    dSIGAR_VOID;

    if (jsigar->logger != NULL) {
        JENV->DeleteGlobalRef(env, jsigar->logger);
        jsigar->logger = NULL;
    }

    if (logger) {
        jsigar->logger = JENV->NewGlobalRef(env, logger);

        sigar_log_impl_set(sigar, jsigar, jsigar_log_impl);
    }
    else {
        sigar_log_impl_set(sigar, NULL, NULL);
    }
}

JNIEXPORT void SIGAR_JNI(SigarLog_setLevel)
(JNIEnv *env, jclass classinstance, jobject sigar_obj, jint level)
{
    dSIGAR_VOID;

    sigar_log_level_set(sigar, level);
}

JNIEXPORT jlong SIGAR_JNIx(getServicePid)
(JNIEnv *env, jobject sigar_obj, jstring jname)
{
#ifdef WIN32
    const char *name;
    jboolean is_copy;
    jlong pid = 0;
    int status;
    dSIGAR(0);

    name = JENV->GetStringUTFChars(env, jname, &is_copy);

    status =
        sigar_service_pid_get(sigar, (char *)name, &pid);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jname, name);
    }

    if (status != ERROR_SUCCESS) {
        sigar_throw_error(env, jsigar, status);
    }

    return pid;
#else
    dSIGAR(0);
    sigar_throw_error(env, jsigar, SIGAR_ENOTIMPL);
    return 0;
#endif
}

JNIEXPORT jlong SIGAR_JNI(ResourceLimit_INFINITY)
(JNIEnv *env, jclass cls)
{
#ifdef WIN32
    return 0x7fffffff;
#else
    return RLIM_INFINITY;
#endif
}

JNIEXPORT jstring SIGAR_JNI(win32_Win32_findExecutable)
(JNIEnv *env, jclass sigar_class, jstring jname)
{
#ifdef WIN32
#include "shellapi.h"
    const char *name;
    jboolean is_copy;
    char exe[MAX_PATH];
    LONG result;
    jstring jexe = NULL;

    name = JENV->GetStringUTFChars(env, jname, &is_copy);

    if ((result = (LONG)FindExecutable(name, ".", exe)) > 32) {
        jexe = JENV->NewStringUTF(env, exe);
    }

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jname, name);
    }

    return jexe;
#else
    sigar_throw_notimpl(env, "win32 only");
    return NULL;
#endif
}

JNIEXPORT jboolean SIGAR_JNI(win32_FileVersion_gather)
(JNIEnv *env, jobject obj, jstring jname)
{
#ifdef WIN32
    int status;
    sigar_file_version_t version;
    jboolean is_copy;
    jfieldID id;
    jclass cls = JENV->GetObjectClass(env, obj);
    const char *name = JENV->GetStringUTFChars(env, jname, &is_copy);
    sigar_proc_env_t infocb;
    jobject hashmap;
    jni_env_put_t put;

    id = JENV->GetFieldID(env, cls, "string_file_info",
                          "Ljava/util/Map;");
    hashmap = JENV->GetObjectField(env, obj, id);
    put.env = env;
    put.id = 
        JENV->GetMethodID(env,
                          JENV->GetObjectClass(env, hashmap),
                          "put", MAP_PUT_SIG);
    put.map = hashmap;

    infocb.type = SIGAR_PROC_ENV_ALL;
    infocb.env_getter = jni_env_getall;
    infocb.data = &put;

    status = sigar_file_version_get(&version, (char *)name, &infocb);

    if (is_copy) {
        JENV->ReleaseStringUTFChars(env, jname, name);
    }

    if (status != SIGAR_OK) {
        return JNI_FALSE;
    }

#define set_vfield(name) \
    id = JENV->GetFieldID(env, cls, #name, "I"); \
    JENV->SetIntField(env, obj, id, version.name)

    set_vfield(product_major);
    set_vfield(product_minor);
    set_vfield(product_build);
    set_vfield(product_revision);
    set_vfield(file_major);
    set_vfield(file_minor);
    set_vfield(file_build);
    set_vfield(file_revision);
#undef set_vfield

    return JNI_TRUE;
#else
    return JNI_FALSE;
#endif
}
