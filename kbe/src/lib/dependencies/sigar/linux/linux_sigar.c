/*
 * Copyright (c) 2004-2009 Hyperic, Inc.
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

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/sysmacros.h>

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include "sigar_os.h"


#define pageshift(x) ((x) << sigar->pagesize)

#define PROC_MEMINFO PROC_FS_ROOT "meminfo"
#define PROC_VMSTAT  PROC_FS_ROOT "vmstat"
#define PROC_MTRR    PROC_FS_ROOT "mtrr"
#define PROC_STAT    PROC_FS_ROOT "stat"
#define PROC_UPTIME  PROC_FS_ROOT "uptime"
#define PROC_LOADAVG PROC_FS_ROOT "loadavg"

#define PROC_PSTAT   "/stat"
#define PROC_PSTATUS "/status"

#define SYS_BLOCK "/sys/block"
#define PROC_PARTITIONS PROC_FS_ROOT "partitions"
#define PROC_DISKSTATS  PROC_FS_ROOT "diskstats"

/*
 * /proc/self/stat fields:
 * 1 - pid
 * 2 - comm
 * 3 - state
 * 4 - ppid
 * 5 - pgrp
 * 6 - session
 * 7 - tty_nr
 * 8 - tpgid
 * 9 - flags
 * 10 - minflt
 * 11 - cminflt
 * 12 - majflt
 * 13 - cmajflt
 * 14 - utime
 * 15 - stime
 * 16 - cutime
 * 17 - cstime
 * 18 - priority
 * 19 - nice
 * 20 - 0 (removed field)
 * 21 - itrealvalue
 * 22 - starttime
 * 23 - vsize
 * 24 - rss
 * 25 - rlim
 * 26 - startcode
 * 27 - endcode
 * 28 - startstack
 * 29 - kstkesp
 * 30 - kstkeip
 * 31 - signal
 * 32 - blocked
 * 33 - sigignore
 * 34 - sigcache
 * 35 - wchan
 * 36 - nswap
 * 37 - cnswap
 * 38 - exit_signal <-- looking for this.
 * 39 - processor
 * ... more for newer RH
 */

#define PROC_SIGNAL_IX 38

static int get_proc_signal_offset(void)
{
    char buffer[BUFSIZ], *ptr=buffer;
    int fields = 0;
    int status = sigar_file2str(PROCP_FS_ROOT "self/stat",
                                buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return 1;
    }

    while (*ptr) {
        if (*ptr++ == ' ') {
            fields++;
        }
    }

    return (fields - PROC_SIGNAL_IX) + 1;
}

sigar_pid_t sigar_pid_get(sigar_t *sigar)
{
    /* XXX cannot safely cache getpid unless using nptl */
    /* we can however, cache it for optimizations in the
     * case of proc_env_get for example.
     */
    sigar->pid = getpid();
    return sigar->pid;
}

static int sigar_boot_time_get(sigar_t *sigar)
{
    FILE *fp;
    char buffer[BUFSIZ], *ptr;
    int found = 0;

    if (!(fp = fopen(PROC_STAT, "r"))) {
        return errno;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        if (strnEQ(ptr, "btime", 5)) {
            if ((ptr = sigar_skip_token(ptr))) {
                sigar->boot_time = sigar_strtoul(ptr);
                found = 1;
            }
            break;
        }
    }

    fclose(fp);

    if (!found) {
        /* should never happen */
        sigar->boot_time = time(NULL);
    }

    return SIGAR_OK;
}

int sigar_os_open(sigar_t **sigar)
{
    int i, status;
    int kernel_rev, has_nptl;
    struct stat sb;
    struct utsname name;

    *sigar = malloc(sizeof(**sigar));

    (*sigar)->pagesize = 0;
    i = getpagesize();
    while ((i >>= 1) > 0) {
        (*sigar)->pagesize++;
    }

    status = sigar_boot_time_get(*sigar);
    if (status != SIGAR_OK) {
        return status;
    }
    
    (*sigar)->ticks = sysconf(_SC_CLK_TCK);

    (*sigar)->ram = -1;

    (*sigar)->proc_signal_offset = -1;

    (*sigar)->last_proc_stat.pid = -1;

    (*sigar)->lcpu = -1;

    if (stat(PROC_DISKSTATS, &sb) == 0) {
        (*sigar)->iostat = IOSTAT_DISKSTATS;
    }
    else if (stat(SYS_BLOCK, &sb) == 0) {
        (*sigar)->iostat = IOSTAT_SYS;
    }
    else if (stat(PROC_PARTITIONS, &sb) == 0) {
        /* XXX file exists does not mean is has the fields */
        (*sigar)->iostat = IOSTAT_PARTITIONS;
    }
    else {
        (*sigar)->iostat = IOSTAT_NONE;
    }

    /* hook for using mirrored /proc/net/tcp file */
    (*sigar)->proc_net = getenv("SIGAR_PROC_NET");

    uname(&name);
    /* 2.X.y.z -> just need X (unless there is ever a kernel version 3!) */
    kernel_rev = atoi(&name.release[2]);
    if (kernel_rev >= 6) {
        has_nptl = 1;
    }
    else {
        has_nptl = getenv("SIGAR_HAS_NPTL") ? 1 : 0;
    }
    (*sigar)->has_nptl = has_nptl;

    return SIGAR_OK;
}

int sigar_os_close(sigar_t *sigar)
{
    free(sigar);
    return SIGAR_OK;
}

char *sigar_os_error_string(sigar_t *sigar, int err)
{
    return NULL;
}

static int sigar_cpu_total_count(sigar_t *sigar)
{
    sigar->ncpu = (int)sysconf(_SC_NPROCESSORS_CONF);
    sigar_log_printf(sigar, SIGAR_LOG_DEBUG, "[cpu] ncpu=%d\n",
                     sigar->ncpu);
    return sigar->ncpu;
}

static int get_ram(sigar_t *sigar, sigar_mem_t *mem)
{
    char buffer[BUFSIZ], *ptr;
    FILE *fp;
    int total = 0;
    sigar_uint64_t sys_total = (mem->total / (1024 * 1024));

    if (sigar->ram > 0) {
        /* return cached value */
        mem->ram = sigar->ram;
        return SIGAR_OK;
    }

    if (sigar->ram == 0) {
        return ENOENT;
    }

    /*
     * Memory Type Range Registers
     * write-back registers add up to the total.
     * Well, they are supposed to add up, but seen
     * at least one configuration where that is not the
     * case.
     */
    if (!(fp = fopen(PROC_MTRR, "r"))) {
        return errno;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        if (!(ptr = strstr(ptr, "size="))) {
            continue;
        }

        if (!strstr(ptr, "write-back")) {
            continue;
        }

        ptr += 5;
        while (sigar_isspace(*ptr)) {
            ++ptr;
        }

        total += atoi(ptr);
    }

    fclose(fp);

    if ((total - sys_total) > 256) {
        /* mtrr write-back registers are way off
         * kernel should not be using more that 256MB of mem
         */
        total = 0; /* punt */
    }

    if (total == 0) {
        return ENOENT;
    }
 
    mem->ram = sigar->ram = total;

    return SIGAR_OK;
}

#define MEMINFO_PARAM(a) a ":", SSTRLEN(a ":")

static SIGAR_INLINE sigar_uint64_t sigar_meminfo(char *buffer,
                                                 char *attr, int len)
{
    sigar_uint64_t val = 0;
    char *ptr, *tok;

    if ((ptr = strstr(buffer, attr))) {
        ptr += len;
        val = strtoull(ptr, &tok, 0);
        while (*tok == ' ') {
            ++tok;
        }
        if (*tok == 'k') {
            val *= 1024;
        }
        else if (*tok == 'M') {
            val *= (1024 * 1024);
        }
    }

    return val;
}

