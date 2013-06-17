/*
 * Copyright (c) 2004-2008 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2009-2010 VMware, Inc.
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

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include "sigar_os.h"

#include <inet/ip.h>
#include <inet/tcp.h>
#include <net/route.h>
#include <sys/lwp.h>
#include <sys/proc.h>
#include <sys/swap.h>
#include <sys/stat.h>
#include <sys/systeminfo.h>
#include <sys/utsname.h>
#include <dlfcn.h>
#include <dirent.h>

#define PROC_ERRNO ((errno == ENOENT) ? ESRCH : errno)
#define SIGAR_USR_UCB_PS "/usr/ucb/ps"


/* like kstat_lookup but start w/ ksp->ks_next instead of kc->kc_chain */
static kstat_t *
kstat_next(kstat_t *ksp, char *ks_module, int ks_instance, char *ks_name)
{
    if (ksp) {
        ksp = ksp->ks_next;
    }
    for (; ksp; ksp = ksp->ks_next) {
        if ((ks_module == NULL ||
             strcmp(ksp->ks_module, ks_module) == 0) &&
            (ks_instance == -1 || ksp->ks_instance == ks_instance) &&
            (ks_name == NULL || strcmp(ksp->ks_name, ks_name) == 0))
            return ksp;
    }

    errno = ENOENT;
    return NULL;
}

int sigar_os_open(sigar_t **sig)
{
    kstat_ctl_t *kc;
    kstat_t *ksp;
    sigar_t *sigar;
    int i, status;
    struct utsname name;
    char *ptr;

    sigar = malloc(sizeof(*sigar));
    *sig = sigar;

    sigar->log_level = -1; /* log nothing by default */
    sigar->log_impl = NULL;
    sigar->log_data = NULL;

    uname(&name);
    if ((ptr = strchr(name.release, '.'))) {
        ptr++;
        sigar->solaris_version = atoi(ptr);
    }
    else {
        sigar->solaris_version = 6;
    }

    if ((ptr = getenv("SIGAR_USE_UCB_PS"))) {
        sigar->use_ucb_ps = strEQ(ptr, "true");
    }
    else {
        struct stat sb;
        if (stat(SIGAR_USR_UCB_PS, &sb) < 0) {
            sigar->use_ucb_ps = 0;
        }
        else {
            sigar->use_ucb_ps = 1;
        }
    }

    sigar->pagesize = 0;
    i = sysconf(_SC_PAGESIZE);
    while ((i >>= 1) > 0) {
        sigar->pagesize++;
    }

    sigar->ticks = sysconf(_SC_CLK_TCK);
    sigar->kc = kc = kstat_open();

    if (!kc) {
        return errno;
    }

    sigar->cpulist.size = 0;
    sigar->ncpu = 0;
    sigar->ks.cpu = NULL;
    sigar->ks.cpu_info = NULL;
    sigar->ks.cpuid = NULL;
    sigar->ks.lcpu = 0;

    sigar->koffsets.system[0] = -1;
    sigar->koffsets.mempages[0] = -1;
    sigar->koffsets.syspages[0] = -1;

    if ((status = sigar_get_kstats(sigar)) != SIGAR_OK) {
        fprintf(stderr, "status=%d\n", status);
    } 

    sigar->boot_time = 0;

    if ((ksp = sigar->ks.system) &&
        (kstat_read(kc, ksp, NULL) >= 0))
    {
        sigar_koffsets_init_system(sigar, ksp);

        sigar->boot_time = kSYSTEM(KSTAT_SYSTEM_BOOT_TIME);
    }

    sigar->last_pid = -1;
    sigar->pinfo = NULL;

    sigar->plib = NULL;
    sigar->pgrab = NULL;
    sigar->pfree = NULL;
    sigar->pobjname = NULL;

    sigar->pargs = NULL;

    SIGAR_ZERO(&sigar->mib2);
    sigar->mib2.sd = -1;

    return SIGAR_OK;
}

int sigar_os_close(sigar_t *sigar)
{
    kstat_close(sigar->kc);
    if (sigar->mib2.sd != -1) {
        close_mib2(&sigar->mib2);
    }

    if (sigar->ks.lcpu) {
        free(sigar->ks.cpu);
        free(sigar->ks.cpu_info);
        free(sigar->ks.cpuid);
    }
    if (sigar->pinfo) {
        free(sigar->pinfo);
    }
    if (sigar->cpulist.size != 0) {
        sigar_cpu_list_destroy(sigar, &sigar->cpulist);
    }
    if (sigar->plib) {
        dlclose(sigar->plib);
    }
    if (sigar->pargs) {
        sigar_cache_destroy(sigar->pargs);
    }
    free(sigar);
    return SIGAR_OK;
}

char *sigar_os_error_string(sigar_t *sigar, int err)
{
    switch (err) {
      case SIGAR_EMIB2:
        return sigar->mib2.errmsg;
      default:
        return NULL;
    }
}

int sigar_mem_get(sigar_t *sigar, sigar_mem_t *mem)
{
    kstat_ctl_t *kc = sigar->kc; 
    kstat_t *ksp;
    sigar_uint64_t kern = 0;

    SIGAR_ZERO(mem);

    /* XXX: is mem hot swappable or can we just do this during open ? */
    mem->total = sysconf(_SC_PHYS_PAGES);
    mem->total <<= sigar->pagesize;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    if ((ksp = sigar->ks.syspages) && kstat_read(kc, ksp, NULL) >= 0) {
        sigar_koffsets_init_syspages(sigar, ksp);

        mem->free = kSYSPAGES(KSTAT_SYSPAGES_FREE);
        mem->free <<= sigar->pagesize;

        mem->used = mem->total - mem->free;
    }

    if ((ksp = sigar->ks.mempages) && kstat_read(kc, ksp, NULL) >= 0) {
        sigar_koffsets_init_mempages(sigar, ksp);
    }

    /* XXX mdb ::memstat cachelist/freelist not available to kstat, see: */
    /* http://bugs.opensolaris.org/bugdatabase/view_bug.do?bug_id=6821980 */

    /* ZFS ARC cache. see: http://opensolaris.org/jive/thread.jspa?messageID=393695 */
    if ((ksp = kstat_lookup(sigar->kc, "zfs", 0, "arcstats")) &&
        (kstat_read(sigar->kc, ksp, NULL) != -1))
    {
        kstat_named_t *kn;

        if ((kn = (kstat_named_t *)kstat_data_lookup(ksp, "size"))) {
            kern = kn->value.i64;
        }
        if ((kn = (kstat_named_t *)kstat_data_lookup(ksp, "c_min"))) {
            /* c_min cannot be reclaimed they say */
            if (kern > kn->value.i64) {
                kern -= kn->value.i64;
            }
        }
    }

    mem->actual_free = mem->free + kern;
    mem->actual_used = mem->used - kern;

    sigar_mem_calc_ram(sigar, mem);

    return SIGAR_OK;
}

int sigar_swap_get(sigar_t *sigar, sigar_swap_t *swap)
{
    kstat_t *ksp;
    kstat_named_t *kn;
    swaptbl_t *stab;
    int num, i;
    char path[PATH_MAX+1]; /* {un,re}used */

    /* see: man swapctl(2) */
    if ((num = swapctl(SC_GETNSWP, NULL)) == -1) {
        return errno;
    }

    stab = malloc(num * sizeof(stab->swt_ent[0]) + sizeof(*stab));

    stab->swt_n = num;
    for (i=0; i<num; i++) {
        stab->swt_ent[i].ste_path = path;
    }

    if ((num = swapctl(SC_LIST, stab)) == -1) {
        free(stab);
        return errno;
    }

    num = num < stab->swt_n ? num : stab->swt_n;
    swap->total = swap->free = 0;
    for (i=0; i<num; i++) {
        if (stab->swt_ent[i].ste_flags & ST_INDEL) {
            continue; /* swap file is being deleted */
        }
        swap->total += stab->swt_ent[i].ste_pages;
        swap->free  += stab->swt_ent[i].ste_free;
    }
    free(stab);

    swap->total <<= sigar->pagesize;
    swap->free  <<= sigar->pagesize;
    swap->used  = swap->total - swap->free;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }
    if (!(ksp = kstat_lookup(sigar->kc, "cpu", -1, "vm"))) {
        swap->page_in = swap->page_out = SIGAR_FIELD_NOTIMPL;
        return SIGAR_OK;
    }

    swap->page_in = swap->page_out = 0;

    /* XXX: these stats do not exist in this form on solaris 8 or 9.
     * they are in the raw cpu_stat struct, but thats not
     * binary compatible
     */
    do {
        if (kstat_read(sigar->kc, ksp, NULL) < 0) {
            break;
        }

        if ((kn = (kstat_named_t *)kstat_data_lookup(ksp, "pgin"))) {
            swap->page_in += kn->value.i64;  /* vmstat -s | grep "page ins" */
        }
        if ((kn = (kstat_named_t *)kstat_data_lookup(ksp, "pgout"))) {
            swap->page_out += kn->value.i64; /* vmstat -s | grep "page outs" */
        }
    } while ((ksp = kstat_next(ksp, "cpu", -1, "vm")));

    return SIGAR_OK;
}

