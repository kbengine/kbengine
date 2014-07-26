/*
 * Copyright (c) 2004-2008 Hyperic, Inc.
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

#include <assert.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

typedef struct {
    sigar_pid_t pid;
    time_t mtime;
    sigar_uint64_t vsize;
    sigar_uint64_t rss;
    sigar_uint64_t minor_faults;
    sigar_uint64_t major_faults;
    sigar_uint64_t ppid;
    int tty;
    int priority;
    int nice;
    sigar_uint64_t start_time;
    sigar_uint64_t utime;
    sigar_uint64_t stime;
    char name[SIGAR_PROC_NAME_LEN];
    char state;
    int processor;
} linux_proc_stat_t;

typedef enum {
    IOSTAT_NONE,
    IOSTAT_PARTITIONS, /* 2.4 */
    IOSTAT_DISKSTATS, /* 2.6 */
    IOSTAT_SYS /* 2.6 */
} linux_iostat_e;

struct sigar_t {
    SIGAR_T_BASE;
    int pagesize;
    int ram;
    int proc_signal_offset;
    linux_proc_stat_t last_proc_stat;
    int lcpu;
    linux_iostat_e iostat;
    char *proc_net;
    /* Native POSIX Thread Library 2.6+ kernel */
    int has_nptl;
};

#define HAVE_STRERROR_R
#ifndef __USE_XOPEN2K
/* use gnu version of strerror_r */
#define HAVE_STRERROR_R_GLIBC
#endif
#define HAVE_READDIR_R
#define HAVE_GETPWNAM_R
#define HAVE_GETPWUID_R
#define HAVE_GETGRGID_R

#endif /* SIGAR_OS_H */