int sigar_mem_get(sigar_t *sigar, sigar_mem_t *mem)
{
    sigar_uint64_t buffers, cached, kern;
    char buffer[BUFSIZ];

    int status = sigar_file2str(PROC_MEMINFO,
                                buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    mem->total  = sigar_meminfo(buffer, MEMINFO_PARAM("MemTotal"));
    mem->free   = sigar_meminfo(buffer, MEMINFO_PARAM("MemFree"));
    mem->used   = mem->total - mem->free;

    buffers = sigar_meminfo(buffer, MEMINFO_PARAM("Buffers"));
    cached  = sigar_meminfo(buffer, MEMINFO_PARAM("Cached"));

    kern = buffers + cached;
    mem->actual_free = mem->free + kern;
    mem->actual_used = mem->used - kern;

    sigar_mem_calc_ram(sigar, mem);

    if (get_ram(sigar, mem) != SIGAR_OK) {
        /* XXX other options on failure? */
    }

    return SIGAR_OK;
}

int sigar_swap_get(sigar_t *sigar, sigar_swap_t *swap)
{
    char buffer[BUFSIZ], *ptr;

    /* XXX: we open/parse the same file here as sigar_mem_get */
    int status = sigar_file2str(PROC_MEMINFO,
                                buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    swap->total  = sigar_meminfo(buffer, MEMINFO_PARAM("SwapTotal"));
    swap->free   = sigar_meminfo(buffer, MEMINFO_PARAM("SwapFree"));
    swap->used   = swap->total - swap->free;

    swap->page_in = swap->page_out = -1;

    status = sigar_file2str(PROC_VMSTAT,
                            buffer, sizeof(buffer));

    if (status == SIGAR_OK) {
        /* 2.6+ kernel */
        if ((ptr = strstr(buffer, "\npswpin"))) {
            ptr = sigar_skip_token(ptr);
            swap->page_in = sigar_strtoull(ptr);
            ptr = sigar_skip_token(ptr);
            swap->page_out = sigar_strtoull(ptr);
        }
    }
    else {
        /* 2.2, 2.4 kernels */
        status = sigar_file2str(PROC_STAT,
                                buffer, sizeof(buffer));
        if (status != SIGAR_OK) {
            return status;
        }

        if ((ptr = strstr(buffer, "\nswap"))) {
            ptr = sigar_skip_token(ptr);
            swap->page_in = sigar_strtoull(ptr);
            swap->page_out = sigar_strtoull(ptr);
        }
    }

    return SIGAR_OK;
}

static void get_cpu_metrics(sigar_t *sigar, sigar_cpu_t *cpu, char *line)
{
    char *ptr = sigar_skip_token(line); /* "cpu%d" */

    cpu->user += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    cpu->nice += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    cpu->sys  += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    cpu->idle += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    if (*ptr == ' ') {
        /* 2.6+ kernels only */
        cpu->wait += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
        cpu->irq += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
        cpu->soft_irq += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    }
    if (*ptr == ' ') {
        /* 2.6.11+ kernels only */
        cpu->stolen += SIGAR_TICK2MSEC(sigar_strtoull(ptr));
    }
    cpu->total =
        cpu->user + cpu->nice + cpu->sys + cpu->idle +
        cpu->wait + cpu->irq + cpu->soft_irq + cpu->stolen;
}

int sigar_cpu_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    char buffer[BUFSIZ];
    int status = sigar_file2str(PROC_STAT, buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    SIGAR_ZERO(cpu);
    get_cpu_metrics(sigar, cpu, buffer);

    return SIGAR_OK;
}

int sigar_cpu_list_get(sigar_t *sigar, sigar_cpu_list_t *cpulist)
{
    FILE *fp;
    char buffer[BUFSIZ], cpu_total[BUFSIZ], *ptr;
    int core_rollup = sigar_cpu_core_rollup(sigar), i=0;
    sigar_cpu_t *cpu;

    if (!(fp = fopen(PROC_STAT, "r"))) {
        return errno;
    }

    /* skip first line */
    (void)fgets(cpu_total, sizeof(cpu_total), fp);

    sigar_cpu_list_create(cpulist);

    /* XXX: merge times of logical processors if hyperthreading */
    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        if (!strnEQ(ptr, "cpu", 3)) {
            break;
        }

        if (core_rollup && (i % sigar->lcpu)) {
            /* merge times of logical processors */
            cpu = &cpulist->data[cpulist->number-1];
        }
        else {
            SIGAR_CPU_LIST_GROW(cpulist);
            cpu = &cpulist->data[cpulist->number++];
            SIGAR_ZERO(cpu);
        }

        get_cpu_metrics(sigar, cpu, ptr);

        i++;
    }

    fclose(fp);

    if (cpulist->number == 0) {
        /* likely older kernel where cpu\d is not present */
        cpu = &cpulist->data[cpulist->number++];
        SIGAR_ZERO(cpu);
        get_cpu_metrics(sigar, cpu, cpu_total);
    }

    return SIGAR_OK;
}

int sigar_uptime_get(sigar_t *sigar,
                     sigar_uptime_t *uptime)
{
    char buffer[BUFSIZ], *ptr = buffer;
    int status = sigar_file2str(PROC_UPTIME, buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    uptime->uptime   = strtod(buffer, &ptr);

    return SIGAR_OK;
}

int sigar_loadavg_get(sigar_t *sigar,
                      sigar_loadavg_t *loadavg)
{
    char buffer[BUFSIZ], *ptr = buffer;
    int status = sigar_file2str(PROC_LOADAVG, buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    loadavg->loadavg[0] = strtod(buffer, &ptr);
    loadavg->loadavg[1] = strtod(ptr, &ptr);
    loadavg->loadavg[2] = strtod(ptr, &ptr);

    return SIGAR_OK;
}

/*
 * seems the easiest/fastest way to tell if a process listed in /proc
 * is a thread is to check the "exit signal" flag in /proc/num/stat.
 * any value other than SIGCHLD seems to be a thread.  this make hulk mad.
 * redhat's procps patch (named "threadbadhack.pat") does not use
 * this flag to filter out threads.  instead does much more expensive
 * comparisions.  their patch also bubbles up thread cpu times to the main
 * process.  functionality we currently lack.
 * when nptl is in use, this is not the case and all threads spawned from
 * a process have the same pid.  however, it seems both old-style linux
 * threads and nptl threads can be run on the same machine.
 * there is also the "Tgid" field in /proc/self/status which could be used
 * to detect threads, but this is not available in older kernels.
 */
static SIGAR_INLINE int proc_isthread(sigar_t *sigar, char *pidstr, int len)
{
    char buffer[BUFSIZ], *ptr=buffer;
    int fd, n, offset=sigar->proc_signal_offset;

    /* sprintf(buffer, "/proc/%s/stat", pidstr) */
    memcpy(ptr, PROCP_FS_ROOT, SSTRLEN(PROCP_FS_ROOT));
    ptr += SSTRLEN(PROCP_FS_ROOT);

    memcpy(ptr, pidstr, len);
    ptr += len;

    memcpy(ptr, PROC_PSTAT, SSTRLEN(PROC_PSTAT));
    ptr += SSTRLEN(PROC_PSTAT);

    *ptr = '\0';

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        /* unlikely if pid was from readdir proc */
        return 0;
    }

    n = read(fd, buffer, sizeof(buffer));
    close(fd);

    if (n < 0) {
        return 0; /* chances: slim..none */
    }

    buffer[n--] = '\0';

    /* exit_signal is the second to last field so we look backwards.
     * XXX if newer kernels drop more turds in this file we'll need
     * to go the other way.  luckily linux has no real api for this shit.
     */

    /* skip trailing crap */
    while ((n > 0) && !isdigit(buffer[n--])) ;

    while (offset-- > 0) {
        /* skip last field */
        while ((n > 0) && isdigit(buffer[n--])) ;

        /* skip whitespace */
        while ((n > 0) && !isdigit(buffer[n--])) ;
    }

    if (n < 3) {
        return 0; /* hulk smashed /proc? */
    }

    ptr = &buffer[n];
    /*
     * '17' == SIGCHLD == real process.
     * '33' and '0' are threads
     */
    if ((*ptr++ == '1') &&
        (*ptr++ == '7') &&
        (*ptr++ == ' '))
    {
        return 0;
    }

    return 1;
}

int sigar_os_proc_list_get(sigar_t *sigar,
                           sigar_proc_list_t *proclist)
{
    DIR *dirp = opendir(PROCP_FS_ROOT);
    struct dirent *ent, dbuf;
    register const int threadbadhack = !sigar->has_nptl;

    if (!dirp) {
        return errno;
    }

    if (threadbadhack && (sigar->proc_signal_offset == -1)) {
        sigar->proc_signal_offset = get_proc_signal_offset();
    }

    while (readdir_r(dirp, &dbuf, &ent) == 0) {
        if (!ent) {
            break;
        }

        if (!sigar_isdigit(*ent->d_name)) {
            continue;
        }

        if (threadbadhack &&
            proc_isthread(sigar, ent->d_name, strlen(ent->d_name)))
        {
            continue;
        }

        /* XXX: more sanity checking */

        SIGAR_PROC_LIST_GROW(proclist);

        proclist->data[proclist->number++] =
            strtoul(ent->d_name, NULL, 10);
    }

    closedir(dirp);

    return SIGAR_OK;
}

static int proc_stat_read(sigar_t *sigar, sigar_pid_t pid)
{
    char buffer[BUFSIZ], *ptr=buffer, *tmp;
    unsigned int len;
    linux_proc_stat_t *pstat = &sigar->last_proc_stat;
    int status;

    time_t timenow = time(NULL);

    /* 
     * short-lived cache read/parse of last /proc/pid/stat
     * as this info is spread out across a few functions.
     */
    if (pstat->pid == pid) {
        if ((timenow - pstat->mtime) < SIGAR_LAST_PROC_EXPIRE) {
            return SIGAR_OK;
        }
    }

    pstat->pid = pid;
    pstat->mtime = timenow;

    status = SIGAR_PROC_FILE2STR(buffer, pid, PROC_PSTAT);

    if (status != SIGAR_OK) {
        return status;
    }

    if (!(ptr = strchr(ptr, '('))) {
        return EINVAL;
    }
    if (!(tmp = strrchr(++ptr, ')'))) {
        return EINVAL;
    }
    len = tmp-ptr;

    if (len >= sizeof(pstat->name)) {
        len = sizeof(pstat->name)-1;
    }

    /* (1,2) */
    memcpy(pstat->name, ptr, len);
    pstat->name[len] = '\0';
    ptr = tmp+1;

    SIGAR_SKIP_SPACE(ptr);
    pstat->state = *ptr++; /* (3) */
    SIGAR_SKIP_SPACE(ptr);

    pstat->ppid = sigar_strtoul(ptr); /* (4) */
    ptr = sigar_skip_token(ptr); /* (5) pgrp */
    ptr = sigar_skip_token(ptr); /* (6) session */
    pstat->tty = sigar_strtoul(ptr); /* (7) */
    ptr = sigar_skip_token(ptr); /* (8) tty pgrp */

    ptr = sigar_skip_token(ptr); /* (9) flags */
    pstat->minor_faults = sigar_strtoull(ptr); /* (10) */
    ptr = sigar_skip_token(ptr); /* (11) cmin flt */
    pstat->major_faults = sigar_strtoull(ptr); /* (12) */
    ptr = sigar_skip_token(ptr); /* (13) cmaj flt */

    pstat->utime = SIGAR_TICK2MSEC(sigar_strtoull(ptr)); /* (14) */
    pstat->stime = SIGAR_TICK2MSEC(sigar_strtoull(ptr)); /* (15) */

    ptr = sigar_skip_token(ptr); /* (16) cutime */
    ptr = sigar_skip_token(ptr); /* (17) cstime */

    pstat->priority = sigar_strtoul(ptr); /* (18) */
    pstat->nice     = sigar_strtoul(ptr); /* (19) */

    ptr = sigar_skip_token(ptr); /* (20) timeout */
    ptr = sigar_skip_token(ptr); /* (21) it_real_value */

    pstat->start_time  = sigar_strtoul(ptr); /* (22) */
    pstat->start_time /= sigar->ticks;
    pstat->start_time += sigar->boot_time; /* seconds */
    pstat->start_time *= 1000; /* milliseconds */

    pstat->vsize = sigar_strtoull(ptr); /* (23) */
    pstat->rss   = pageshift(sigar_strtoull(ptr)); /* (24) */

    ptr = sigar_skip_token(ptr); /* (25) rlim */
    ptr = sigar_skip_token(ptr); /* (26) startcode */
    ptr = sigar_skip_token(ptr); /* (27) endcode */
    ptr = sigar_skip_token(ptr); /* (28) startstack */
    ptr = sigar_skip_token(ptr); /* (29) kstkesp */
    ptr = sigar_skip_token(ptr); /* (30) kstkeip */
    ptr = sigar_skip_token(ptr); /* (31) signal */
    ptr = sigar_skip_token(ptr); /* (32) blocked */
    ptr = sigar_skip_token(ptr); /* (33) sigignore */
    ptr = sigar_skip_token(ptr); /* (34) sigcache */
    ptr = sigar_skip_token(ptr); /* (35) wchan */
    ptr = sigar_skip_token(ptr); /* (36) nswap */
    ptr = sigar_skip_token(ptr); /* (37) cnswap */
    ptr = sigar_skip_token(ptr); /* (38) exit_signal */

    pstat->processor = sigar_strtoul(ptr); /* (39) */

    return SIGAR_OK;
}

int sigar_proc_mem_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_mem_t *procmem)
{
    char buffer[BUFSIZ], *ptr=buffer;
    int status = proc_stat_read(sigar, pid);
    linux_proc_stat_t *pstat = &sigar->last_proc_stat;

    procmem->minor_faults = pstat->minor_faults;
    procmem->major_faults = pstat->major_faults;
    procmem->page_faults =
        procmem->minor_faults + procmem->major_faults;
    
    status = SIGAR_PROC_FILE2STR(buffer, pid, "/statm");

    if (status != SIGAR_OK) {
        return status;
    }

    procmem->size     = pageshift(sigar_strtoull(ptr));
    procmem->resident = pageshift(sigar_strtoull(ptr));
    procmem->share    = pageshift(sigar_strtoull(ptr));

    return SIGAR_OK;
}

#define NO_ID_MSG "[proc_cred] /proc/%lu" PROC_PSTATUS " missing "

int sigar_proc_cred_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_cred_t *proccred)
{
    char buffer[BUFSIZ], *ptr;
    int status = SIGAR_PROC_FILE2STR(buffer, pid, PROC_PSTATUS);

    if (status != SIGAR_OK) {
        return status;
    }

    if ((ptr = strstr(buffer, "\nUid:"))) {
        ptr = sigar_skip_token(ptr);

        proccred->uid  = sigar_strtoul(ptr);
        proccred->euid = sigar_strtoul(ptr);
    }
    else {
        sigar_log_printf(sigar, SIGAR_LOG_WARN,
                         NO_ID_MSG "Uid", pid);
        return ENOENT;
    }

    if ((ptr = strstr(ptr, "\nGid:"))) {
        ptr = sigar_skip_token(ptr);

        proccred->gid  = sigar_strtoul(ptr);
        proccred->egid = sigar_strtoul(ptr);
    }
    else {
        sigar_log_printf(sigar, SIGAR_LOG_WARN,
                         NO_ID_MSG "Gid", pid);
        return ENOENT;
    }

    return SIGAR_OK;
}