#ifndef KSTAT_NAMED_STR_PTR
/* same offset as KSTAT_NAMED_STR_PTR(brand) */
#define KSTAT_NAMED_STR_PTR(n) (char *)((n)->value.i32)
#endif

static int get_chip_brand(sigar_t *sigar, int processor,
                          sigar_cpu_info_t *info)
{
    kstat_t *ksp = sigar->ks.cpu_info[processor];
    kstat_named_t *brand;

    if (sigar->solaris_version < 10) {
        /* don't bother; doesn't exist. */
        return 0;
    }

    if (ksp &&
        (kstat_read(sigar->kc, ksp, NULL) != -1) &&
        (brand = (kstat_named_t *)kstat_data_lookup(ksp, "brand")))
    {
        char *name = KSTAT_NAMED_STR_PTR(brand);

        char *vendor = "Sun";
        char *vendors[] = {
            "Intel", "AMD", NULL
        };
        int i;

        if (!name) {
            return 0;
        }

        for (i=0; vendors[i]; i++) {
            if (strstr(name, vendors[i])) {
                vendor = vendors[i];
                break;
            }
        }

        SIGAR_SSTRCPY(info->vendor, vendor);
#if 0
        SIGAR_SSTRCPY(info->model, name);
        sigar_cpu_model_adjust(sigar, info);
#endif
        return 1;
    }
    else {
        return 0;
    }
}

static void free_chip_id(void *ptr)
{
    /*noop*/
}

static int get_chip_id(sigar_t *sigar, int processor)
{
    kstat_t *ksp = sigar->ks.cpu_info[processor];
    kstat_named_t *chipid;

    if (ksp &&
        (kstat_read(sigar->kc, ksp, NULL) != -1) &&
        (chipid = (kstat_named_t *)kstat_data_lookup(ksp, "chip_id")))
    {
        return chipid->value.i32;
    }
    else {
        return -1;
    }
}

int sigar_cpu_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    int status, i;

    status = sigar_cpu_list_get(sigar, &sigar->cpulist);

    if (status != SIGAR_OK) {
        return status;
    }

    SIGAR_ZERO(cpu);

    for (i=0; i<sigar->cpulist.number; i++) {
        sigar_cpu_t *xcpu = &sigar->cpulist.data[i];

        cpu->user  += xcpu->user;
        cpu->sys   += xcpu->sys;
        cpu->idle  += xcpu->idle;
        cpu->nice  += xcpu->nice;
        cpu->wait  += xcpu->wait;
        cpu->total = xcpu->total;
    }

    return SIGAR_OK;
}

int sigar_cpu_list_get(sigar_t *sigar, sigar_cpu_list_t *cpulist)
{
    kstat_ctl_t *kc = sigar->kc; 
    kstat_t *ksp;
    uint_t cpuinfo[CPU_STATES];
    unsigned int i;
    int is_debug = SIGAR_LOG_IS_DEBUG(sigar);
    sigar_cache_t *chips;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    if (cpulist == &sigar->cpulist) {
        if (sigar->cpulist.size == 0) {
            /* create once */
            sigar_cpu_list_create(cpulist);
        }
        else {
            /* reset, re-using cpulist.data */
            sigar->cpulist.number = 0;
        }
    }
    else {
        sigar_cpu_list_create(cpulist);
    }

    if (is_debug) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "[cpu_list] OS reports %d CPUs",
                         sigar->ncpu);
    }

    chips = sigar_cache_new(16);
    chips->free_value = free_chip_id;

    for (i=0; i<sigar->ncpu; i++) {
        sigar_cpu_t *cpu;
        char *buf;
        int chip_id;
        sigar_cache_entry_t *ent;

        if (!CPU_ONLINE(sigar->ks.cpuid[i])) {
            sigar_log_printf(sigar, SIGAR_LOG_INFO,
                             "cpu %d (id=%d) is offline",
                             i, sigar->ks.cpuid[i]);
            continue;
        }

        if (!(ksp = sigar->ks.cpu[i])) {
            sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                             "NULL ksp for cpu %d (id=%d)",
                             i, sigar->ks.cpuid[i]);
            continue; /* shouldnot happen */
        }

        if (kstat_read(kc, ksp, NULL) < 0) {
            sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                             "kstat_read failed for cpu %d (id=%d): %s",
                             i, sigar->ks.cpuid[i],
                             sigar_strerror(sigar, errno));
            continue; /* shouldnot happen */
        }

        /*
         * cpu_stat_t is not binary compatible between solaris versions.
         * since cpu_stat is a 'raw' kstat and not 'named' we cannot
         * use name based lookups as we do for others.
         * the start of the cpu_stat_t structure is binary compatible,
         * which looks like so:
         * typedef struct cpu_stat {
         *    kmutex_t        cpu_stat_lock;
         *    cpu_sysinfo_t   cpu_sysinfo;
         *    ...
         *    typedef struct cpu_sysinfo {
         *       ulong cpu[CPU_STATES];
         *       ...
         * we just copy the piece we need below:
         */
        buf = ksp->ks_data;
        buf += sizeof(kmutex_t);
        memcpy(&cpuinfo[0], buf, sizeof(cpuinfo));
        chip_id = sigar->cpu_list_cores ? -1 : get_chip_id(sigar, i);

        if (chip_id == -1) {
            SIGAR_CPU_LIST_GROW(cpulist);
            cpu = &cpulist->data[cpulist->number++];
            SIGAR_ZERO(cpu);
        }
        else {
            /* merge times of logical processors */
            ent = sigar_cache_get(chips, chip_id);
            if (ent->value) {
                cpu = &cpulist->data[(long)ent->value-1];
            }
            else {
                SIGAR_CPU_LIST_GROW(cpulist);
                cpu = &cpulist->data[cpulist->number++];
                ent->value = (void *)(long)cpulist->number;
                SIGAR_ZERO(cpu);

                if (is_debug) {
                    sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                     "[cpu_list] Merging times of"
                                     " logical processors for chip_id=%d",
                                     chip_id);
                }
            }
        }

        cpu->user += SIGAR_TICK2MSEC(cpuinfo[CPU_USER]);
        cpu->sys  += SIGAR_TICK2MSEC(cpuinfo[CPU_KERNEL]);
        cpu->idle += SIGAR_TICK2MSEC(cpuinfo[CPU_IDLE]);
        cpu->wait += SIGAR_TICK2MSEC(cpuinfo[CPU_WAIT]);
        cpu->nice += 0; /* no cpu->nice */
        cpu->total = cpu->user + cpu->sys + cpu->idle + cpu->wait;
    }

    sigar_cache_destroy(chips);

    return SIGAR_OK;
}

int sigar_uptime_get(sigar_t *sigar,
                     sigar_uptime_t *uptime)
{
    if (sigar->boot_time) {
        uptime->uptime = time(NULL) - sigar->boot_time;
    }
    else {
        uptime->uptime = 0; /* XXX: shouldn't happen */
    }

    return SIGAR_OK;
}

static int loadavg_keys[] = {
    KSTAT_SYSTEM_LOADAVG_1,
    KSTAT_SYSTEM_LOADAVG_2,
    KSTAT_SYSTEM_LOADAVG_3
};

int sigar_loadavg_get(sigar_t *sigar,
                      sigar_loadavg_t *loadavg)
{
    kstat_t *ksp;
    int i;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    if (!(ksp = sigar->ks.system)) {
        return -1;
    }

    if (kstat_read(sigar->kc, ksp, NULL) < 0) {
        return -1;
    }

    sigar_koffsets_init_system(sigar, ksp);
    
    for (i=0; i<3; i++) {
        loadavg->loadavg[i] = (double)kSYSTEM(loadavg_keys[i]) / FSCALE;
    }

    return SIGAR_OK;
}

#define LIBPROC "/usr/lib/libproc.so"

