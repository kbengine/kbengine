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

#ifndef SIGAR_OS_H
#define SIGAR_OS_H

#ifdef DARWIN
#include <mach/port.h>
#include <mach/host_info.h>
#ifdef DARWIN_HAS_LIBPROC_H
#include <mach-o/dyld.h>
#include <libproc.h>
typedef int (*proc_pidinfo_func_t)(int, int, uint64_t,  void *, int);
typedef int (*proc_pidfdinfo_func_t)(int, int, int, void *, int); 
#endif
#else
#include <kvm.h>
#endif

#ifdef __NetBSD__
#include <sys/param.h>
#endif
#include <sys/sysctl.h>

enum {
    KOFFSET_CPUINFO,
    KOFFSET_VMMETER,
#if defined(__OpenBSD__) || defined(__NetBSD__)
    KOFFSET_TCPSTAT,
    KOFFSET_TCBTABLE,
#endif
    KOFFSET_MAX
};

#if defined(__OpenBSD__) || defined(__NetBSD__)
typedef struct kinfo_proc2 bsd_pinfo_t;
#else
typedef struct kinfo_proc bsd_pinfo_t;
#endif

struct sigar_t {
    SIGAR_T_BASE;
    int pagesize;
    time_t last_getprocs;
    sigar_pid_t last_pid;
    bsd_pinfo_t *pinfo;
    int lcpu;
    size_t argmax;
#ifdef DARWIN
    mach_port_t mach_port;
#  ifdef DARWIN_HAS_LIBPROC_H
    void *libproc;
    proc_pidinfo_func_t proc_pidinfo;
    proc_pidfdinfo_func_t proc_pidfdinfo;
#  endif
#else
    kvm_t *kmem;
    /* offsets for seeking on kmem */
    unsigned long koffsets[KOFFSET_MAX];
    int proc_mounted;
#endif
};

#define SIGAR_EPERM_KMEM (SIGAR_OS_START_ERROR+EACCES)
#define SIGAR_EPROC_NOENT (SIGAR_OS_START_ERROR+2)

#endif /* SIGAR_OS_H */