int sigar_proc_time_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_time_t *proctime)
{
    int status = proc_stat_read(sigar, pid);
    linux_proc_stat_t *pstat = &sigar->last_proc_stat;

    if (status != SIGAR_OK) {
        return status;
    }

    proctime->user = pstat->utime;
    proctime->sys  = pstat->stime;
    proctime->total = proctime->user + proctime->sys;
    proctime->start_time = pstat->start_time;

    return SIGAR_OK;
}

static int proc_status_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_state_t *procstate)
{
    char buffer[BUFSIZ], *ptr;
    int status = SIGAR_PROC_FILE2STR(buffer, pid, PROC_PSTATUS);

    if (status != SIGAR_OK) {
        return status;
    }

    ptr = strstr(buffer, "\nThreads:");
    if (ptr) {
        /* 2.6+ kernel only */
        ptr = sigar_skip_token(ptr);
        procstate->threads = sigar_strtoul(ptr);
    }
    else {
        procstate->threads = SIGAR_FIELD_NOTIMPL;
    }

    return SIGAR_OK;
}

int sigar_proc_state_get(sigar_t *sigar, sigar_pid_t pid,
                         sigar_proc_state_t *procstate)
{
    int status = proc_stat_read(sigar, pid);
    linux_proc_stat_t *pstat = &sigar->last_proc_stat;

    if (status != SIGAR_OK) {
        return status;
    }

    memcpy(procstate->name, pstat->name, sizeof(procstate->name));
    procstate->state = pstat->state;

    procstate->ppid     = pstat->ppid;
    procstate->tty      = pstat->tty;
    procstate->priority = pstat->priority;
    procstate->nice     = pstat->nice;
    procstate->processor = pstat->processor;

    if (sigar_cpu_core_rollup(sigar)) {
        procstate->processor /= sigar->lcpu;
    }

    proc_status_get(sigar, pid, procstate);

    return SIGAR_OK;
}

int sigar_os_proc_args_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_args_t *procargs)
{
    return sigar_procfs_args_get(sigar, pid, procargs);
}

/* glibc 2.8 XXX use sysconf(_SC_ARG_MAX) */
#ifndef ARG_MAX
#define ARG_MAX 131072
#endif

int sigar_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_env_t *procenv)
{
    int fd;
    char buffer[ARG_MAX]; /* XXX: ARG_MAX == 130k */
    char name[BUFSIZ];
    size_t len;
    char *ptr, *end;

    /* optimize if pid == $$ and type == ENV_KEY */
    SIGAR_PROC_ENV_KEY_LOOKUP();

    (void)SIGAR_PROC_FILENAME(name, pid, "/environ");

    if ((fd = open(name, O_RDONLY)) < 0) {
        if (errno == ENOENT) {
            return ESRCH;
        }
        return errno;
    }

    len = read(fd, buffer, sizeof(buffer));

    close(fd);

    buffer[len] = '\0';
    ptr = buffer;

    end = buffer + len;
    while (ptr < end) {
        char *val = strchr(ptr, '=');
        int klen, vlen, status;
        char key[128]; /* XXX is there a max key size? */

        if (val == NULL) {
            /* not key=val format */
            break;
        }

        klen = val - ptr;
        SIGAR_SSTRCPY(key, ptr);
        key[klen] = '\0';
        ++val;

        vlen = strlen(val);
        status = procenv->env_getter(procenv->data,
                                     key, klen, val, vlen);

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            break;
        }

        ptr += (klen + 1 + vlen + 1);
    }

    return SIGAR_OK;
}

int sigar_proc_fd_get(sigar_t *sigar, sigar_pid_t pid,
                      sigar_proc_fd_t *procfd)
{
    int status =
        sigar_proc_fd_count(sigar, pid, &procfd->total);

    return status;
}