#define CHECK_PSYM(s) \
    if (!sigar->s) { \
        sigar_log_printf(sigar, SIGAR_LOG_WARN, \
                         "[%s] Symbol not found: %s", \
                         SIGAR_FUNC, #s); \
        dlclose(sigar->plib); \
        sigar->plib = NULL; \
        return SIGAR_ENOTIMPL; \
    }

static char *proc_readlink(const char *name, char *buffer, size_t size)
{
    int len;

    if ((len = readlink(name, buffer, size-1)) < 0) {
        return NULL;
    }

    buffer[len] = '\0';
    return buffer;
}

static int sigar_init_libproc(sigar_t *sigar)
{
    if (sigar->plib) {
        return SIGAR_OK;
    }

    /* libproc.so ships with 5.8+ */
    /* interface is undocumented, see libproc.h in the sun jdk sources */
    sigar->plib = dlopen(LIBPROC, RTLD_LAZY);

    if (!sigar->plib) {
        sigar_log_printf(sigar, SIGAR_LOG_WARN,
                         "[%s] dlopen(%s) = %s",
                         SIGAR_FUNC, LIBPROC, dlerror());
        return SIGAR_ENOTIMPL;
    }

    sigar->pgrab    = (proc_grab_func_t)dlsym(sigar->plib, "Pgrab");
    sigar->pfree    = (proc_free_func_t)dlsym(sigar->plib, "Pfree");
    sigar->pcreate_agent = (proc_create_agent_func_t)dlsym(sigar->plib, "Pcreate_agent");
    sigar->pdestroy_agent = (proc_destroy_agent_func_t)dlsym(sigar->plib, "Pdestroy_agent");
    sigar->pobjname = (proc_objname_func_t)dlsym(sigar->plib, "Pobjname");
    sigar->pexename = (proc_exename_func_t)dlsym(sigar->plib, "Pexecname");
    sigar->pdirname = (proc_dirname_func_t)dlsym(sigar->plib, "proc_dirname");
    sigar->pfstat64 = (proc_fstat64_func_t)dlsym(sigar->plib, "pr_fstat64");
    sigar->pgetsockopt = (proc_getsockopt_func_t)dlsym(sigar->plib, "pr_getsockopt");
    sigar->pgetsockname = (proc_getsockname_func_t)dlsym(sigar->plib, "pr_getsockname");

    CHECK_PSYM(pgrab);
    CHECK_PSYM(pfree);
    CHECK_PSYM(pobjname);

    return SIGAR_OK;
}

/* from libproc.h, not included w/ solaris distro */
/* Error codes from Pgrab(), Pfgrab_core(), and Pgrab_core() */
#define	G_STRANGE	-1	/* Unanticipated error, errno is meaningful */
#define	G_NOPROC	1	/* No such process */
#define	G_NOCORE	2	/* No such core file */
#define	G_NOPROCORCORE	3	/* No such proc or core (for proc_arg_grab) */
#define	G_NOEXEC	4	/* Cannot locate executable file */
#define	G_ZOMB		5	/* Zombie process */
#define	G_PERM		6	/* No permission */
#define	G_BUSY		7	/* Another process has control */
#define	G_SYS		8	/* System process */
#define	G_SELF		9	/* Process is self */
#define	G_INTR		10	/* Interrupt received while grabbing */
#define	G_LP64		11	/* Process is _LP64, self is ILP32 */
#define	G_FORMAT	12	/* File is not an ELF format core file */
#define	G_ELF		13	/* Libelf error, elf_errno() is meaningful */
#define	G_NOTE		14	/* Required PT_NOTE Phdr not present in core */

static int sigar_pgrab(sigar_t *sigar, sigar_pid_t pid,
                       const char *func,
                       struct ps_prochandle **phandle)
{
    int pstatus;

    if (!(*phandle = sigar->pgrab(pid, 0x01, &pstatus))) {
        switch (pstatus) {
          case G_NOPROC:
            return ESRCH;
          case G_PERM:
            return EACCES;
          default:
            sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                             "[%s] Pgrab error=%d",
                             func, pstatus);
            return ENOTSUP; /*XXX*/
        }
    }

    return SIGAR_OK;
}

int sigar_os_proc_list_get(sigar_t *sigar,
                           sigar_proc_list_t *proclist)
{
    return sigar_proc_list_procfs_get(sigar, proclist);
}

int sigar_proc_mem_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_mem_t *procmem)
{
    int status = sigar_proc_psinfo_get(sigar, pid);
    psinfo_t *pinfo = sigar->pinfo;
    prusage_t usage;

    if (status != SIGAR_OK) {
        return status;
    }

    procmem->size     = pinfo->pr_size << 10;
    procmem->resident = pinfo->pr_rssize << 10;
    procmem->share    = SIGAR_FIELD_NOTIMPL;

    if (sigar_proc_usage_get(sigar, &usage, pid) == SIGAR_OK) {
        procmem->minor_faults = usage.pr_minf;
        procmem->major_faults = usage.pr_majf;
        procmem->page_faults =
            procmem->minor_faults +
            procmem->major_faults;
    }
    else {
        procmem->minor_faults = SIGAR_FIELD_NOTIMPL;
        procmem->major_faults = SIGAR_FIELD_NOTIMPL;
        procmem->page_faults = SIGAR_FIELD_NOTIMPL;
    }

    return SIGAR_OK;
}

int sigar_proc_cred_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_cred_t *proccred)
{
    int status = sigar_proc_psinfo_get(sigar, pid);
    psinfo_t *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    proccred->uid  = pinfo->pr_uid;
    proccred->gid  = pinfo->pr_gid;
    proccred->euid = pinfo->pr_euid;
    proccred->egid = pinfo->pr_egid;

    return SIGAR_OK;
}

#define TIMESTRUCT_2MSEC(t) \
    ((t.tv_sec * MILLISEC) + (t.tv_nsec / (NANOSEC/MILLISEC)))

int sigar_proc_time_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_time_t *proctime)
{
    prusage_t usage;
    int status;

    if ((status = sigar_proc_usage_get(sigar, &usage, pid)) != SIGAR_OK) {
        return status;
    }

    proctime->start_time = usage.pr_create.tv_sec + sigar->boot_time;
    proctime->start_time *= MILLISEC;

    if (usage.pr_utime.tv_sec < 0) {
        /* XXX wtf?  seen on solaris 10, only for the self process */
        pstatus_t pstatus;

        status = sigar_proc_status_get(sigar, &pstatus, pid);
        if (status != SIGAR_OK) {
            return status;
        }

        usage.pr_utime.tv_sec  = pstatus.pr_utime.tv_sec;
        usage.pr_utime.tv_nsec = pstatus.pr_utime.tv_nsec;
        usage.pr_stime.tv_sec  = pstatus.pr_stime.tv_sec;
        usage.pr_stime.tv_nsec = pstatus.pr_stime.tv_nsec;
    }

    proctime->user = TIMESTRUCT_2MSEC(usage.pr_utime);
    proctime->sys  = TIMESTRUCT_2MSEC(usage.pr_stime);
    proctime->total = proctime->user + proctime->sys;

    return SIGAR_OK;
}

int sigar_proc_state_get(sigar_t *sigar, sigar_pid_t pid,
                         sigar_proc_state_t *procstate)
{
    int status = sigar_proc_psinfo_get(sigar, pid);
    psinfo_t *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    SIGAR_SSTRCPY(procstate->name, pinfo->pr_fname);
    procstate->ppid = pinfo->pr_ppid;
    procstate->tty  = pinfo->pr_ttydev;
    procstate->priority = pinfo->pr_lwp.pr_pri;
    procstate->nice     = pinfo->pr_lwp.pr_nice - NZERO;
    procstate->threads  = pinfo->pr_nlwp;
    procstate->processor = pinfo->pr_lwp.pr_onpro;

    switch (pinfo->pr_lwp.pr_state) {
      case SONPROC:
      case SRUN:
        procstate->state = 'R';
        break;
      case SZOMB:
        procstate->state = 'Z';
        break;
      case SSLEEP:
        procstate->state = 'S';
        break;
      case SSTOP:
        procstate->state = 'T';
        break;
      case SIDL:
        procstate->state = 'D';
        break;
    }

    return SIGAR_OK;
}

typedef struct {
    int timestamp;
    char *args;
} pargs_t;

static void pargs_free(void *value)
{
    pargs_t *pargs = (pargs_t *)value;
    if (pargs->args != NULL) {
        free(pargs->args);
    }
    free(pargs);
}

static int ucb_ps_args_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_args_t *procargs,
                           int timestamp)
{
    char buffer[9086], *args=NULL, *arg;
    sigar_cache_entry_t *ent;
    FILE *fp;
    pargs_t *pargs;

    if (!sigar->pargs) {
        sigar->pargs = sigar_cache_new(15);
        sigar->pargs->free_value = pargs_free;
    }

    ent = sigar_cache_get(sigar->pargs, pid);
    if (ent->value) {
        pargs = (pargs_t *)ent->value;
        if (pargs->timestamp != timestamp) {
            if (pargs->args) {
                free(pargs->args);
                pargs->args = NULL;
            }
        }
    }
    else {
        pargs = malloc(sizeof(*pargs));
        pargs->args = NULL;
        ent->value = pargs;
    }

    pargs->timestamp = timestamp;

    if (pargs->args) {
        args = pargs->args;
    }
    else {
        snprintf(buffer, sizeof(buffer),
                 SIGAR_USR_UCB_PS " -ww %ld", (long)pid);

        if (!(fp = popen(buffer, "r"))) {
            return errno;
        }
        /* skip header */
        (void)fgets(buffer, sizeof(buffer), fp);
        if ((args = fgets(buffer, sizeof(buffer), fp))) {
            int len;

            /* skip PID,TT,S,TIME */
            args = sigar_skip_multiple_token(args, 4);
            SIGAR_SKIP_SPACE(args);
            len = strlen(args);
            if (len > 0) {
                args[len-1] = '\0'; /* chop \n */
            }

            pargs->args = malloc(len+1);
            memcpy(pargs->args, args, len);
        }

        pclose(fp);

        if (!args) {
            return ESRCH;
        }
    }

    while (*args && (arg = sigar_getword(&args, ' '))) {
        SIGAR_PROC_ARGS_GROW(procargs);
        procargs->data[procargs->number++] = arg;
    }

    return SIGAR_OK;
}

