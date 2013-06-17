/*
 * Copyright (c) 2004-2007 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
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

#ifndef SIGAR_OS_H
#define SIGAR_OS_H

#ifndef _POSIX_PTHREAD_SEMANTICS
#define _POSIX_PTHREAD_SEMANTICS
#endif

typedef unsigned long long int u_int64_t;

#include <ctype.h>
#include <assert.h>
#ifndef DMALLOC
#include <malloc.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/processor.h>
#include <sys/sysinfo.h>
#include <sys/param.h>

#include <kstat.h>
#include <procfs.h>

#include "get_mib2.h"

/* avoid -Wall warning since solaris doesnt have a prototype for this */
int getdomainname(char *, int);

typedef struct {
    kstat_t **ks;
    int num;
    char *name;
    int nlen;
} kstat_list_t;

SIGAR_INLINE kid_t sigar_kstat_update(sigar_t *sigar);

int sigar_get_kstats(sigar_t *sigar);

void sigar_init_multi_kstats(sigar_t *sigar);

void sigar_free_multi_kstats(sigar_t *sigar);

int sigar_get_multi_kstats(sigar_t *sigar,
                           kstat_list_t *kl,
                           const char *name,
                           kstat_t **retval);

void sigar_koffsets_lookup(kstat_t *ksp, int *offsets, int kidx);

int sigar_proc_psinfo_get(sigar_t *sigar, sigar_pid_t pid);

int sigar_proc_usage_get(sigar_t *sigar, prusage_t *prusage, sigar_pid_t pid);

int sigar_proc_status_get(sigar_t *sigar, pstatus_t *pstatus, sigar_pid_t pid);

#define CPU_ONLINE(n) \
    (p_online(n, P_STATUS) == P_ONLINE)

typedef enum {
    KSTAT_SYSTEM_BOOT_TIME,
    KSTAT_SYSTEM_LOADAVG_1,
    KSTAT_SYSTEM_LOADAVG_2,
    KSTAT_SYSTEM_LOADAVG_3,
    KSTAT_SYSTEM_MAX
} kstat_system_off_e;

typedef enum {
    KSTAT_MEMPAGES_ANON,
    KSTAT_MEMPAGES_EXEC,
    KSTAT_MEMPAGES_VNODE,
    KSTAT_MEMPAGES_MAX
} kstat_mempages_off_e;

typedef enum {
    KSTAT_SYSPAGES_FREE,
    KSTAT_SYSPAGES_MAX
} kstat_syspages_off_e;

enum {
    KSTAT_KEYS_system,
    KSTAT_KEYS_mempages,
    KSTAT_KEYS_syspages,
} kstat_keys_e;

typedef struct ps_prochandle * (*proc_grab_func_t)(pid_t, int, int *);

typedef void (*proc_free_func_t)(struct ps_prochandle *);

typedef int (*proc_create_agent_func_t)(struct ps_prochandle *);

typedef void (*proc_destroy_agent_func_t)(struct ps_prochandle *);

typedef void (*proc_objname_func_t)(struct ps_prochandle *,
                                    uintptr_t, const char *, size_t);

typedef char * (*proc_dirname_func_t)(const char *, char *, size_t);

typedef char * (*proc_exename_func_t)(struct ps_prochandle *, char *, size_t);

typedef int (*proc_fstat64_func_t)(struct ps_prochandle *, int, void *);

typedef int (*proc_getsockopt_func_t)(struct ps_prochandle *,
                                     int, int, int, void *, int *);

typedef int (*proc_getsockname_func_t)(struct ps_prochandle *,
                                      int, struct sockaddr *, socklen_t *);

struct sigar_t {
    SIGAR_T_BASE;

    int solaris_version;
    int use_ucb_ps;

    kstat_ctl_t *kc;

    /* kstat_lookup() as needed */
    struct {
        kstat_t **cpu;
        kstat_t **cpu_info;
        processorid_t *cpuid;
        unsigned int lcpu; /* number malloced slots in the cpu array above */
        kstat_t *system;
        kstat_t *syspages;
        kstat_t *mempages;
    } ks;

    struct {
        int system[KSTAT_SYSTEM_MAX];
        int mempages[KSTAT_MEMPAGES_MAX];
        int syspages[KSTAT_SYSPAGES_MAX];
    } koffsets;
    
    int pagesize;

    time_t last_getprocs;
    sigar_pid_t last_pid;
    psinfo_t *pinfo;
    sigar_cpu_list_t cpulist;

    /* libproc.so interface */
    void *plib;
    proc_grab_func_t pgrab;
    proc_free_func_t pfree;
    proc_create_agent_func_t pcreate_agent;
    proc_destroy_agent_func_t pdestroy_agent;
    proc_objname_func_t pobjname;
    proc_dirname_func_t pdirname;
    proc_exename_func_t pexename;
    proc_fstat64_func_t pfstat64;
    proc_getsockopt_func_t pgetsockopt;
    proc_getsockname_func_t pgetsockname;

    sigar_cache_t *pargs;

    solaris_mib2_t mib2;
};

#ifdef SIGAR_64BIT
#define KSTAT_UINT ui64
#else
#define KSTAT_UINT ui32
#endif

#define kSTAT_exists(v, type) \
    (sigar->koffsets.type[v] != -2)

#define kSTAT_ptr(v, type) \
    ((kstat_named_t *)ksp->ks_data + sigar->koffsets.type[v])

#define kSTAT_uint(v, type) \
    (kSTAT_exists(v, type) ? kSTAT_ptr(v, type)->value.KSTAT_UINT : 0)

#define kSTAT_ui32(v, type) \
    (kSTAT_exists(v, type) ? kSTAT_ptr(v, type)->value.ui32 : 0)

#define kSYSTEM(v) kSTAT_ui32(v, system)

#define kMEMPAGES(v) kSTAT_uint(v, mempages)

#define kSYSPAGES(v) kSTAT_uint(v, syspages)

#define sigar_koffsets_init(sigar, ksp, type) \
    if (sigar->koffsets.type[0] == -1) \
        sigar_koffsets_lookup(ksp, sigar->koffsets.type, KSTAT_KEYS_##type)

#define sigar_koffsets_init_system(sigar, ksp) \
    sigar_koffsets_init(sigar, ksp, system)

#define sigar_koffsets_init_mempages(sigar, ksp) \
    sigar_koffsets_init(sigar, ksp, mempages)

#define sigar_koffsets_init_syspages(sigar, ksp) \
    sigar_koffsets_init(sigar, ksp, syspages)

#define HAVE_READDIR_R
#define HAVE_GETPWNAM_R
#define HAVE_GETPWUID_R

#define SIGAR_EMIB2 (SIGAR_OS_START_ERROR+1)

#endif /* SIGAR_OS_H */