int sigar_proc_exe_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_exe_t *procexe)
{
    int len;
    char name[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(name, pid, "/cwd");

    if ((len = readlink(name, procexe->cwd,
                        sizeof(procexe->cwd)-1)) < 0)
    {
        return errno;
    }

    procexe->cwd[len] = '\0';

    (void)SIGAR_PROC_FILENAME(name, pid, "/exe");

    if ((len = readlink(name, procexe->name,
                        sizeof(procexe->name)-1)) < 0)
    {
        return errno;
    }

    procexe->name[len] = '\0';

    (void)SIGAR_PROC_FILENAME(name, pid, "/root");

    if ((len = readlink(name, procexe->root,
                        sizeof(procexe->root)-1)) < 0)
    {
        return errno;
    }

    procexe->root[len] = '\0';

    return SIGAR_OK;
}

int sigar_proc_modules_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_modules_t *procmods)
{
    FILE *fp;
    char buffer[BUFSIZ], *ptr;
    unsigned long inode, last_inode = 0;

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/maps");

    if (!(fp = fopen(buffer, "r"))) {
        return errno;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        int len, status;
        /* skip region, flags, offset, dev */
        ptr = sigar_skip_multiple_token(ptr, 4);
        inode = sigar_strtoul(ptr);

        if ((inode == 0) || (inode == last_inode)) {
            last_inode = 0;
            continue;
        }

        last_inode = inode;
        SIGAR_SKIP_SPACE(ptr);
        len = strlen(ptr);
        ptr[len-1] = '\0'; /* chop \n */

        status =
            procmods->module_getter(procmods->data,
                                    ptr, len-1);

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            break;
        }
    }
    
    fclose(fp);

    return SIGAR_OK;
}

int sigar_thread_cpu_get(sigar_t *sigar,
                         sigar_uint64_t id,
                         sigar_thread_cpu_t *cpu)
{
    struct tms now;

    if (id != 0) {
        return SIGAR_ENOTIMPL;
    }

    times(&now);

    cpu->user  = SIGAR_TICK2NSEC(now.tms_utime);
    cpu->sys   = SIGAR_TICK2NSEC(now.tms_stime);
    cpu->total = SIGAR_TICK2NSEC(now.tms_utime + now.tms_stime);

    return SIGAR_OK;
}

#include <mntent.h>