int sigar_os_proc_args_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_args_t *procargs)
{
    psinfo_t *pinfo;
    int fd, status;
    char buffer[9086];
    char *argvb[56];
    char **argvp = argvb;

    int n;
    size_t nread = 0;
    unsigned int argv_size;

    if ((status = sigar_proc_psinfo_get(sigar, pid)) != SIGAR_OK) {
        return status;
    }
    pinfo = sigar->pinfo;

    if (pinfo->pr_argc == 0) {
        procargs->number = 0;
        return SIGAR_OK;
    }
    else if (pinfo->pr_dmodel != PR_MODEL_NATIVE) {
        /* we are compiled in 32bit mode
         * punt any 64bit native process,
         * sizeof our structures can't handle.
         */
        if (sigar->use_ucb_ps) {
            return ucb_ps_args_get(sigar, pid, procargs,
                                   pinfo->pr_start.tv_sec);
        }
        else {
            return ENOTSUP;
        }
    }

    argv_size = sizeof(*argvp) * pinfo->pr_argc;

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/as");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        if ((errno == EACCES) && sigar->use_ucb_ps) {
            return ucb_ps_args_get(sigar, pid, procargs,
                                   pinfo->pr_start.tv_sec);
        }
        else {
            return PROC_ERRNO;
        }
    }

    if (argv_size > sizeof(argvb)) {
        argvp = malloc(argv_size);
    }

    if ((nread = pread(fd, argvp, argv_size, pinfo->pr_argv)) <= 0) {
        close(fd);
        if (argvp != argvb) {
            free(argvp);
        }
        return errno;
    }

    for (n = 0; n < pinfo->pr_argc; n++) {
        int alen;
        char *arg;

        if ((nread = pread(fd, buffer, sizeof(buffer)-1, (off_t)argvp[n])) <= 0) {
            close(fd);
            if (argvp != argvb) {
                free(argvp);
            }
            return errno;
        }

        buffer[nread] = '\0'; 
        alen = strlen(buffer)+1;
        arg = malloc(alen);
        memcpy(arg, buffer, alen);

        SIGAR_PROC_ARGS_GROW(procargs);
        procargs->data[procargs->number++] = arg;
    }

    if (argvp != argvb) {
        free(argvp);
    }

    close(fd);

    return SIGAR_OK;
}

int sigar_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_env_t *procenv)
{
    psinfo_t *pinfo;
    int fd, status;
    char buffer[BUFSIZ], *offsets[512];
    size_t nread;
    int n=0, max=sizeof(offsets)/sizeof(char *);

    if ((status = sigar_proc_psinfo_get(sigar, pid)) != SIGAR_OK) {
        return status;
    }
    pinfo = sigar->pinfo;

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/as");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        return PROC_ERRNO;
    }

    if ((nread = pread(fd, offsets, sizeof(offsets),
                       pinfo->pr_envp)) <= 0)
    {
        close(fd);
        return errno;
    }

    while ((n < max) && offsets[n]) {
        char *val;
        int klen, vlen, status;
        char key[128]; /* XXX is there a max key size? */

        if ((nread = pread(fd, buffer, sizeof(buffer),
                           (off_t)offsets[n++])) <= 0)
        {
            close(fd);
            return errno;
        }

        val = strchr(buffer, '=');

        if (val == NULL) {
            break; /*XXX*/
        }

        klen = val - buffer;
        SIGAR_SSTRCPY(key, buffer);
        key[klen] = '\0';
        ++val;

        vlen = strlen(val);

        status = procenv->env_getter(procenv->data,
                                     key, klen, val, vlen);

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            break;
        }
    }

    close(fd);

    return SIGAR_OK;
}

int sigar_proc_fd_get(sigar_t *sigar, sigar_pid_t pid,
                      sigar_proc_fd_t *procfd)
{
    int status =
        sigar_proc_fd_count(sigar, pid, &procfd->total);

    return status;
}

static int sigar_proc_path_exe_get(sigar_t *sigar, sigar_pid_t pid,
                                   sigar_proc_exe_t *procexe)
{
    /* solaris 10+ */
    char buffer[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/path/a.out");
    if (!proc_readlink(buffer, procexe->name, sizeof(procexe->name))) {
        procexe->name[0] = '\0';
    }

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/path/cwd");
    if (!proc_readlink(buffer, procexe->cwd, sizeof(procexe->cwd))) {
        procexe->cwd[0] = '\0';
    }

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/path/root");
    if (!proc_readlink(buffer, procexe->root, sizeof(procexe->root))) {
        procexe->root[0] = '\0';
    }

    return SIGAR_OK;
}

static int proc_module_get_exe(void *data, char *name, int len)
{
    sigar_proc_exe_t *procexe = (sigar_proc_exe_t *)data;
    SIGAR_STRNCPY(procexe->name, name, sizeof(procexe->name));
    return !SIGAR_OK; /* break loop */
}

static int sigar_which_exe_get(sigar_t *sigar, sigar_proc_exe_t *procexe)
{
    char *path = getenv("PATH");
    char exe[PATH_MAX];
    if (path == NULL) {
        return EINVAL;
    }

    while (path) {
        char *ptr = strchr(path, ':');
        if (!ptr) {
            break;
        }
        exe[0] = '\0';
        strncat(exe, path, ptr-path);
        strncat(exe, "/", 1);
        strcat(exe, procexe->name);
        if (access(exe, X_OK) == 0) {
            SIGAR_STRNCPY(procexe->name, exe, sizeof(procexe->name));
            break;
        }
        path = ptr+1;
    }

    return ENOENT;
}

int sigar_proc_exe_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_exe_t *procexe)
{
    int status;
    char buffer[BUFSIZ];
    struct ps_prochandle *phandle;

    if (sigar->solaris_version >= 10) {
        return sigar_proc_path_exe_get(sigar, pid, procexe);
    }

    if ((status = sigar_init_libproc(sigar)) != SIGAR_OK) {
        return status;
    }

    procexe->name[0] = '\0';

    /* Pgrab would return G_SELF error */
    if (pid == sigar_pid_get(sigar)) {
        sigar_proc_modules_t procmods;
        procmods.module_getter = proc_module_get_exe;
        procmods.data = procexe;

        status =
            sigar_dlinfo_modules(sigar, &procmods);
        if (status == SIGAR_OK) {
            if (procexe->name[0] != '/') {
                sigar_which_exe_get(sigar, procexe);
            }
        }
    }
    else {
        status = sigar_pgrab(sigar, pid, SIGAR_FUNC, &phandle);

        if (status == SIGAR_OK) {
            sigar->pexename(phandle, procexe->name, sizeof(procexe->name));
            sigar->pfree(phandle);
        }
    }

    if (procexe->name[0] == '\0') {
        /*XXX*/
    }

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/cwd");

    if (!sigar->pdirname(buffer, procexe->cwd, sizeof(procexe->cwd))) {
        procexe->cwd[0] = '\0';
    }

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/root");

    if (!(sigar->pdirname(buffer, procexe->root, sizeof(procexe->root)))) {
        procexe->root[0] = '\0';
    }

    return SIGAR_OK;
}

static int sigar_read_xmaps(sigar_t *sigar, 
                            prxmap_t *xmaps, int total,
                            unsigned long *last_inode,
                            struct ps_prochandle *phandle,
                            sigar_proc_modules_t *procmods)
{
    int status, i;
    unsigned long inode;
    char buffer[BUFSIZ];

    for (i=0; i<total; i++) {
        if (xmaps[i].pr_mflags & MA_ANON) {
            continue; /* heap, stack, etc */
        }

        inode = xmaps[i].pr_ino;

        if ((inode == 0) || (inode == *last_inode)) {
            *last_inode = 0;
            continue;
        }

        *last_inode = inode;

        sigar->pobjname(phandle, xmaps[i].pr_vaddr, buffer, sizeof(buffer));

        status = 
            procmods->module_getter(procmods->data, buffer, strlen(buffer));

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            return status;
        }
    }

    return SIGAR_OK;
}

static int sigar_pgrab_modules(sigar_t *sigar, sigar_pid_t pid,
                               sigar_proc_modules_t *procmods)
{
    int fd, pstatus;
    off_t map_size, nread;
    unsigned long last_inode = 0;
    prxmap_t xmaps[15]; /* ~2K */
    struct ps_prochandle *phandle;
    struct stat statbuf;
    char buffer[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/xmap");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        return errno;
    }

    if (fstat(fd, &statbuf) < 0) {
        close(fd);
        return errno;
    }

    map_size = statbuf.st_size;

    if (SIGAR_LOG_IS_DEBUG(sigar)) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "[%s] pid=%d, size=%d",
                         SIGAR_FUNC, pid, map_size);
    }

    if ((pstatus = sigar_init_libproc(sigar)) != SIGAR_OK) {
        close(fd);
        return pstatus;
    }

    pstatus = sigar_pgrab(sigar, pid, SIGAR_FUNC, &phandle);

    if (pstatus != SIGAR_OK) {
        close(fd);
        return pstatus;
    }

    for (nread=0; nread<statbuf.st_size; ) {
        off_t wanted = map_size > sizeof(xmaps) ? sizeof(xmaps) : map_size;
        int total = wanted / sizeof(prxmap_t);

        if (pread(fd, xmaps, wanted, nread) != wanted) {
            close(fd);
            sigar->pfree(phandle);
            return errno;
        }

        if (SIGAR_LOG_IS_DEBUG(sigar)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[%s] nread=%d, map_size=%d, wanted=%d, total=%d",
                             SIGAR_FUNC,
                             nread, map_size, wanted, total);
        }

        if (sigar_read_xmaps(sigar, xmaps, total,
                             &last_inode,
                             phandle, procmods) != SIGAR_OK)
        {
            break;
        }

        nread += wanted;
        map_size -= wanted;
    }

    close(fd);

    sigar->pfree(phandle);

    return SIGAR_OK;
}

