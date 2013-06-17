/*
 * Copyright (c) 2004-2007 Hyperic, Inc.
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

#if defined(__ia64) && !defined(__ia64__)
#define __ia64__
#endif

#ifdef __ia64__
#ifndef _LP64
#define _LP64
#endif
#endif

#define _PSTAT64

#include <sys/pstat.h>
#include <sys/mib.h>
#include <stdlib.h>
#include <fcntl.h>

struct sigar_t {
    SIGAR_T_BASE;
    struct pst_static pstatic;
    time_t last_getprocs;
    sigar_pid_t last_pid;
    struct pst_status *pinfo;

    int mib;
};

int hpux_get_mib_ifentry(int ppa, mib_ifEntry *mib);

#endif /* SIGAR_OS_H */