int sigar_os_fs_type_get(sigar_file_system_t *fsp)
{
    char *type = fsp->sys_type_name;

    switch (*type) {
      case 'e':
        if (strnEQ(type, "ext", 3)) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'g':
        if (strEQ(type, "gfs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'h':
        if (strEQ(type, "hpfs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'j':
        if (strnEQ(type, "jfs", 3)) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'o':
        if (strnEQ(type, "ocfs", 4)) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'p':
        if (strnEQ(type, "psfs", 4)) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'r':
        if (strEQ(type, "reiserfs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'v':
        if (strEQ(type, "vzfs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'x':
        if (strEQ(type, "xfs") || strEQ(type, "xiafs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
    }

    return fsp->type;
}

int sigar_file_system_list_get(sigar_t *sigar,
                               sigar_file_system_list_t *fslist)
{
    struct mntent ent;
    char buf[1025]; /* buffer for strings within ent */
    FILE *fp;
    sigar_file_system_t *fsp;

    if (!(fp = setmntent(MOUNTED, "r"))) {
        return errno;
    }

    sigar_file_system_list_create(fslist);

    while (getmntent_r(fp, &ent, buf, sizeof(buf))) {
        SIGAR_FILE_SYSTEM_LIST_GROW(fslist);

        fsp = &fslist->data[fslist->number++];

        fsp->type = SIGAR_FSTYPE_UNKNOWN; /* unknown, will be set later */
        SIGAR_SSTRCPY(fsp->dir_name, ent.mnt_dir);
        SIGAR_SSTRCPY(fsp->dev_name, ent.mnt_fsname);
        SIGAR_SSTRCPY(fsp->sys_type_name, ent.mnt_type);
        SIGAR_SSTRCPY(fsp->options, ent.mnt_opts);
        sigar_fs_type_get(fsp);
    }

    endmntent(fp);

    return SIGAR_OK;
}

#define ST_MAJOR(sb) major((sb).st_rdev)
#define ST_MINOR(sb) minor((sb).st_rdev)

static int get_iostat_sys(sigar_t *sigar,
                          const char *dirname,
                          sigar_disk_usage_t *disk,
                          sigar_iodev_t **iodev)
{
    char stat[1025], dev[1025];
    char *name, *ptr, *fsdev;
    int partition, status;

    if (!(*iodev = sigar_iodev_get(sigar, dirname))) {
        return ENXIO;
    }

    name = fsdev = (*iodev)->name;

    if (SIGAR_NAME_IS_DEV(name)) {
        name += SSTRLEN(SIGAR_DEV_PREFIX); /* strip "/dev/" */
    }

    while (!sigar_isdigit(*fsdev)) {
        fsdev++;
    }

    partition = strtoul(fsdev, NULL, 0);
    *fsdev = '\0';

    snprintf(stat, sizeof(stat),
             SYS_BLOCK "/%s/%s%d/stat", name, name, partition);

    status = sigar_file2str(stat, dev, sizeof(dev));
    if (status != SIGAR_OK) {
        return status;
    }

    ptr = dev;
    ptr = sigar_skip_token(ptr);
    disk->reads = sigar_strtoull(ptr);
    ptr = sigar_skip_token(ptr);
    disk->writes = sigar_strtoull(ptr);

    disk->read_bytes  = SIGAR_FIELD_NOTIMPL;
    disk->write_bytes = SIGAR_FIELD_NOTIMPL;
    disk->queue       = SIGAR_FIELD_NOTIMPL;

    return SIGAR_OK;
}

static int get_iostat_proc_dstat(sigar_t *sigar,
                                 const char *dirname,
                                 sigar_disk_usage_t *disk,
                                 sigar_iodev_t **iodev,
                                 sigar_disk_usage_t *device_usage)
{
    FILE *fp;
    char buffer[1025];
    char *ptr;
    struct stat sb;
    int status=ENOENT;

    SIGAR_DISK_STATS_INIT(device_usage);

    if (!(*iodev = sigar_iodev_get(sigar, dirname))) {
        return ENXIO;
    }

    if (stat((*iodev)->name, &sb) < 0) {
        return errno;
    }

    if (SIGAR_LOG_IS_DEBUG(sigar)) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         PROC_DISKSTATS " %s -> %s [%d,%d]",
                         dirname, (*iodev)->name,
                         ST_MAJOR(sb), ST_MINOR(sb));
    }

    if (!(fp = fopen(PROC_DISKSTATS, "r"))) {
        return errno;
    }

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        unsigned long major, minor;

        major = sigar_strtoul(ptr);
        minor = sigar_strtoul(ptr);

        if ((major == ST_MAJOR(sb)) &&
            ((minor == ST_MINOR(sb)) || (minor == 0)))
        {
            int num;
            unsigned long
                rio, rmerge, rsect, ruse,
                wio, wmerge, wsect, wuse,
                running, use, aveq;

            ptr = sigar_skip_token(ptr); /* name */

            num = sscanf(ptr,
                         "%lu %lu %lu %lu "
                         "%lu %lu %lu %lu "
                         "%lu %lu %lu",
                         &rio,     /* 1  # reads issued */
                         &rmerge,  /* 2  # reads merged */
                         &rsect,   /* 3  # sectors read */
                         &ruse,    /* 4  # millis spent reading */
                         &wio,     /* 5  # writes completed */
                         &wmerge,  /* 6  # writes merged */
                         &wsect,   /* 7  # sectors written */
                         &wuse,    /* 8  # millis spent writing */
                         &running, /* 9  # I/Os currently in progress */
                         &use,     /* 10 # millis spent doing I/Os */
                         &aveq);   /* 11 # of millis spent doing I/Os (weighted) */

            if (num == 11) {
                disk->rtime = ruse;
                disk->wtime = wuse;
                disk->time = use;
                disk->qtime = aveq;
            }
            else if (num == 4) {
                wio = rsect;
                rsect = rmerge;
                wsect = ruse;
                disk->time = disk->qtime = SIGAR_FIELD_NOTIMPL;
            }
            else {
                status = ENOENT;
            }

            disk->reads = rio;
            disk->writes = wio;
            disk->read_bytes  = rsect;
            disk->write_bytes = wsect;

            /* convert sectors to bytes (512 is fixed size in 2.6 kernels) */
            disk->read_bytes  *= 512;
            disk->write_bytes *= 512;

            if (minor == ST_MINOR(sb)) {
                status = SIGAR_OK;
                break;
            }
            else if (minor == 0) {
                memcpy(device_usage, disk, sizeof(*device_usage));
            }
        }
    }

    fclose(fp);

    return status;
}

static int get_iostat_procp(sigar_t *sigar,
                            const char *dirname,
                            sigar_disk_usage_t *disk,
                            sigar_iodev_t **iodev)
{
    FILE *fp;
    char buffer[1025];
    char *ptr;
    struct stat sb;

    if (!(*iodev = sigar_iodev_get(sigar, dirname))) {
        return ENXIO;
    }

    if (stat((*iodev)->name, &sb) < 0) {
        return errno;
    }

    if (SIGAR_LOG_IS_DEBUG(sigar)) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         PROC_PARTITIONS " %s -> %s [%d,%d]",
                         dirname, (*iodev)->name,
                         ST_MAJOR(sb), ST_MINOR(sb));
    }

    if (!(fp = fopen(PROC_PARTITIONS, "r"))) {
        return errno;
    }

    (void)fgets(buffer, sizeof(buffer), fp); /* skip header */
    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        unsigned long major, minor;

        major = sigar_strtoul(ptr);
        minor = sigar_strtoul(ptr);

        if ((major == ST_MAJOR(sb)) && (minor == ST_MINOR(sb))) {
            ptr = sigar_skip_token(ptr); /* blocks */
            ptr = sigar_skip_token(ptr); /* name */
            disk->reads = sigar_strtoull(ptr); /* rio */
            ptr = sigar_skip_token(ptr);  /* rmerge */ 
            disk->read_bytes  = sigar_strtoull(ptr); /* rsect */
            disk->rtime = sigar_strtoull(ptr); /* ruse */
            disk->writes = sigar_strtoull(ptr); /* wio */
            ptr = sigar_skip_token(ptr);  /* wmerge */ 
            disk->write_bytes = sigar_strtoull(ptr); /* wsect */
            disk->wtime = sigar_strtoull(ptr); /* wuse */
            ptr = sigar_skip_token(ptr); /* running */
            disk->time = sigar_strtoull(ptr); /* use */
            disk->qtime  = sigar_strtoull(ptr); /* aveq */

            /* convert sectors to bytes (512 is fixed size in 2.6 kernels) */
            disk->read_bytes  *= 512;
            disk->write_bytes *= 512;

            fclose(fp);
            return SIGAR_OK;
        }
    }

    fclose(fp);

    return ENOENT;
}

int sigar_disk_usage_get(sigar_t *sigar, const char *name,
                         sigar_disk_usage_t *disk)
{
    int status;
    sigar_iodev_t *iodev = NULL;
    sigar_disk_usage_t device_usage;
    SIGAR_DISK_STATS_INIT(disk);

    /*
     * 2.2 has metrics /proc/stat, but wtf is the device mapping?
     * 2.4 has /proc/partitions w/ the metrics.
     * 2.6 has /proc/partitions w/o the metrics.
     *     instead the metrics are within the /proc-like /sys filesystem.
     *     also has /proc/diskstats
     */
    switch (sigar->iostat) {
      case IOSTAT_SYS:
        status = get_iostat_sys(sigar, name, disk, &iodev);
        break;
      case IOSTAT_DISKSTATS:
        status = get_iostat_proc_dstat(sigar, name, disk, &iodev, &device_usage);
        break;
      case IOSTAT_PARTITIONS:
        status = get_iostat_procp(sigar, name, disk, &iodev);
        break;
      /*
       * case IOSTAT_SOME_OTHER_WIERD_THING:
       * break;
       */
      case IOSTAT_NONE:
      default:
        status = ENOENT;
        break;
    }

    if ((status == SIGAR_OK) && iodev) {
        sigar_uptime_t uptime;
        sigar_uint64_t interval, ios;
        double tput, util;
        sigar_disk_usage_t *partition_usage=NULL;

        sigar_uptime_get(sigar, &uptime);

        if (iodev->is_partition &&
            (sigar->iostat == IOSTAT_DISKSTATS))
        {
            /* 2.6 kernels do not have per-partition times */
            partition_usage = disk;
            disk = &device_usage;
        }

        disk->snaptime = uptime.uptime;

        if (iodev->disk.snaptime) {
            interval = disk->snaptime - iodev->disk.snaptime;
        }
        else {
            interval = disk->snaptime;
        }

        ios =
            (disk->reads - iodev->disk.reads) +
            (disk->writes - iodev->disk.writes);

        if (disk->time == SIGAR_FIELD_NOTIMPL) {
            disk->service_time = SIGAR_FIELD_NOTIMPL;
        }
        else {
            tput = ((double)ios) * HZ / interval;
            util = ((double)(disk->time - iodev->disk.time)) / interval * HZ;
            disk->service_time = tput ? util / tput : 0.0;
        }
        if (disk->qtime == SIGAR_FIELD_NOTIMPL) {
            disk->queue = SIGAR_FIELD_NOTIMPL;
        }
        else {
            util = ((double)(disk->qtime - iodev->disk.qtime)) / interval;
            disk->queue = util / 1000.0;
        }

        memcpy(&iodev->disk, disk, sizeof(iodev->disk));
        if (partition_usage) {
            partition_usage->service_time = disk->service_time;
            partition_usage->queue = disk->queue;
        }
    }

    return status;
}

int sigar_file_system_usage_get(sigar_t *sigar,
                                const char *dirname,
                                sigar_file_system_usage_t *fsusage)
{
    int status = sigar_statvfs(sigar, dirname, fsusage);

    if (status != SIGAR_OK) {
        return status;
    }

    fsusage->use_percent = sigar_file_system_usage_calc_used(sigar, fsusage);

    (void)sigar_disk_usage_get(sigar, dirname, &fsusage->disk);

    return SIGAR_OK;
}

static SIGAR_INLINE char *cpu_info_strval(char *ptr)
{
    if ((ptr = strchr(ptr, ':'))) {
        ptr++;
        while (isspace (*ptr)) ptr++;
        return ptr;
    }
    return NULL;
}

static SIGAR_INLINE void cpu_info_strcpy(char *ptr, char *buf, int len)
{
    int slen;
    ptr = cpu_info_strval(ptr);
    if (!ptr) {
        return;
    }
    slen = strlen(ptr);
    strncpy(buf, ptr, len);
    buf[len] = '\0';
    if (slen < len) {
        buf[slen-1] = '\0'; /* rid \n */
    }
}

static int get_cpu_info(sigar_t *sigar, sigar_cpu_info_t *info,
                        FILE *fp)
{
    char buffer[BUFSIZ], *ptr;

    int found = 0;

    /* UML vm wont have "cpu MHz" or "cache size" fields */
    info->mhz        = 0;
    info->cache_size = 0;

#ifdef __powerpc64__
    SIGAR_SSTRCPY(info->vendor, "IBM");
#endif

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        switch (*ptr) {
          case 'p': /* processor	: 0 */
            if (strnEQ(ptr, "processor", 9)) {
                found = 1;
            }
            break;
          case 'v':
            /* "vendor_id" or "vendor" */
            if (strnEQ(ptr, "vendor", 6)) {
                cpu_info_strcpy(ptr, info->vendor, sizeof(info->vendor));
                if (strEQ(info->vendor, "GenuineIntel")) {
                    SIGAR_SSTRCPY(info->vendor, "Intel");
                }
                else if (strEQ(info->vendor, "AuthenticAMD")) {
                    SIGAR_SSTRCPY(info->vendor, "AMD");
                }
            }
            break;
          case 'f':
            if (strnEQ(ptr, "family", 6)) {
                /* IA64 version of "model name" */
                cpu_info_strcpy(ptr, info->model, sizeof(info->model));
                sigar_cpu_model_adjust(sigar, info);
            }
            break;
          case 'm':
            if (strnEQ(ptr, "model name", 10)) {
                cpu_info_strcpy(ptr, info->model, sizeof(info->model));
                sigar_cpu_model_adjust(sigar, info);
            }
            break;
          case 'c':
            if (strnEQ(ptr, "cpu MHz", 7)) {
                ptr = cpu_info_strval(ptr);
                info->mhz = atoi(ptr);
            }
            else if (strnEQ(ptr, "cache size", 10)) {
                ptr = cpu_info_strval(ptr);
                info->cache_size = sigar_strtoul(ptr);
            }
#ifdef __powerpc64__
            /* each /proc/cpuinfo entry looks like so:
             *   processor       : 0
             *   cpu             : POWER5 (gr)
             *   clock           : 1656.392000MHz
             *   revision        : 2.2
             */
            else if (strnEQ(ptr, "clock", 5)) {
                ptr = cpu_info_strval(ptr);
                info->mhz = atoi(ptr);
            }
            else if (strnEQ(ptr, "cpu", 3)) {
                cpu_info_strcpy(ptr, info->model, sizeof(info->model));

                if ((ptr = strchr(info->model, ' '))) {
                    /* "POWER5 (gr)" -> "POWER5" */
                    *ptr = '\0';
                }
            }
#endif
            break;
           /* lone \n means end of info for this processor */
          case '\n':
            return found;
        }
    }

    return found;
}

/* /proc/cpuinfo MHz will change w/ AMD + PowerNow */
static void get_cpuinfo_max_freq(sigar_cpu_info_t *cpu_info, int num)
{
    int status;
    char max_freq[PATH_MAX];
    snprintf(max_freq, sizeof(max_freq),
             "/sys/devices/system/cpu/cpu%d"
             "/cpufreq/cpuinfo_max_freq", num);

    status =
        sigar_file2str(max_freq, max_freq, sizeof(max_freq)-1);

    if (status == SIGAR_OK) {
        cpu_info->mhz = atoi(max_freq) / 1000;
    }
}

int sigar_cpu_info_list_get(sigar_t *sigar,
                            sigar_cpu_info_list_t *cpu_infos)
{
    FILE *fp;
    int core_rollup = sigar_cpu_core_rollup(sigar), i=0;

    if (!(fp = fopen(PROC_FS_ROOT "cpuinfo", "r"))) {
        return errno;
    }

    (void)sigar_cpu_total_count(sigar);
    sigar_cpu_info_list_create(cpu_infos);

    while (get_cpu_info(sigar, &cpu_infos->data[cpu_infos->number], fp)) {
        sigar_cpu_info_t *info;

        if (core_rollup && (i++ % sigar->lcpu)) {
            continue; /* fold logical processors */
        }

        info = &cpu_infos->data[cpu_infos->number];
        get_cpuinfo_max_freq(info, cpu_infos->number);

        info->total_cores = sigar->ncpu;
        info->cores_per_socket = sigar->lcpu;
        info->total_sockets = sigar_cpu_socket_count(sigar);

        ++cpu_infos->number;
        SIGAR_CPU_INFO_LIST_GROW(cpu_infos);
    }

    fclose(fp);

    return SIGAR_OK;
}

static SIGAR_INLINE unsigned int hex2int(const char *x, int len)
{
    int i;
    unsigned int j;

    for (i=0, j=0; i<len; i++) {
        register int ch = x[i];
        j <<= 4;
        if (isdigit(ch)) {
            j |= ch - '0';
        }
        else if (isupper(ch)) {
            j |= ch - ('A' - 10);
        }
        else {
            j |= ch - ('a' - 10);
        }
    }

    return j;
}

#define HEX_ENT_LEN 8

#ifdef SIGAR_64BIT
#define ROUTE_FMT "%16s %128s %128s %X %ld %ld %ld %128s %ld %ld %ld\n"
#else
#define ROUTE_FMT "%16s %128s %128s %X %lld %lld %lld %128s %lld %lld %lld\n"
#endif
#define RTF_UP 0x0001

int sigar_net_route_list_get(sigar_t *sigar,
                             sigar_net_route_list_t *routelist)
{
    FILE *fp;
    char buffer[1024];
    char net_addr[128], gate_addr[128], mask_addr[128];
    int flags;
    sigar_net_route_t *route;

    routelist->size = routelist->number = 0;

    if (!(fp = fopen(PROC_FS_ROOT "net/route", "r"))) {
        return errno;
    }

    sigar_net_route_list_create(routelist);

    (void)fgets(buffer, sizeof(buffer), fp); /* skip header */
    while (fgets(buffer, sizeof(buffer), fp)) {
        int num;

        SIGAR_NET_ROUTE_LIST_GROW(routelist);
        route = &routelist->data[routelist->number++];

        /* XXX rid sscanf */
        num = sscanf(buffer, ROUTE_FMT,
                     route->ifname, net_addr, gate_addr,
                     &flags, &route->refcnt, &route->use,
                     &route->metric, mask_addr,
                     &route->mtu, &route->window, &route->irtt);

        if ((num < 10) || !(flags & RTF_UP)) {
            --routelist->number;
            continue;
        }

        route->flags = flags;
        
        sigar_net_address_set(route->destination, hex2int(net_addr, HEX_ENT_LEN));
        sigar_net_address_set(route->gateway, hex2int(gate_addr, HEX_ENT_LEN));
        sigar_net_address_set(route->mask, hex2int(mask_addr, HEX_ENT_LEN));
    }

    fclose(fp);

    return SIGAR_OK;
}

int sigar_net_interface_stat_get(sigar_t *sigar, const char *name,
                                 sigar_net_interface_stat_t *ifstat)
{
    int found = 0;
    char buffer[BUFSIZ];
    FILE *fp = fopen(PROC_FS_ROOT "net/dev", "r");
    
    if (!fp) {
        return errno;
    }

    /* skip header */
    fgets(buffer, sizeof(buffer), fp);
    fgets(buffer, sizeof(buffer), fp);

    while (fgets(buffer, sizeof(buffer), fp)) {
        char *ptr, *dev;

        dev = buffer;
        while (isspace(*dev)) {
            dev++;
        }

        if (!(ptr = strchr(dev, ':'))) {
            continue;
        }

        *ptr++ = 0;

        if (!strEQ(dev, name)) {
            continue;
        }

        found = 1;
        ifstat->rx_bytes    = sigar_strtoull(ptr);
        ifstat->rx_packets  = sigar_strtoull(ptr);
        ifstat->rx_errors   = sigar_strtoull(ptr);
        ifstat->rx_dropped  = sigar_strtoull(ptr);
        ifstat->rx_overruns = sigar_strtoull(ptr);
        ifstat->rx_frame    = sigar_strtoull(ptr);

        /* skip: compressed multicast */
        ptr = sigar_skip_multiple_token(ptr, 2);

        ifstat->tx_bytes      = sigar_strtoull(ptr);
        ifstat->tx_packets    = sigar_strtoull(ptr);
        ifstat->tx_errors     = sigar_strtoull(ptr);
        ifstat->tx_dropped    = sigar_strtoull(ptr);
        ifstat->tx_overruns   = sigar_strtoull(ptr);
        ifstat->tx_collisions = sigar_strtoull(ptr);
        ifstat->tx_carrier    = sigar_strtoull(ptr);

        ifstat->speed         = SIGAR_FIELD_NOTIMPL;

        break;
    }

    fclose(fp);

    return found ? SIGAR_OK : ENXIO;
}

static SIGAR_INLINE void convert_hex_address(sigar_net_address_t *address,
                                             char *ptr, int len)
{
    if (len > HEX_ENT_LEN) {
        int i;
        for (i=0; i<=3; i++, ptr+=HEX_ENT_LEN) {
            address->addr.in6[i] = hex2int(ptr, HEX_ENT_LEN);
        }

        address->family = SIGAR_AF_INET6;
    }
    else {
        address->addr.in =
            (len == HEX_ENT_LEN) ? hex2int(ptr, HEX_ENT_LEN) : 0;

        address->family = SIGAR_AF_INET;
    }
}

typedef struct {
    sigar_net_connection_list_t *connlist;
    sigar_net_connection_t *conn;
    unsigned long port;
} net_conn_getter_t;

static int proc_net_walker(sigar_net_connection_walker_t *walker,
                           sigar_net_connection_t *conn)
{
    net_conn_getter_t *getter =
        (net_conn_getter_t *)walker->data;

    if (getter->connlist) {
        SIGAR_NET_CONNLIST_GROW(getter->connlist);
        memcpy(&getter->connlist->data[getter->connlist->number++],
               conn, sizeof(*conn));
    }
    else {
        if ((getter->port == conn->local_port) &&
            (conn->remote_port == 0))
        {
            memcpy(getter->conn, conn, sizeof(*conn));
            return !SIGAR_OK; /* break loop */
        }
    }

    return SIGAR_OK; /* continue loop */
}

#define SKIP_WHILE(p, c) while (*p == c) p++
#define SKIP_PAST(p, c) \
   while(*p && (*p != c)) p++; \
   SKIP_WHILE(p, c)

static int proc_net_read(sigar_net_connection_walker_t *walker,
                         const char *fname,
                         int type)
{
    FILE *fp = NULL;
    char buffer[8192];
    sigar_t *sigar = walker->sigar;
    char *ptr = sigar->proc_net;
    int flags = walker->flags;

    if (ptr) {
        snprintf(buffer, sizeof(buffer),
                 "%s/%s", ptr,
                 fname + sizeof(PROC_FS_ROOT)-1);

        if ((fp = fopen(buffer, "r"))) {
            if (SIGAR_LOG_IS_DEBUG(sigar)) {
                sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                 "[proc_net] using %s",
                                 buffer);
            }
        }
        else if (SIGAR_LOG_IS_DEBUG(sigar)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[proc_net] cannot open %s",
                             buffer);
        }
    }

    if (!(fp || (fp = fopen(fname, "r")))) {
        return errno;
    }

    fgets(buffer, sizeof(buffer), fp); /* skip header */

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        sigar_net_connection_t conn;
        char *laddr, *raddr;
        int laddr_len=0, raddr_len=0;
        int more;

        /* skip leading space */
        SKIP_WHILE(ptr, ' ');

        /* skip "%d: " */
        SKIP_PAST(ptr, ' ');

        laddr = ptr;
        while (*ptr && (*ptr != ':')) {
            laddr_len++;
            ptr++;
        }
        SKIP_WHILE(ptr, ':');

        conn.local_port = (strtoul(ptr, &ptr, 16) & 0xffff);

        SKIP_WHILE(ptr, ' ');

        raddr = ptr;
        while (*ptr && (*ptr != ':')) {
            raddr_len++;
            ptr++;
        }
        SKIP_WHILE(ptr, ':');

        conn.remote_port = (strtoul(ptr, &ptr, 16) & 0xffff);

        SKIP_WHILE(ptr, ' ');

        if (!((conn.remote_port && (flags & SIGAR_NETCONN_CLIENT)) ||
              (!conn.remote_port && (flags & SIGAR_NETCONN_SERVER))))
        {
            continue;
        }

        conn.type = type;

        convert_hex_address(&conn.local_address,
                            laddr, laddr_len);

        convert_hex_address(&conn.remote_address,
                            raddr, raddr_len);

        /* SIGAR_TCP_* currently matches TCP_* in linux/tcp.h */
        conn.state = hex2int(ptr, 2);
        ptr += 2;
        SKIP_WHILE(ptr, ' ');

        conn.send_queue = hex2int(ptr, HEX_ENT_LEN);
        ptr += HEX_ENT_LEN+1; /* tx + ':' */;

        conn.receive_queue = hex2int(ptr, HEX_ENT_LEN);
        ptr += HEX_ENT_LEN;
        SKIP_WHILE(ptr, ' ');

        SKIP_PAST(ptr, ' '); /* tr:tm->whem */
        SKIP_PAST(ptr, ' '); /* retrnsmt */

        conn.uid = sigar_strtoul(ptr);

        SKIP_WHILE(ptr, ' ');
        SKIP_PAST(ptr, ' '); /* timeout */

        conn.inode = sigar_strtoul(ptr);

        more = walker->add_connection(walker, &conn);
        if (more != SIGAR_OK) {
            fclose(fp);
            return SIGAR_OK;
        }
    }

    fclose(fp);

    return SIGAR_OK;
}