int sigar_proc_modules_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_modules_t *procmods)
{
    if (pid == sigar_pid_get(sigar)) {
        /* Pgrab would return G_SELF, this is faster anyhow */
        /* XXX one difference to Pgrab, first entry is not the exe name */
        return sigar_dlinfo_modules(sigar, procmods);
    }
    else {
        return sigar_pgrab_modules(sigar, pid, procmods);
    }
}

#define TIME_NSEC(t) \
    (SIGAR_SEC2NANO((t).tv_sec) + (sigar_uint64_t)(t).tv_nsec)

int sigar_thread_cpu_get(sigar_t *sigar,
                         sigar_uint64_t id,
                         sigar_thread_cpu_t *cpu)
{
    struct lwpinfo info;

    if (id != 0) {
        return SIGAR_ENOTIMPL;
    }

    _lwp_info(&info);

    cpu->user  = TIME_NSEC(info.lwp_utime);
    cpu->sys   = TIME_NSEC(info.lwp_stime);
    cpu->total = TIME_NSEC(info.lwp_utime) + TIME_NSEC(info.lwp_stime);

    return SIGAR_OK;
}

#include <sys/mnttab.h>

int sigar_os_fs_type_get(sigar_file_system_t *fsp)
{
    char *type = fsp->sys_type_name;

    switch (*type) {
      case 'u':
        if (strEQ(type, "ufs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
        /* XXX */
    }

    return fsp->type;
}

int sigar_file_system_list_get(sigar_t *sigar,
                               sigar_file_system_list_t *fslist)
{
    struct mnttab ent;
    sigar_file_system_t *fsp;
    FILE *fp = fopen(MNTTAB, "r");

    if (!fp) {
        return errno;
    }

    sigar_file_system_list_create(fslist);

    while (getmntent(fp, &ent) == 0) {
        if (strstr(ent.mnt_mntopts, "ignore")) {
            continue; /* e.g. vold */
        }

        SIGAR_FILE_SYSTEM_LIST_GROW(fslist);

        fsp = &fslist->data[fslist->number++];

        SIGAR_SSTRCPY(fsp->dir_name, ent.mnt_mountp);
        SIGAR_SSTRCPY(fsp->dev_name, ent.mnt_special);
        SIGAR_SSTRCPY(fsp->sys_type_name, ent.mnt_fstype);
        SIGAR_SSTRCPY(fsp->options, ent.mnt_mntopts);
        sigar_fs_type_init(fsp);
    }

    fclose(fp);

    return SIGAR_OK;
}

typedef struct {
    char device[PATH_MAX];
    char name[8];
    int instance;
} fsdev_path_t;

typedef struct {
    char name[256];
    int is_partition;
    sigar_disk_usage_t disk;
} iodev_t;

static fsdev_path_t *get_fsdev_paths(sigar_t *sigar,
                                     sigar_file_system_list_t *fslist)
{
    int i, ndisk, size;
    char buffer[BUFSIZ], *ptr;
    char *dev, *inst, *drv;
    fsdev_path_t *paths, *mapping;
    FILE *fp = fopen("/etc/path_to_inst", "r");

    if (!fp) {
        return NULL;
    }

    for (i=0, ndisk=0; i<fslist->number; i++) {
        sigar_file_system_t *fsp = &fslist->data[i];
        if (fsp->type == SIGAR_FSTYPE_LOCAL_DISK) {
            ndisk++;
        }
    }

    size = sizeof(*paths) * (ndisk+1);
    mapping = paths = malloc(size);
    memset(mapping, '\0', size);

    while ((ptr = fgets(buffer, sizeof(buffer), fp))) {
        /* eat dust java */
        char *q;

        SIGAR_SKIP_SPACE(ptr);
        if (*ptr == '#') {
            continue;
        }
        if (*ptr == '"') {
            ptr++;
        }
        dev = ptr;
        if (!(q = strchr(ptr, '"'))) {
            continue;
        }
        ptr = q+1;
        *q = '\0';
        SIGAR_SKIP_SPACE(ptr);
        inst = ptr;
        while (sigar_isdigit(*ptr)) {
            ptr++;
        }
        *ptr = '\0';
        ptr++;
        SIGAR_SKIP_SPACE(ptr);
        if (*ptr == '"') {
            ptr++;
        }
        drv = ptr;
        if (!(q = strchr(ptr, '"'))) {
            continue;
        }
        *q = '\0';

        if (!(strEQ(drv, "sd") ||
              strEQ(drv, "ssd") ||
              strEQ(drv, "st") ||
              strEQ(drv, "dad") ||
              strEQ(drv, "cmdk")))
        {
            continue;
        }

        paths->instance = atoi(inst);
        if (!kstat_lookup(sigar->kc, drv, paths->instance, NULL)) {
            continue;
        }

        SIGAR_SSTRCPY(paths->device, dev);
        SIGAR_SSTRCPY(paths->name, drv);

        if (--ndisk < 0) {
            /* XXX prevent overflow */
            break;
        }
        paths++;
    }
    fclose(fp);

    return mapping;
}

static int create_fsdev_cache(sigar_t *sigar)
{
    fsdev_path_t *paths, *mapping;
    sigar_file_system_list_t fslist;
    int i, j;
    int status;
    int debug = SIGAR_LOG_IS_DEBUG(sigar);

    sigar->fsdev = sigar_cache_new(15);

    status = sigar_file_system_list_get(sigar, &fslist);
    
    if (status != SIGAR_OK) {
        return status;
    }

    if (!(mapping = get_fsdev_paths(sigar, &fslist))) {
        sigar_file_system_list_destroy(sigar, &fslist);
        return ENOENT;
    }

    for (i=0; i<fslist.number; i++) {
        sigar_file_system_t *fsp = &fslist.data[i];

        if (fsp->type == SIGAR_FSTYPE_LOCAL_DISK) {
            char device[PATH_MAX+1], *ptr=device;
            int len = readlink(fsp->dev_name, device, sizeof(device)-1);
            char *s;
            char partition;

            if (len < 0) {
                continue;
            }
            device[len] = '\0';

            if (debug) {
                sigar_log_printf(sigar, SIGAR_LOG_DEBUG, "[fsdev] name=%s, dev=%s",
                                 fsp->dev_name, device);
            }

            while (strnEQ(ptr, "../", 3)) {
                ptr += 3;
            }
            if (strnEQ(ptr, "devices", 7)) {
                ptr += 7;
            }
            if ((s = strchr(ptr, ':'))) {
                partition = *(s+1);
            }
            else {
                continue;
            }

            for (j=0, paths=mapping; paths->name[0]; j++) {
                if (strnEQ(paths->device, ptr, strlen(paths->device))) {
                    sigar_cache_entry_t *ent;
                    struct stat sb;
                    int retval = stat(fsp->dir_name, &sb);
                    iodev_t *iodev;

                    if (retval == 0) {
                        iodev = malloc(sizeof(*iodev));

                        SIGAR_DISK_STATS_INIT(&iodev->disk);
                        /* e.g. sd9,g
                         * module    == sd
                         * instance  == 9
                         * partition == 8
                         */
                        snprintf(iodev->name, sizeof(iodev->name), "%s%d,%c",
                                 paths->name, paths->instance, partition);

                        ent = sigar_cache_get(sigar->fsdev, SIGAR_FSDEV_ID(sb));
                        ent->value = iodev;

                        if (debug) {
                            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                             "[fsdev] map %s -> %s",
                                             fsp->dir_name, iodev->name);
                        }
                    }
                    break;
                }
                paths++;
            }
        }
    }

    free(mapping);
    sigar_file_system_list_destroy(sigar, &fslist);

    return SIGAR_OK;
}

static int io_kstat_read(sigar_t *sigar,
                         sigar_disk_usage_t *disk,
                         kstat_t *ksp)
{
    kstat_io_t *io;

    kstat_read(sigar->kc, ksp, NULL);

    io = (kstat_io_t *)ksp->ks_data;

    disk->reads       = io->reads;
    disk->writes      = io->writes;
    disk->read_bytes  = io->nread;
    disk->write_bytes = io->nwritten;
    disk->qtime       = io->wlentime;
    disk->rtime       = io->rlentime;
    disk->wtime       = io->wlentime;
    disk->time        = disk->rtime + disk->wtime;
    disk->snaptime    = ksp->ks_snaptime;

    return SIGAR_OK;
}


static int sigar_kstat_disk_usage_get(sigar_t *sigar, const char *name,
                                      sigar_disk_usage_t *disk,
                                      kstat_t **kio)
{
    kstat_t *ksp;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    for (ksp = sigar->kc->kc_chain;
         ksp;
         ksp = ksp->ks_next)
    {
        if (ksp->ks_type != KSTAT_TYPE_IO) {
            continue;
        }
        if (strEQ(ksp->ks_name, name)) {
            int status = io_kstat_read(sigar, disk, ksp);
            *kio = ksp;
            return status;
        }
    }

    return ENXIO;
}

static int simple_hash(const char *s)
{
    int hash = 0;
    while (*s) {
        hash = 31*hash + *s++; 
    }
    return hash;
}

int sigar_disk_usage_get(sigar_t *sigar, const char *name,
                         sigar_disk_usage_t *disk)
{
    kstat_t *ksp;
    int status;
    iodev_t *iodev = NULL;
    sigar_cache_entry_t *ent;
    sigar_uint64_t id;

    SIGAR_DISK_STATS_INIT(disk);

    if (!sigar->fsdev) {
        if (create_fsdev_cache(sigar) != SIGAR_OK) {
            return SIGAR_OK;
        }
    }

    if (*name == '/') {
        struct stat sb;

        if (stat(name, &sb) < 0) {
            return errno;
        }

        id = SIGAR_FSDEV_ID(sb);
        ent = sigar_cache_get(sigar->fsdev, id);
        if (ent->value == NULL) {
            return ENXIO;
        }
        iodev = (iodev_t *)ent->value;

        status = sigar_kstat_disk_usage_get(sigar, iodev->name, disk, &ksp);
    }
    else {
        status = sigar_kstat_disk_usage_get(sigar, name, disk, &ksp);
        if (status != SIGAR_OK) {
            return status;
        }
        id = simple_hash(name); /*XXX*/
        ent = sigar_cache_get(sigar->fsdev, id);
        if (ent->value) {
            iodev = (iodev_t *)ent->value;
        }
        else {
            ent->value = iodev = malloc(sizeof(*iodev));
            SIGAR_SSTRCPY(iodev->name, name);
            SIGAR_DISK_STATS_INIT(&iodev->disk);
        }
    }

    /* service_time formula derived from opensolaris.org:iostat.c */
    if ((status == SIGAR_OK) && iodev) {
        sigar_uint64_t delta;
        double avw, avr, tps, mtps; 
        double etime, hr_etime;

        if (iodev->disk.snaptime) {
            delta = disk->snaptime - iodev->disk.snaptime;
        }
        else {
            delta = ksp->ks_crtime - ksp->ks_snaptime;
        }

        hr_etime = (double)delta;
        if (hr_etime == 0.0) {
            hr_etime = (double)NANOSEC;
        }
        etime = hr_etime / (double)NANOSEC;

        tps =
            (((double)(disk->reads - iodev->disk.reads)) / etime) +
            (((double)(disk->writes - iodev->disk.writes)) / etime);

        delta = disk->wtime - iodev->disk.wtime;
        if (delta) {
            avw = (double)delta;
            avw /= hr_etime;
        }
        else {
            avw = 0.0;
        }

        delta = disk->rtime - iodev->disk.rtime;
        if (delta) {
            avr = (double)delta;
            avr /= hr_etime;
        }
        else {
            avr = 0.0;
        }

        disk->queue = avw;
        disk->service_time = 0.0;

        if (tps && (avw != 0.0 || avr != 0.0)) {
            mtps = 1000.0 / tps;
            if (avw != 0.0) {
                disk->service_time += avw * mtps;
            }
            if (avr != 0.0) {
                disk->service_time += avr * mtps;
            }
        }

        memcpy(&iodev->disk, disk, sizeof(iodev->disk));
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

    sigar_disk_usage_get(sigar, dirname, &fsusage->disk);

    return SIGAR_OK;
}

int sigar_cpu_info_list_get(sigar_t *sigar,
                            sigar_cpu_info_list_t *cpu_infos)
{
    processor_info_t stats;
    unsigned int i;
    int status = SIGAR_OK;
    int brand = -1;
    sigar_cache_t *chips;
    int is_debug = SIGAR_LOG_IS_DEBUG(sigar);
    int nsockets = 0;

    if (sigar_kstat_update(sigar) == -1) { /* for sigar->ncpu */
        return errno;
    }

    /*
     * stats we care about will be the same for each
     * online processor, so just grab the first.
     */
    for (i=0; i<sigar->ncpu; i++) {
        processorid_t id = sigar->ks.cpuid[i];

        if ((status = processor_info(id, &stats)) < 0) {
            continue;
        }
        else {
            status = SIGAR_OK;
            break;
        }
    }

    if (status != SIGAR_OK) {
        /* should never happen */
        return ENOENT;
    }

    sigar_cpu_info_list_create(cpu_infos);
    chips = sigar_cache_new(16);
    chips->free_value = free_chip_id;

    for (i=0; i<sigar->ncpu; i++) {
        sigar_cpu_info_t *info;
        int chip_id = get_chip_id(sigar, i);

        if (chip_id != -1) {
            sigar_cache_entry_t *ent =
                sigar_cache_get(chips, chip_id);

            if (ent->value) {
                if (!sigar->cpu_list_cores) {
                    continue;
                }
            }
            else {
                ++nsockets;
                ent->value = chips; /*anything non-NULL*/
                if (is_debug) {
                    sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                     "[cpu_list] Merging info of"
                                     " logical processors for chip_id=%d",
                                     chip_id);
                }
            }
        }
        else {
            ++nsockets;
        }

        SIGAR_CPU_INFO_LIST_GROW(cpu_infos);

        info = &cpu_infos->data[cpu_infos->number++];

        SIGAR_SSTRCPY(info->model, stats.pi_processor_type);

        if (brand == -1) {
            brand = get_chip_brand(sigar, i, info);
        }

        if (strEQ(info->model, "i386")) {
            if (!brand) {
                /* assume Intel on x86 */
                SIGAR_SSTRCPY(info->vendor, "Intel");
            }
            SIGAR_SSTRCPY(info->model, "x86");
        }
        else {
            if (!brand) {
                /* assume Sun */
                SIGAR_SSTRCPY(info->vendor, "Sun");
            }
            /* s/sparc/Sparc/ */
            info->model[0] = toupper(info->model[0]);
        }

        if (brand) {
            SIGAR_SSTRCPY(info->vendor, cpu_infos->data[0].vendor);
        }

        info->mhz = stats.pi_clock;
        info->cache_size = SIGAR_FIELD_NOTIMPL; /*XXX*/
    }

    sigar_cache_destroy(chips);

    for (i=0; i<cpu_infos->number; i++) {
        sigar_cpu_info_t *info = &cpu_infos->data[i];
        info->total_sockets = nsockets;
        info->total_cores = sigar->ncpu;
        info->cores_per_socket = sigar->ncpu / nsockets;
    }

    return SIGAR_OK;
}

int sigar_net_route_list_get(sigar_t *sigar,
                             sigar_net_route_list_t *routelist)

{
    char *data;
    int len, rc;
    struct opthdr *op;
    size_t nread=0, size=0;
    const char *size_from;

    sigar_net_route_list_create(routelist);

    while ((rc = get_mib2(&sigar->mib2, &op, &data, &len)) == GET_MIB2_OK) {
        mib2_ipRouteEntry_t *entry;
        char *end;

        if (op->level != MIB2_IP) {
            continue;
        }

        if (op->name == 0) {
            /* we want to use this size for bincompat */
            size = ((mib2_ip_t *)data)->ipRouteEntrySize;
            continue;
        }
        else if (op->name != MIB2_IP_21) {
            continue;
        }

        if (size == 0) {
            size_from = "sizeof";
            size = sizeof(*entry);
        }
        else {
            size_from = "mib2_ip";
        }

        if (SIGAR_LOG_IS_DEBUG(sigar)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[route_list] ipRouteEntrySize=%d (from %s)",
                             size, size_from);
        }

        for (entry = (mib2_ipRouteEntry_t *)data, end = data + len;
             (char *)entry < end;
             nread+=size, entry = (mib2_ipRouteEntry_t *)((char *)data+nread))
        {
            sigar_net_route_t *route;
            int type = entry->ipRouteInfo.re_ire_type;

            /* filter same as netstat -r */
            if ((type == IRE_CACHE) ||
                (type == IRE_BROADCAST) ||
                (type == IRE_LOCAL))
            {
                continue;
            }

            SIGAR_NET_ROUTE_LIST_GROW(routelist);
            route = &routelist->data[routelist->number++];

            sigar_net_address_set(route->destination,
                                  entry->ipRouteDest);

            sigar_net_address_set(route->gateway,
                                  entry->ipRouteNextHop);

            sigar_net_address_set(route->mask,
                                  entry->ipRouteMask);

            route->refcnt      = entry->ipRouteInfo.re_ref;
            route->irtt        = entry->ipRouteInfo.re_rtt;
            route->metric      = entry->ipRouteMetric1;

            SIGAR_SSTRCPY(route->ifname, entry->ipRouteIfIndex.o_bytes);

            route->flags = RTF_UP;
            if ((route->destination.addr.in == 0) &&
                (route->mask.addr.in == 0))
            {
                route->flags |= RTF_GATEWAY;
            }

            route->use = route->window = route->mtu = 
                SIGAR_FIELD_NOTIMPL; /*XXX*/
        }
    }

    if (rc != GET_MIB2_EOD) {
        close_mib2(&sigar->mib2);
        return SIGAR_EMIB2;
    }

    return SIGAR_OK;
}

