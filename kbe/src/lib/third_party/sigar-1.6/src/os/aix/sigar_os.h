/*
 * Copyright (c) 2004-2007, 2009 Hyperic, Inc.
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

#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>
#include <procinfo.h>
#include <sys/resource.h>

enum {
    KOFFSET_LOADAVG,
    KOFFSET_VAR,
    KOFFSET_SYSINFO,
    KOFFSET_IFNET,
    KOFFSET_VMINFO,
    KOFFSET_CPUINFO,
    KOFFSET_TCB,
    KOFFSET_MAX
};

typedef struct {
    time_t mtime;
    int num;
    char **devs;
} swaps_t;

typedef int (*proc_fd_func_t) (sigar_t *, sigar_pid_t, sigar_proc_fd_t *);

struct sigar_t {
    SIGAR_T_BASE;
    int kmem;
    /* offsets for seeking on kmem */
    long koffsets[KOFFSET_MAX];
    proc_fd_func_t getprocfd;
    int pagesize;
    swaps_t swaps;
    time_t last_getprocs;
    sigar_pid_t last_pid;
    struct procsinfo64 *pinfo;
    struct cpuinfo *cpuinfo;
    int cpuinfo_size;
    int cpu_mhz;
    char model[128];
    int aix_version;
    int thrusage;
    sigar_cache_t *diskmap; 
};

#define HAVE_STRERROR_R

#define SIGAR_EPERM_KMEM (SIGAR_OS_START_ERROR+EACCES)

#endif /* SIGAR_OS_H */