int sigar_net_connection_walk(sigar_net_connection_walker_t *walker)
{
    int flags = walker->flags;
    int status;

    if (flags & SIGAR_NETCONN_TCP) {
        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/tcp",
                               SIGAR_NETCONN_TCP);

        if (status != SIGAR_OK) {
            return status;
        }

        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/tcp6",
                               SIGAR_NETCONN_TCP);

        if (!((status == SIGAR_OK) || (status == ENOENT))) {
            return status;
        }
    }

    if (flags & SIGAR_NETCONN_UDP) {
        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/udp",
                               SIGAR_NETCONN_UDP);

        if (status != SIGAR_OK) {
            return status;
        }

        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/udp6",
                               SIGAR_NETCONN_UDP);

        if (!((status == SIGAR_OK) || (status == ENOENT))) {
            return status;
        }
    }

    if (flags & SIGAR_NETCONN_RAW) {
        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/raw",
                               SIGAR_NETCONN_RAW);
        
        if (status != SIGAR_OK) {
            return status;
        }

        status = proc_net_read(walker,
                               PROC_FS_ROOT "net/raw6",
                               SIGAR_NETCONN_RAW);

        if (!((status == SIGAR_OK) || (status == ENOENT))) {
            return status;
        }
    }

    /* XXX /proc/net/unix */

    return SIGAR_OK;
}