static void ifstat_kstat_common(sigar_net_interface_stat_t *ifstat,
                                kstat_named_t *data, int ndata)
{
    int i;

    for (i=0; i<ndata; i++) {
        sigar_uint64_t value = data[i].value.KSTAT_UINT;

        char *ptr = data[i].name;

        switch (*ptr) {
          case 'c':
            if (strEQ(ptr, "collisions")) {
                ifstat->tx_collisions = value;
            }
            break;
          case 'd':
            if (strEQ(ptr, "drop")) {
                ifstat->rx_dropped = value;
                ifstat->tx_dropped = value;
            }
            break;
          case 'i':
            if (strEQ(ptr, "ipackets")) {
                if (ifstat->rx_packets == 0) {
                    ifstat->rx_packets = value;
                }
            }
            else if (strEQ(ptr, "ipackets64")) {
                ifstat->rx_packets = data[i].value.ui64;
            }
            else if (strEQ(ptr, "ierrors")) {
                ifstat->rx_errors = value;
            }
            else if (strEQ(ptr, "ifspeed")) {
                ifstat->speed = value;
            }
            break;
          case 'f':
            if (strEQ(ptr, "framing")) {
                ifstat->rx_frame = value;
            }
            break;
          case 'm':
            if (strEQ(ptr, "missed")) {
                ifstat->rx_dropped = value;
                ifstat->tx_dropped = value;
            }
            break;
          case 'n':
            if (strEQ(ptr, "nocarrier")) {
                ifstat->tx_carrier = value;
            }
            break;
          case 'o':
            if (strEQ(ptr, "obytes")) {
                if (ifstat->tx_bytes == 0) {
                    ifstat->tx_bytes = value;
                }
            }
            else if (strEQ(ptr, "obytes64")) {
                ifstat->tx_bytes = data[i].value.ui64;
            }
            else if (strEQ(ptr, "oerrors")) {
                ifstat->tx_errors = value;
            }
            else if (strEQ(ptr, "oflo")) {
                ifstat->tx_overruns = value;
            }
            else if (strEQ(ptr, "opackets")) {
                if (ifstat->tx_packets == 0) {
                    ifstat->tx_packets = value;
                }
            }
            else if (strEQ(ptr, "opackets64")) {
                ifstat->tx_packets = data[i].value.ui64;
            }
            else if (strEQ(ptr, "toolong_errors")) {
                ifstat->tx_overruns = value;
            }
            break;
          case 'r':
            if (strEQ(ptr, "rbytes")) {
                if (ifstat->rx_bytes == 0) {
                    ifstat->rx_bytes = value;
                }
            }
            else if (strEQ(ptr, "rbytes64")) {
                ifstat->rx_bytes = data[i].value.ui64;
            }
            else if (strEQ(ptr, "rx_overflow")) {
                ifstat->rx_overruns = value;
            }
            break;
          default:
            break;
        }
    }
}

static int sigar_net_ifstat_get_any(sigar_t *sigar, const char *name,
                                    sigar_net_interface_stat_t *ifstat)
{
    kstat_ctl_t *kc = sigar->kc; 
    kstat_t *ksp;
    kstat_named_t *data;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    if (!(ksp = kstat_lookup(kc, NULL, -1, (char *)name))) {
        return ENXIO;
    }

    if (kstat_read(kc, ksp, NULL) < 0) {
        return ENOENT;
    }

    data = (kstat_named_t *)ksp->ks_data;

    ifstat_kstat_common(ifstat, data, ksp->ks_ndata);

    return SIGAR_OK;
}

/* loopback interface only has rx/tx packets */
static int sigar_net_ifstat_get_lo(sigar_t *sigar, const char *name,
                                   sigar_net_interface_stat_t *ifstat)
{
    ifstat->rx_packets    = 0;
    ifstat->rx_bytes      = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_errors     = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_dropped    = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_overruns   = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_frame      = SIGAR_FIELD_NOTIMPL;

    ifstat->tx_packets    = 0;
    ifstat->tx_bytes      = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_errors     = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_dropped    = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_overruns   = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_collisions = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_carrier    = SIGAR_FIELD_NOTIMPL;

    ifstat->speed         = SIGAR_FIELD_NOTIMPL;

    return sigar_net_ifstat_get_any(sigar, name, ifstat);
}

int sigar_net_interface_stat_get(sigar_t *sigar, const char *name,
                                 sigar_net_interface_stat_t *ifstat)
{
    ifstat->speed = SIGAR_FIELD_NOTIMPL;

    if (strnEQ(name, "lo", 2)) {
        return sigar_net_ifstat_get_lo(sigar, name, ifstat);
    }
    else {
        SIGAR_ZERO(ifstat);
        return sigar_net_ifstat_get_any(sigar, name, ifstat);
    }
}

#define TCPQ_SIZE(s) ((s) >= 0 ? (s) : 0)

static int tcp_connection_get(sigar_net_connection_walker_t *walker,
                              struct mib2_tcpConnEntry *entry,
                              int len)
{
    int flags = walker->flags;
    int status;
    char *end = (char *)entry + len;

    while ((char *)entry < end) {
        int state = entry->tcpConnEntryInfo.ce_state;

        if (((flags & SIGAR_NETCONN_SERVER) && (state == TCPS_LISTEN)) ||
            ((flags & SIGAR_NETCONN_CLIENT) && (state != TCPS_LISTEN)))
        {
            sigar_net_connection_t conn;

            SIGAR_ZERO(&conn);

            sigar_net_address_set(conn.local_address, entry->tcpConnLocalAddress);
            sigar_net_address_set(conn.remote_address, entry->tcpConnRemAddress);

            conn.local_port = entry->tcpConnLocalPort;
            conn.remote_port = entry->tcpConnRemPort;
            conn.type = SIGAR_NETCONN_TCP;
            conn.send_queue =
                TCPQ_SIZE(entry->tcpConnEntryInfo.ce_snxt -
                          entry->tcpConnEntryInfo.ce_suna - 1);
            conn.receive_queue =
                TCPQ_SIZE(entry->tcpConnEntryInfo.ce_rnxt -
                          entry->tcpConnEntryInfo.ce_rack);

            switch (state) {
              case TCPS_CLOSED:
                conn.state = SIGAR_TCP_CLOSE;
                break;
              case TCPS_IDLE:
                conn.state = SIGAR_TCP_IDLE;
                break;
              case TCPS_BOUND:
                conn.state = SIGAR_TCP_BOUND;
                break;
              case TCPS_LISTEN:
                conn.state = SIGAR_TCP_LISTEN;
                break;
              case TCPS_SYN_SENT:
                conn.state = SIGAR_TCP_SYN_SENT;
                break;
              case TCPS_SYN_RCVD:
                conn.state = SIGAR_TCP_SYN_RECV;
                break;
              case TCPS_ESTABLISHED:
                conn.state = SIGAR_TCP_ESTABLISHED;
                break;
              case TCPS_CLOSE_WAIT:
                conn.state = SIGAR_TCP_CLOSE_WAIT;
                break;
              case TCPS_FIN_WAIT_1:
                conn.state = SIGAR_TCP_FIN_WAIT1;
                break;
              case TCPS_CLOSING:
                conn.state = SIGAR_TCP_CLOSING;
                break;
              case TCPS_LAST_ACK:
                conn.state = SIGAR_TCP_LAST_ACK;
                break;
              case TCPS_FIN_WAIT_2:
                conn.state = SIGAR_TCP_FIN_WAIT2;
                break;
              case TCPS_TIME_WAIT:
                conn.state = SIGAR_TCP_TIME_WAIT;
                break;
              default:
                conn.state = SIGAR_TCP_UNKNOWN;
                break;
            }

            status = walker->add_connection(walker, &conn);
            if (status != SIGAR_OK) {
                return status;
            }
        }

        entry++;
    }

    return SIGAR_OK;
}

static int udp_connection_get(sigar_net_connection_walker_t *walker,
                              struct mib2_udpEntry *entry,
                              int len)
{
    int flags = walker->flags;
    int status;
    char *end = (char *)entry + len;

    while ((char *)entry < end) {
        int state = entry->udpEntryInfo.ue_state;

        /* XXX dunno if this state check is right */
        if (((flags & SIGAR_NETCONN_SERVER) && (state == MIB2_UDP_idle)) ||
            ((flags & SIGAR_NETCONN_CLIENT) && (state != MIB2_UDP_idle)))
        {
            sigar_net_connection_t conn;

            SIGAR_ZERO(&conn);

            sigar_net_address_set(conn.local_address, entry->udpLocalAddress);
            sigar_net_address_set(conn.remote_address, 0);

            conn.local_port = entry->udpLocalPort;
            conn.remote_port = 0;
            conn.type = SIGAR_NETCONN_UDP;

            status = walker->add_connection(walker, &conn);
            if (status != SIGAR_OK) {
                return status;
            }
        }

        entry++;
    }

    return SIGAR_OK;
}