int sigar_net_connection_list_get(sigar_t *sigar,
                                  sigar_net_connection_list_t *connlist,
                                  int flags)
{
    int status;
    sigar_net_connection_walker_t walker;
    net_conn_getter_t getter;

    sigar_net_connection_list_create(connlist);

    getter.conn = NULL;
    getter.connlist = connlist;

    walker.sigar = sigar;
    walker.flags = flags;
    walker.data = &getter;
    walker.add_connection = proc_net_walker;

    status = sigar_net_connection_walk(&walker);

    if (status != SIGAR_OK) {
        sigar_net_connection_list_destroy(sigar, connlist);
    }

    return status;
}

static int sigar_net_connection_get(sigar_t *sigar,
                                    sigar_net_connection_t *netconn,
                                    unsigned long port,
                                    int flags)
{
    int status;
    sigar_net_connection_walker_t walker;
    net_conn_getter_t getter;

    getter.conn = netconn;
    getter.connlist = NULL;
    getter.port = port;

    walker.sigar = sigar;
    walker.flags = flags;
    walker.data = &getter;
    walker.add_connection = proc_net_walker;

    status = sigar_net_connection_walk(&walker);

    return status;
}

#define SNMP_TCP_PREFIX "Tcp: "

SIGAR_DECLARE(int)
sigar_tcp_get(sigar_t *sigar,
              sigar_tcp_t *tcp)
{
    FILE *fp;
    char buffer[1024], *ptr=buffer;
    int status = SIGAR_ENOENT;

    if (!(fp = fopen(PROC_FS_ROOT "net/snmp", "r"))) {
        return errno;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        if (strnEQ(buffer, SNMP_TCP_PREFIX, sizeof(SNMP_TCP_PREFIX)-1)) {
            if (fgets(buffer, sizeof(buffer), fp)) {
                status = SIGAR_OK;
                break;
            }
        }
    }

    fclose(fp);

    if (status == SIGAR_OK) {
        /* assuming field order, same in 2.2, 2.4 and 2.6 kernels */ 
        /* Tcp: RtoAlgorithm RtoMin RtoMax MaxConn */
        ptr = sigar_skip_multiple_token(ptr, 5);
        tcp->active_opens = sigar_strtoull(ptr);
        tcp->passive_opens = sigar_strtoull(ptr);
        tcp->attempt_fails = sigar_strtoull(ptr);
        tcp->estab_resets = sigar_strtoull(ptr);
        tcp->curr_estab = sigar_strtoull(ptr);
        tcp->in_segs = sigar_strtoull(ptr);
        tcp->out_segs = sigar_strtoull(ptr);
        tcp->retrans_segs = sigar_strtoull(ptr);
        tcp->in_errs = sigar_strtoull(ptr);
        tcp->out_rsts = sigar_strtoull(ptr);
    }

    return status;
}

static int sigar_proc_nfs_gets(char *file, char *tok,
                               char *buffer, size_t size)
{
    int status = ENOENT;
    int len = strlen(tok);
    FILE *fp = fopen(file, "r");
    
    if (!fp) {
        return SIGAR_ENOTIMPL;
    }

    while (fgets(buffer, size, fp)) {
        if (strnEQ(buffer, tok, len)) {
            status = SIGAR_OK;
            break;
        }
    }

    fclose(fp);

    return status;
}