int sigar_net_connection_walk(sigar_net_connection_walker_t *walker)
{
    sigar_t *sigar = walker->sigar;
    int flags = walker->flags;
    int status;
    int want_tcp = flags & SIGAR_NETCONN_TCP;
    int want_udp = flags & SIGAR_NETCONN_UDP;
    char *data;
    int len;
    int rc;
    struct opthdr *op;

    while ((rc = get_mib2(&sigar->mib2, &op, &data, &len)) == GET_MIB2_OK) {
        if ((op->level == MIB2_TCP) && 
            (op->name == MIB2_TCP_13) &&
            want_tcp)
        {
            status =
                tcp_connection_get(walker,
                                   (struct mib2_tcpConnEntry *)data,
                                   len);
        }
        else if ((op->level == MIB2_UDP) && 
                 (op->name == MIB2_UDP_5) &&
                 want_udp)
        {
            status =
                udp_connection_get(walker,
                                   (struct mib2_udpEntry *)data,
                                   len);
        }
        else {
            status = SIGAR_OK;
        }

        if (status != SIGAR_OK) {
            break;
        }
    }

    if (rc != GET_MIB2_EOD) {
        close_mib2(&sigar->mib2);
        return SIGAR_EMIB2;
    }

    return SIGAR_OK;
}

SIGAR_DECLARE(int)
sigar_tcp_get(sigar_t *sigar,
              sigar_tcp_t *tcp)
{
    char *data;
    int len;
    int rc;
    struct opthdr *op;
    mib2_tcp_t *mib = NULL;

    while ((rc = get_mib2(&sigar->mib2, &op, &data, &len)) == GET_MIB2_OK) {
        if ((op->level == MIB2_TCP) && (op->name == 0)) {
            mib = (mib2_tcp_t *)data;
            break;
        }
    }

    if (mib) {
        tcp->active_opens = mib->tcpActiveOpens;
        tcp->passive_opens = mib->tcpPassiveOpens;
        tcp->attempt_fails = mib->tcpAttemptFails;
        tcp->estab_resets = mib->tcpEstabResets;
        tcp->curr_estab = mib->tcpCurrEstab;
        tcp->in_segs = mib->tcpInSegs;
        tcp->out_segs = mib->tcpOutSegs;
        tcp->retrans_segs = mib->tcpRetransSegs;
        tcp->in_errs = SIGAR_FIELD_NOTIMPL; /* XXX mib2_ip_t.tcpInErrs */
        tcp->out_rsts = mib->tcpOutRsts;
        return SIGAR_OK;
    }
    else {
        return SIGAR_ENOTIMPL;
    }
}

static int sigar_nfs_get(sigar_t *sigar,
                         char *type, 
                         char **names,
                         char *nfs)
{
    size_t offset;
    kstat_t *ksp;
    int i;

    if (sigar_kstat_update(sigar) == -1) {
        return errno;
    }

    if (!(ksp = kstat_lookup(sigar->kc, "nfs", 0, type))) {
        return SIGAR_ENOTIMPL;
    }

    if (kstat_read(sigar->kc, ksp, NULL) < 0) {
        return errno;
    }

    for (i=0, offset=0;
         names[i];
         i++, offset+=sizeof(sigar_uint64_t))
    {
        sigar_uint64_t val;
        kstat_named_t *kv =
            kstat_data_lookup(ksp, names[i]);

        if (kv) {
            val = kv->value.ui64;
        }
        else {
            val = -1;
        }

        *(sigar_uint64_t *)((char *)nfs + offset) = val;
    }

    return SIGAR_OK;
}

static char *nfs_v2_names[] = {
    "null",
    "getattr",
    "setattr",
    "root",
    "lookup",
    "readlink",
    "read",
    "wrcache",
    "write",
    "create",
    "remove",
    "rename",
    "link",
    "symlink",
    "mkdir",
    "rmdir",
    "readdir",
    "statfs",
    NULL
};

int sigar_nfs_client_v2_get(sigar_t *sigar,
                            sigar_nfs_client_v2_t *nfs)
{
    return sigar_nfs_get(sigar, "rfsreqcnt_v2", nfs_v2_names, (char *)nfs);
}

int sigar_nfs_server_v2_get(sigar_t *sigar,
                            sigar_nfs_server_v2_t *nfs)
{
    return sigar_nfs_get(sigar, "rfsproccnt_v2", nfs_v2_names, (char *)nfs);
}

static char *nfs_v3_names[] = {
    "null",
    "getattr",
    "setattr",
    "lookup",
    "access",
    "readlink",
    "read",
    "write",
    "create",
    "mkdir",
    "symlink",
    "mknod",
    "remove",
    "rmdir",
    "rename",
    "link",
    "readdir",
    "readdirplus",
    "fsstat",
    "fsinfo",
    "pathconf",
    "commit",
    NULL
};

int sigar_nfs_client_v3_get(sigar_t *sigar,
                            sigar_nfs_client_v3_t *nfs)
{
    return sigar_nfs_get(sigar, "rfsreqcnt_v3", nfs_v3_names, (char *)nfs);
}

int sigar_nfs_server_v3_get(sigar_t *sigar,
                            sigar_nfs_server_v3_t *nfs)
{
    return sigar_nfs_get(sigar, "rfsproccnt_v3", nfs_v3_names, (char *)nfs);
}

static int find_port(sigar_t *sigar, struct ps_prochandle *phandle,
                     sigar_pid_t pid, unsigned long port)
{
    DIR *dirp;
    struct dirent *ent;
    char pname[PATH_MAX];
    struct stat64 statb;
    int found=0;

    sprintf(pname, "/proc/%d/fd", (int)pid);

    if (!(dirp = opendir(pname))) {
        return 0;
    }

    while ((ent = readdir(dirp))) {
        int fd;

        if (!sigar_isdigit(ent->d_name[0])) {
            continue;
        }
        fd = atoi(ent->d_name);

        if (sigar->pfstat64(phandle, fd, &statb) == -1) {
            continue;
        }

        if ((statb.st_mode & S_IFMT) == S_IFSOCK) {
            struct sockaddr_in sin;
            struct sockaddr *sa = (struct sockaddr *)&sin;
            socklen_t len = sizeof(sin);
            int opt, optsz, rc;

            optsz = sizeof(opt);
            rc = sigar->pgetsockopt(phandle, fd, SOL_SOCKET, SO_TYPE, &opt, &optsz);
            if (rc != 0) {
                continue;
            }
            if (opt != SOCK_STREAM) {
                continue;
            }
            optsz = sizeof(opt);
            rc = sigar->pgetsockopt(phandle, fd, SOL_SOCKET, SO_ACCEPTCONN, &opt, &optsz);
            if (rc != 0) {
                continue;
            }
            if (opt != SO_ACCEPTCONN) {
                continue;
            }

            rc = sigar->pgetsockname(phandle, fd, sa, &len);
            if (rc != 0) {
                continue;
            }

            if ((sa->sa_family == AF_INET) ||
                (sa->sa_family == AF_INET6))
            {
                if (ntohs(sin.sin_port) == port) {
                    found = 1;
                    break;
                }
            }
        }
    }

    closedir(dirp);

    return found;
}

/* derived from /usr/bin/pfiles.c */
int sigar_proc_port_get(sigar_t *sigar, int protocol,
                        unsigned long port, sigar_pid_t *pid)
{
    sigar_proc_list_t pids;
    int i, status, found=0;

    if (sigar->solaris_version < 10) {
        return SIGAR_ENOTIMPL;
    }

    if ((status = sigar_init_libproc(sigar)) != SIGAR_OK) {
        return SIGAR_ENOTIMPL;
    }
    status = sigar_proc_list_get(sigar, &pids);
    if (status != SIGAR_OK) {
        return status;
    }

    for (i=0; i<pids.number; i++) {
        sigar_pid_t ps_id = pids.data[i];
        struct ps_prochandle *phandle;

        if (ps_id == sigar_pid_get(sigar)) {
            continue; /* XXX */
        }
        status = sigar_pgrab(sigar, ps_id, SIGAR_FUNC, &phandle);

        if (status != SIGAR_OK) {
            continue;
        }

        if (sigar->pcreate_agent(phandle) == 0) {
            found = find_port(sigar, phandle, ps_id, port);
            sigar->pdestroy_agent(phandle);
        }

        sigar->pfree(phandle);
        if (found) {
            *pid = ps_id;
            break;
        }
    }

    sigar_proc_list_destroy(sigar, &pids);

    return found ? SIGAR_OK : ENOENT;
}

int sigar_os_sys_info_get(sigar_t *sigar,
                          sigar_sys_info_t *sys_info)
{
    char *vendor_version;

    sysinfo(SI_ARCHITECTURE, sys_info->arch, sizeof(sys_info->arch));

    SIGAR_SSTRCPY(sys_info->name, "Solaris");
    SIGAR_SSTRCPY(sys_info->vendor, "Sun Microsystems");

    if (strEQ(sys_info->version, "5.6")) {
        vendor_version = "2.6";
    }
    else {
        if ((vendor_version = strchr(sys_info->version, '.'))) {
            ++vendor_version;
        }
        else {
            vendor_version = sys_info->version;
        }
    }

    SIGAR_SSTRCPY(sys_info->vendor_version, vendor_version);

    snprintf(sys_info->description,
             sizeof(sys_info->description),
             "%s %s",
             sys_info->name, sys_info->vendor_version);

    return SIGAR_OK;
}