static int sigar_nfs_v2_get(char *file, sigar_nfs_v2_t *nfs)
{
    char buffer[BUFSIZ], *ptr=buffer;
    int status =
        sigar_proc_nfs_gets(file,
                            "proc2", buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    ptr = sigar_skip_multiple_token(ptr, 2);

    nfs->null = sigar_strtoull(ptr);
    nfs->getattr = sigar_strtoull(ptr);
    nfs->setattr = sigar_strtoull(ptr);
    nfs->root = sigar_strtoull(ptr);
    nfs->lookup = sigar_strtoull(ptr);
    nfs->readlink = sigar_strtoull(ptr);
    nfs->read = sigar_strtoull(ptr);
    nfs->writecache = sigar_strtoull(ptr);
    nfs->write = sigar_strtoull(ptr);
    nfs->create = sigar_strtoull(ptr);
    nfs->remove = sigar_strtoull(ptr);
    nfs->rename = sigar_strtoull(ptr);
    nfs->link = sigar_strtoull(ptr);
    nfs->symlink = sigar_strtoull(ptr);
    nfs->mkdir = sigar_strtoull(ptr);
    nfs->rmdir = sigar_strtoull(ptr);
    nfs->readdir = sigar_strtoull(ptr);
    nfs->fsstat = sigar_strtoull(ptr);

    return SIGAR_OK;
}

int sigar_nfs_client_v2_get(sigar_t *sigar,
                            sigar_nfs_client_v2_t *nfs)
{
    return sigar_nfs_v2_get(PROC_FS_ROOT "net/rpc/nfs",
                            (sigar_nfs_v2_t *)nfs);
}

int sigar_nfs_server_v2_get(sigar_t *sigar,
                            sigar_nfs_server_v2_t *nfs)
{
    return sigar_nfs_v2_get(PROC_FS_ROOT "net/rpc/nfsd",
                            (sigar_nfs_v2_t *)nfs);
}

static int sigar_nfs_v3_get(char *file, sigar_nfs_v3_t *nfs)
{
    char buffer[BUFSIZ], *ptr=buffer;
    int status =
        sigar_proc_nfs_gets(file,
                            "proc3", buffer, sizeof(buffer));

    if (status != SIGAR_OK) {
        return status;
    }

    ptr = sigar_skip_multiple_token(ptr, 2);

    nfs->null = sigar_strtoull(ptr);
    nfs->getattr = sigar_strtoull(ptr);
    nfs->setattr = sigar_strtoull(ptr);
    nfs->lookup = sigar_strtoull(ptr);
    nfs->access = sigar_strtoull(ptr);
    nfs->readlink = sigar_strtoull(ptr);
    nfs->read = sigar_strtoull(ptr);
    nfs->write = sigar_strtoull(ptr);
    nfs->create = sigar_strtoull(ptr);
    nfs->mkdir = sigar_strtoull(ptr);
    nfs->symlink = sigar_strtoull(ptr);
    nfs->mknod = sigar_strtoull(ptr);
    nfs->remove = sigar_strtoull(ptr);
    nfs->rmdir = sigar_strtoull(ptr);
    nfs->rename = sigar_strtoull(ptr);
    nfs->link = sigar_strtoull(ptr);
    nfs->readdir = sigar_strtoull(ptr);
    nfs->readdirplus = sigar_strtoull(ptr);
    nfs->fsstat = sigar_strtoull(ptr);
    nfs->fsinfo = sigar_strtoull(ptr);
    nfs->pathconf = sigar_strtoull(ptr);
    nfs->commit = sigar_strtoull(ptr);

    return SIGAR_OK;
}

int sigar_nfs_client_v3_get(sigar_t *sigar,
                            sigar_nfs_client_v3_t *nfs)
{
    return sigar_nfs_v3_get(PROC_FS_ROOT "net/rpc/nfs",
                            (sigar_nfs_v3_t *)nfs);
}

int sigar_nfs_server_v3_get(sigar_t *sigar,
                            sigar_nfs_server_v3_t *nfs)
{
    return sigar_nfs_v3_get(PROC_FS_ROOT "net/rpc/nfsd",
                            (sigar_nfs_v3_t *)nfs);
}

int sigar_proc_port_get(sigar_t *sigar, int protocol,
                        unsigned long port, sigar_pid_t *pid)
{
    int status;
    sigar_net_connection_t netconn;
    DIR *dirp;
    struct dirent *ent, dbuf;

    SIGAR_ZERO(&netconn);
    *pid = 0;

    status = sigar_net_connection_get(sigar, &netconn, port,
                                      SIGAR_NETCONN_SERVER|protocol);

    if (status != SIGAR_OK) {
        return status;
    }

    if (netconn.local_port != port) {
        return SIGAR_OK; /* XXX or ENOENT? */
    }

    if (!(dirp = opendir(PROCP_FS_ROOT))) {
        return errno;
    }

    while (readdir_r(dirp, &dbuf, &ent) == 0) {
        DIR *fd_dirp;
        struct dirent *fd_ent, fd_dbuf;
        struct stat sb;
        char fd_name[BUFSIZ], pid_name[BUFSIZ];
        int len, slen;

        if (ent == NULL) {
            break;
        }

        if (!sigar_isdigit(*ent->d_name)) {
            continue;
        }

        /* sprintf(pid_name, "/proc/%s", ent->d_name) */
        memcpy(&pid_name[0], PROCP_FS_ROOT, SSTRLEN(PROCP_FS_ROOT));
        len = SSTRLEN(PROCP_FS_ROOT);
        pid_name[len++] = '/';

        slen = strlen(ent->d_name);
        memcpy(&pid_name[len], ent->d_name, slen);
        len += slen;
        pid_name[len] = '\0';
        
        if (stat(pid_name, &sb) < 0) {
            continue;
        }
        if (sb.st_uid != netconn.uid) {
            continue;
        }

        /* sprintf(fd_name, "%s/fd", pid_name) */
        memcpy(&fd_name[0], pid_name, len);
        memcpy(&fd_name[len], "/fd", 3);
        fd_name[len+=3] = '\0';

        if (!(fd_dirp = opendir(fd_name))) {
            continue;
        }

        while (readdir_r(fd_dirp, &fd_dbuf, &fd_ent) == 0) {
            char fd_ent_name[BUFSIZ];

            if (fd_ent == NULL) {
                break;
            }

            if (!sigar_isdigit(*fd_ent->d_name)) {
                continue;
            }

            /* sprintf(fd_ent_name, "%s/%s", fd_name, fd_ent->d_name) */
            slen = strlen(fd_ent->d_name);
            memcpy(&fd_ent_name[0], fd_name, len);
            fd_ent_name[len] = '/';
            memcpy(&fd_ent_name[len+1], fd_ent->d_name, slen);
            fd_ent_name[len+1+slen] = '\0';

            if (stat(fd_ent_name, &sb) < 0) {
                continue;
            }

            if (sb.st_ino == netconn.inode) {
                closedir(fd_dirp);
                closedir(dirp);
                *pid = strtoul(ent->d_name, NULL, 10);
                return SIGAR_OK;
            }
            
        }

        closedir(fd_dirp);
    }

    closedir(dirp);

    return SIGAR_OK;
}

static void generic_vendor_parse(char *line, sigar_sys_info_t *info)
{
    char *ptr;
    int len = 0;
        
    while (*line) {
        SIGAR_SKIP_SPACE(line);
        if (!isdigit(*line)) {
            ++line;
            continue;
        }

        ptr = line;
        while ((isdigit(*ptr) || (*ptr == '.'))) {
            ++ptr;
            ++len;
        }

        if (len) {
            /* sanity check */
            if (len > sizeof(info->vendor_version)) {
                continue;
            }
            memcpy(info->vendor_version, line, len);/*XXX*/
            info->vendor_version[len] = '\0';
            return;
        }
    }
}

static void redhat_vendor_parse(char *line, sigar_sys_info_t *info)
{
    char *start, *end;

    generic_vendor_parse(line, info); /* super.parse */

    if ((start = strchr(line, '('))) {
        ++start;
        if ((end = strchr(start, ')'))) {
            int len = end-start;
            memcpy(info->vendor_code_name, start, len);/*XXX*/
            info->vendor_code_name[len] = '\0';
        }
    }

#define RHEL_PREFIX "Red Hat Enterprise Linux "
#define CENTOS_VENDOR "CentOS"
#define SL_VENDOR "Scientific Linux"

    if (strnEQ(line, RHEL_PREFIX, sizeof(RHEL_PREFIX)-1)) {
        snprintf(info->vendor_version,
                 sizeof(info->vendor_version),
                 "Enterprise Linux %c",
                 info->vendor_version[0]);
    }
    else if (strnEQ(line, CENTOS_VENDOR, sizeof(CENTOS_VENDOR)-1)) {
        SIGAR_SSTRCPY(info->vendor, CENTOS_VENDOR);
    }
    else if (strnEQ(line, SL_VENDOR, sizeof(SL_VENDOR)-1)) {
        SIGAR_SSTRCPY(info->vendor, SL_VENDOR);
    }
}

#define is_quote(c) ((c == '\'') || (c == '"')) 

static void kv_parse(char *data, sigar_sys_info_t *info,
                     void (*func)(sigar_sys_info_t *, char *, char *))
{
    char *ptr = data;
    int len = strlen(data);
    char *end = data+len;

    while (ptr < end) {
        char *val = strchr(ptr, '=');
        int klen, vlen;
        char key[256], *ix;

        if (!val) {
            continue;
        }
        klen = val - ptr;
        SIGAR_SSTRCPY(key, ptr);
        key[klen] = '\0';
        ++val;

        if ((ix = strchr(val, '\n'))) {
            *ix = '\0';
        }
        vlen = strlen(val);
        if (is_quote(*val)) {
            if (is_quote(val[vlen-1])) {
                val[vlen-1] =  '\0';
            }
            ++val;
        }

        func(info, key, val);

        ptr += (klen + 1 + vlen + 1);
    }
}

static void lsb_parse(sigar_sys_info_t *info,
                      char *key, char *val)
{
    if (strEQ(key, "DISTRIB_ID")) {
        SIGAR_SSTRCPY(info->vendor, val);
    }
    else if (strEQ(key, "DISTRIB_RELEASE")) {
        SIGAR_SSTRCPY(info->vendor_version, val);
    }
    else if (strEQ(key, "DISTRIB_CODENAME")) {
        SIGAR_SSTRCPY(info->vendor_code_name, val);
    }
}

static void lsb_vendor_parse(char *data, sigar_sys_info_t *info)
{
    kv_parse(data, info, lsb_parse);
}

static void xen_parse(sigar_sys_info_t *info,
                      char *key, char *val)
{
    if (strEQ(key, "PRODUCT_VERSION")) {
        SIGAR_SSTRCPY(info->vendor_version, val);
    }
    else if (strEQ(key, "KERNEL_VERSION")) {
        SIGAR_SSTRCPY(info->version, val);
    }
}

static void xen_vendor_parse(char *data, sigar_sys_info_t *info)
{
    kv_parse(data, info, xen_parse);

    snprintf(info->description,
             sizeof(info->description),
             "XenServer %s",
             info->vendor_version);
}

typedef struct {
    const char *name;
    const char *file;
    void (*parse)(char *, sigar_sys_info_t *);
} linux_vendor_info_t;

static linux_vendor_info_t linux_vendors[] = {
    { "Fedora",    "/etc/fedora-release", NULL },
    { "SuSE",      "/etc/SuSE-release", NULL },
    { "Gentoo",    "/etc/gentoo-release", NULL },
    { "Slackware", "/etc/slackware-version", NULL },
    { "Mandrake",  "/etc/mandrake-release", NULL },
    { "VMware",    "/proc/vmware/version", NULL },
    { "XenSource", "/etc/xensource-inventory", xen_vendor_parse },
    { "Red Hat",   "/etc/redhat-release", redhat_vendor_parse },
    { "lsb",       "/etc/lsb-release", lsb_vendor_parse },
    { "Debian",    "/etc/debian_version", NULL },
    { NULL }
};

static int get_linux_vendor_info(sigar_sys_info_t *info)
{
    int i, status = ENOENT;
    /* env vars for testing */
    const char *release_file = getenv("SIGAR_OS_RELEASE_FILE");
    const char *vendor_name  = getenv("SIGAR_OS_VENDOR_NAME");
    char buffer[8192], *data;
    linux_vendor_info_t *vendor = NULL;

    for (i=0; linux_vendors[i].name; i++) {
        struct stat sb;
        vendor = &linux_vendors[i];

        if (release_file && vendor_name) {
            if (!strEQ(vendor->name, vendor_name)) {
                continue;
            }
        }
        else {
            if (stat(vendor->file, &sb) < 0) {
                continue;
            }
            release_file = vendor->file;
        }

        status =
            sigar_file2str(release_file, buffer, sizeof(buffer)-1);

        break;
    }

    if (status != SIGAR_OK) {
        return status;
    }

    data = buffer;

    SIGAR_SSTRCPY(info->vendor, vendor->name);

    if (vendor->parse) {
        vendor->parse(data, info);
    }
    else {
        generic_vendor_parse(data, info);
    }

    if (info->description[0] == '\0') {
        snprintf(info->description,
                 sizeof(info->description),
                 "%s %s",
                 info->vendor, info->vendor_version);
    }

    return SIGAR_OK;
}

int sigar_os_sys_info_get(sigar_t *sigar,
                          sigar_sys_info_t *sysinfo)
{

    get_linux_vendor_info(sysinfo);

    return SIGAR_OK;
}
