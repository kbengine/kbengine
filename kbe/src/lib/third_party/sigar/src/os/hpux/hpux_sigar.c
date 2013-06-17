/*
 * Copyright (c) 2004-2009 Hyperic, Inc.
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

#include <sys/dk.h>
#ifndef __ia64__
#include <sys/lwp.h>
#endif
#include <sys/stat.h>
#include <errno.h>

#ifdef _PSTAT64
typedef int64_t pstat_int_t;
#else
typedef int32_t pstat_int_t;
#endif

int sigar_os_open(sigar_t **sigar)
{
    *sigar = malloc(sizeof(**sigar));

    /* does not change while system is running */
    pstat_getstatic(&(*sigar)->pstatic,
                    sizeof((*sigar)->pstatic),
                    1, 0);

    (*sigar)->ticks = sysconf(_SC_CLK_TCK);

    (*sigar)->last_pid = -1;

    (*sigar)->pinfo = NULL;

    (*sigar)->mib = -1;
    
    return SIGAR_OK;
    
}

int sigar_os_close(sigar_t *sigar)
{
    if (sigar->pinfo) {
        free(sigar->pinfo);
    }
    if (sigar->mib >= 0) {
        close_mib(sigar->mib);
    } 
    free(sigar);
    return SIGAR_OK;
}

char *sigar_os_error_string(sigar_t *sigar, int err)
{
    return NULL;
}

int sigar_mem_get(sigar_t *sigar, sigar_mem_t *mem)
{
    struct pst_dynamic stats;
    struct pst_vminfo vminfo;
    sigar_uint64_t pagesize = sigar->pstatic.page_size;
    sigar_uint64_t kern;

    mem->total = sigar->pstatic.physical_memory * pagesize;

    pstat_getdynamic(&stats, sizeof(stats), 1, 0);

    mem->free = stats.psd_free * pagesize;
    mem->used = mem->total - mem->free;

    pstat_getvminfo(&vminfo, sizeof(vminfo), 1, 0);

    /* "kernel dynamic memory" */
    kern = vminfo.psv_kern_dynmem * pagesize;
    mem->actual_free = mem->free + kern;
    mem->actual_used = mem->used - kern;

    sigar_mem_calc_ram(sigar, mem);

    return SIGAR_OK;
}

int sigar_swap_get(sigar_t *sigar, sigar_swap_t *swap)
{
    struct pst_swapinfo swapinfo;
    struct pst_vminfo vminfo;
    int i=0;

    swap->total = swap->free = 0;

    while (pstat_getswap(&swapinfo, sizeof(swapinfo), 1, i++) > 0) {
        swapinfo.pss_nfpgs *= 4;  /* nfpgs is in 512 byte blocks */

        if (swapinfo.pss_nblksenabled == 0) {
            swapinfo.pss_nblksenabled = swapinfo.pss_nfpgs;
        }

        swap->total += swapinfo.pss_nblksenabled;
        swap->free  += swapinfo.pss_nfpgs;
    }

    swap->used = swap->total - swap->free;

    pstat_getvminfo(&vminfo, sizeof(vminfo), 1, 0);

    swap->page_in = vminfo.psv_spgin;
    swap->page_out = vminfo.psv_spgout;

    return SIGAR_OK;
}

static void get_cpu_metrics(sigar_t *sigar,
                            sigar_cpu_t *cpu,
                            pstat_int_t *cpu_time)
{
    cpu->user = SIGAR_TICK2MSEC(cpu_time[CP_USER]);

    cpu->sys  = SIGAR_TICK2MSEC(cpu_time[CP_SYS] +
                                cpu_time[CP_SSYS]);

    cpu->nice = SIGAR_TICK2MSEC(cpu_time[CP_NICE]);

    cpu->idle = SIGAR_TICK2MSEC(cpu_time[CP_IDLE]);

    cpu->wait = SIGAR_TICK2MSEC(cpu_time[CP_SWAIT] +
                                cpu_time[CP_BLOCK]);
    
    cpu->irq = SIGAR_TICK2MSEC(cpu_time[CP_INTR]);
    cpu->soft_irq = 0; /*N/A*/
    cpu->stolen = 0; /*N/A*/

    cpu->total =
        cpu->user + cpu->sys + cpu->nice + cpu->idle + cpu->wait + cpu->irq;
}

int sigar_cpu_get(sigar_t *sigar, sigar_cpu_t *cpu)
{
    struct pst_dynamic stats;

    pstat_getdynamic(&stats, sizeof(stats), 1, 0);
    sigar->ncpu = stats.psd_proc_cnt;

    get_cpu_metrics(sigar, cpu, stats.psd_cpu_time);

    return SIGAR_OK;
}

int sigar_cpu_list_get(sigar_t *sigar, sigar_cpu_list_t *cpulist)
{
    int i;
    struct pst_dynamic stats;

    pstat_getdynamic(&stats, sizeof(stats), 1, 0);
    sigar->ncpu = stats.psd_proc_cnt;

    sigar_cpu_list_create(cpulist);

    for (i=0; i<sigar->ncpu; i++) {
        sigar_cpu_t *cpu;
        struct pst_processor proc;

        if (pstat_getprocessor(&proc, sizeof(proc), 1, i) < 0) {
            continue;
        }

        SIGAR_CPU_LIST_GROW(cpulist);

        cpu = &cpulist->data[cpulist->number++];

        get_cpu_metrics(sigar, cpu, proc.psp_cpu_time);
    }

    return SIGAR_OK;
}

int sigar_uptime_get(sigar_t *sigar,
                     sigar_uptime_t *uptime)
{
    uptime->uptime = time(NULL) - sigar->pstatic.boot_time;

    return SIGAR_OK;
}

int sigar_loadavg_get(sigar_t *sigar,
                      sigar_loadavg_t *loadavg)
{
    struct pst_dynamic stats;

    pstat_getdynamic(&stats, sizeof(stats), 1, 0);

    loadavg->loadavg[0] = stats.psd_avg_1_min;
    loadavg->loadavg[1] = stats.psd_avg_5_min;
    loadavg->loadavg[2] = stats.psd_avg_15_min;
    
    return SIGAR_OK;
}

#define PROC_ELTS 16

int sigar_os_proc_list_get(sigar_t *sigar,
                           sigar_proc_list_t *proclist)
{
    int num, idx=0;
    struct pst_status proctab[PROC_ELTS];

    while ((num = pstat_getproc(proctab, sizeof(proctab[0]),
                                PROC_ELTS, idx)) > 0)
    {
        int i;

        for (i=0; i<num; i++) {
            SIGAR_PROC_LIST_GROW(proclist);
            proclist->data[proclist->number++] =
                proctab[i].pst_pid;
        }

        idx = proctab[num-1].pst_idx + 1;
    }

    if (proclist->number == 0) {
        return errno;
    }

    return SIGAR_OK;
}

static int sigar_pstat_getproc(sigar_t *sigar, sigar_pid_t pid)
{
    int status, num;
    time_t timenow = time(NULL);

    if (sigar->pinfo == NULL) {
        sigar->pinfo = malloc(sizeof(*sigar->pinfo));
    }

    if (sigar->last_pid == pid) {
        if ((timenow - sigar->last_getprocs) < SIGAR_LAST_PROC_EXPIRE) {
            return SIGAR_OK;
        }
    }

    sigar->last_pid = pid;
    sigar->last_getprocs = timenow;

    if (pstat_getproc(sigar->pinfo,
                      sizeof(*sigar->pinfo),
                      0, pid) == -1)
    {
        return errno;
    }

    return SIGAR_OK;
}

int sigar_proc_mem_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_mem_t *procmem)
{
    int pagesize = sigar->pstatic.page_size;
    int status = sigar_pstat_getproc(sigar, pid);
    struct pst_status *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    procmem->size = 
        pinfo->pst_vtsize + /* text */
        pinfo->pst_vdsize + /* data */
        pinfo->pst_vssize + /* stack */
        pinfo->pst_vshmsize + /* shared memory */
        pinfo->pst_vmmsize + /* mem-mapped files */
        pinfo->pst_vusize + /* U-Area & K-Stack */
        pinfo->pst_viosize; /* I/O dev mapping */

    procmem->size *= pagesize;
        
    procmem->resident = pinfo->pst_rssize * pagesize;

    procmem->share = pinfo->pst_vshmsize * pagesize;

    procmem->minor_faults = pinfo->pst_minorfaults;
    procmem->major_faults = pinfo->pst_majorfaults;
    procmem->page_faults =
        procmem->minor_faults +
        procmem->major_faults;

    return SIGAR_OK;
}

int sigar_proc_cred_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_cred_t *proccred)
{
    int status = sigar_pstat_getproc(sigar, pid);
    struct pst_status *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    proccred->uid  = pinfo->pst_uid;
    proccred->gid  = pinfo->pst_gid;
    proccred->euid = pinfo->pst_euid;
    proccred->egid = pinfo->pst_egid;

    return SIGAR_OK;
}

int sigar_proc_time_get(sigar_t *sigar, sigar_pid_t pid,
                        sigar_proc_time_t *proctime)
{
    int status = sigar_pstat_getproc(sigar, pid);
    struct pst_status *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

    proctime->start_time = pinfo->pst_start;
    proctime->start_time *= SIGAR_MSEC;
    proctime->user = pinfo->pst_utime * SIGAR_MSEC;
    proctime->sys  = pinfo->pst_stime * SIGAR_MSEC;
    proctime->total = proctime->user + proctime->sys;

    return SIGAR_OK;
}

int sigar_proc_state_get(sigar_t *sigar, sigar_pid_t pid,
                         sigar_proc_state_t *procstate)
{
    int status = sigar_pstat_getproc(sigar, pid);
    struct pst_status *pinfo = sigar->pinfo;

    if (status != SIGAR_OK) {
        return status;
    }

        
    SIGAR_SSTRCPY(procstate->name, pinfo->pst_ucomm);
    procstate->ppid = pinfo->pst_ppid;
    procstate->tty  = makedev(pinfo->pst_term.psd_major,
                              pinfo->pst_term.psd_minor);
    procstate->priority = pinfo->pst_pri;
    procstate->nice     = pinfo->pst_nice;
    procstate->threads  = pinfo->pst_nlwps;
    procstate->processor = pinfo->pst_procnum;

    /* cast to prevent compiler warning: */
    /* Case label too big for the type of the switch expression */
    switch ((int32_t)pinfo->pst_stat) {
      case PS_SLEEP:
        procstate->state = 'S';
        break;
      case PS_RUN:
        procstate->state = 'R';
        break;
      case PS_STOP:
        procstate->state = 'T';
        break;
      case PS_ZOMBIE:
        procstate->state = 'Z';
        break;
      case PS_IDLE:
        procstate->state = 'D';
        break;
    }

    return SIGAR_OK;
}

int sigar_os_proc_args_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_args_t *procargs)
{
    char *args, *arg;
#ifdef PSTAT_GETCOMMANDLINE
    char buf[1024]; /* kernel limit */

# ifdef pstat_getcommandline /* 11i v2 + */
    if (pstat_getcommandline(buf, sizeof(buf), sizeof(buf[0]), pid) == -1) {
        return errno;
    }
# else
    union pstun pu;

    pu.pst_command = buf;
    if (pstat(PSTAT_GETCOMMANDLINE, pu, sizeof(buf), sizeof(buf[0]), pid) == -1) {
        return errno;
    }
# endif /* pstat_getcommandline */

    args = buf;
#else
    struct pst_status status;

    if (pstat_getproc(&status, sizeof(status), 0, pid) == -1) {
        return errno;
    }

    args = status.pst_cmd;
#endif

    while (*args && (arg = sigar_getword(&args, ' '))) {
        SIGAR_PROC_ARGS_GROW(procargs);
        procargs->data[procargs->number++] = arg;
    }
    
    return SIGAR_OK;
}

int sigar_proc_env_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_env_t *procenv)
{
    return SIGAR_ENOTIMPL;
}

int sigar_proc_fd_get(sigar_t *sigar, sigar_pid_t pid,
                      sigar_proc_fd_t *procfd)
{
    struct pst_status status;
    int idx=0, n;
    struct pst_fileinfo2 psf[16];

    procfd->total = 0;

    if (pstat_getproc(&status, sizeof(status), 0, pid) == -1) {
        return errno;
    }

    /* HPUX 11.31 removed the deprecated pstat_getfile call */
    while ((n = pstat_getfile2(psf, sizeof(psf[0]),
                               sizeof(psf)/sizeof(psf[0]),
                               idx, pid)) > 0)
    {
        procfd->total += n;
        idx = psf[n-1].psf_fd + 1;
    }

    if (n == -1) {
        return errno;
    }

    return SIGAR_OK;
}

int sigar_proc_exe_get(sigar_t *sigar, sigar_pid_t pid,
                       sigar_proc_exe_t *procexe)
{
#ifdef __pst_fid /* 11.11+ */
    int rc;
    struct pst_status status;

    if (pstat_getproc(&status, sizeof(status), 0, pid) == -1) {
        return errno;
    }

    rc = pstat_getpathname(procexe->cwd,
                           sizeof(procexe->cwd),
                           &status.pst_fid_cdir);
    if (rc == -1) {
        return errno;
    }

    rc = pstat_getpathname(procexe->name,
                           sizeof(procexe->name),
                           &status.pst_fid_text);
    if (rc == -1) {
        return errno;
    }

    rc = pstat_getpathname(procexe->root,
                           sizeof(procexe->root),
                           &status.pst_fid_rdir);
    if (rc == -1) {
        return errno;
    }

    return SIGAR_OK;
#else
    return SIGAR_ENOTIMPL; /* 11.00 */
#endif
}

int sigar_proc_modules_get(sigar_t *sigar, sigar_pid_t pid,
                           sigar_proc_modules_t *procmods)
{
    return SIGAR_ENOTIMPL;
}

#define TIME_NSEC(t) \
    (SIGAR_SEC2NANO((t).tv_sec) + (sigar_uint64_t)(t).tv_nsec)

int sigar_thread_cpu_get(sigar_t *sigar,
                         sigar_uint64_t id,
                         sigar_thread_cpu_t *cpu)
{
#ifdef __ia64__
    /* XXX seems _lwp funcs were for solaris compat and dont exist
     * on itanium.  hp docs claim that have equiv functions,
     * but wtf is it for _lwp_info?
     */
    return SIGAR_ENOTIMPL;
#else
    struct lwpinfo info;

    if (id != 0) {
        return SIGAR_ENOTIMPL;
    }

    _lwp_info(&info);

    cpu->user  = TIME_NSEC(info.lwp_utime);
    cpu->sys   = TIME_NSEC(info.lwp_stime);
    cpu->total = TIME_NSEC(info.lwp_utime) + TIME_NSEC(info.lwp_stime);

    return SIGAR_OK;
#endif
}

#include <mntent.h>

int sigar_os_fs_type_get(sigar_file_system_t *fsp)
{
    char *type = fsp->sys_type_name;

    switch (*type) {
      case 'h':
        if (strEQ(type, "hfs")) {
            fsp->type = SIGAR_FSTYPE_LOCAL_DISK;
        }
        break;
      case 'c':
        if (strEQ(type, "cdfs")) {
            fsp->type = SIGAR_FSTYPE_CDROM;
        }
        break;
    }

    return fsp->type;
}

int sigar_file_system_list_get(sigar_t *sigar,
                               sigar_file_system_list_t *fslist)
{
    struct mntent *ent;

    FILE *fp;
    sigar_file_system_t *fsp;

    if (!(fp = setmntent(MNT_MNTTAB, "r"))) {
        return errno;
    }

    sigar_file_system_list_create(fslist);

    while ((ent = getmntent(fp))) {
        if ((*(ent->mnt_type) == 's') &&
            strEQ(ent->mnt_type, "swap"))
        {
            /*
             * in this case, devname == "...", for
             * which statfs chokes on.  so skip it.
             * also notice hpux df command has no swap info.
             */
            continue;
        }
        
        SIGAR_FILE_SYSTEM_LIST_GROW(fslist);

        fsp = &fslist->data[fslist->number++];

        SIGAR_SSTRCPY(fsp->dir_name, ent->mnt_dir);
        SIGAR_SSTRCPY(fsp->dev_name, ent->mnt_fsname);
        SIGAR_SSTRCPY(fsp->sys_type_name, ent->mnt_type);
        SIGAR_SSTRCPY(fsp->options, ent->mnt_opts);
        sigar_fs_type_init(fsp);
    }

    endmntent(fp);

    return SIGAR_OK;
}

static int create_fsdev_cache(sigar_t *sigar)
{
    sigar_file_system_list_t fslist;
    int i;
    int status =
        sigar_file_system_list_get(sigar, &fslist);

    if (status != SIGAR_OK) {
        return status;
    }

    sigar->fsdev = sigar_cache_new(15);

    for (i=0; i<fslist.number; i++) {
        sigar_file_system_t *fsp = &fslist.data[i];

        if (fsp->type == SIGAR_FSTYPE_LOCAL_DISK) {
            sigar_cache_entry_t *ent;
            struct stat sb;

            if (stat(fsp->dir_name, &sb) < 0) {
                continue;
            }

            ent = sigar_cache_get(sigar->fsdev, SIGAR_FSDEV_ID(sb));
            ent->value = strdup(fsp->dev_name);
        }
    }

    return SIGAR_OK;
}

int sigar_disk_usage_get(sigar_t *sigar, const char *name,
                         sigar_disk_usage_t *usage)
{
    return SIGAR_ENOTIMPL;
}

int sigar_file_system_usage_get(sigar_t *sigar,
                                const char *dirname,
                                sigar_file_system_usage_t *fsusage)
{
    struct stat sb;
    int status = sigar_statvfs(sigar, dirname, fsusage);

    if (status != SIGAR_OK) {
        return status;
    }

    fsusage->use_percent = sigar_file_system_usage_calc_used(sigar, fsusage);

    SIGAR_DISK_STATS_INIT(&fsusage->disk);

    if (!sigar->fsdev) {
        if (create_fsdev_cache(sigar) != SIGAR_OK) {
            return SIGAR_OK;
        }
    }

    if (stat(dirname, &sb) == 0) {
        sigar_cache_entry_t *ent;
        struct pst_lvinfo lv;
        struct stat devsb;
        char *devname;
        int retval;

        ent = sigar_cache_get(sigar->fsdev, SIGAR_FSDEV_ID(sb));
        if (ent->value == NULL) {
            return SIGAR_OK;
        }

        if (stat((char *)ent->value, &devsb) < 0) {
            return SIGAR_OK;
        }

        retval = pstat_getlv(&lv, sizeof(lv), 0, (int)devsb.st_rdev);

        if (retval == 1) {
            fsusage->disk.reads  = lv.psl_rxfer;
            fsusage->disk.writes = lv.psl_wxfer;
            fsusage->disk.read_bytes  = lv.psl_rcount;
            fsusage->disk.write_bytes = lv.psl_wcount;
            fsusage->disk.queue       = SIGAR_FIELD_NOTIMPL;
        }
    }

    return SIGAR_OK;
}

int sigar_cpu_info_list_get(sigar_t *sigar,
                            sigar_cpu_info_list_t *cpu_infos)
{
    int i;
    struct pst_dynamic stats;

    pstat_getdynamic(&stats, sizeof(stats), 1, 0);
    sigar->ncpu = stats.psd_proc_cnt;

    sigar_cpu_info_list_create(cpu_infos);

    for (i=0; i<sigar->ncpu; i++) {
        sigar_cpu_info_t *info;
        struct pst_processor proc;

        if (pstat_getprocessor(&proc, sizeof(proc), 1, i) < 0) {
            perror("pstat_getprocessor");
            continue;
        }

        SIGAR_CPU_INFO_LIST_GROW(cpu_infos);

        info = &cpu_infos->data[cpu_infos->number++];

        info->total_cores = sigar->ncpu;
        info->cores_per_socket = 1; /*XXX*/
        info->total_sockets = sigar->ncpu; /*XXX*/

#ifdef __ia64__
        SIGAR_SSTRCPY(info->vendor, "Intel"); /*XXX*/
        SIGAR_SSTRCPY(info->model, "Itanium"); /*XXX*/
#else
        SIGAR_SSTRCPY(info->vendor, "HP"); /*XXX*/
        SIGAR_SSTRCPY(info->model, "PA RISC"); /*XXX*/
#endif
#ifdef PSP_MAX_CACHE_LEVELS /* 11.31+; see SIGAR-196 */
        info->mhz = proc.psp_cpu_frequency / 1000000;
#else
        info->mhz = sigar->ticks * proc.psp_iticksperclktick / 1000000;
#endif
        info->cache_size = SIGAR_FIELD_NOTIMPL; /*XXX*/
    }

    return SIGAR_OK;
}

static int sigar_get_mib_info(sigar_t *sigar,
                              struct nmparms *parms)
{
    if (sigar->mib < 0) {
        if ((sigar->mib = open_mib("/dev/ip", O_RDONLY, 0, 0)) < 0) {
            return errno;
        }
    }
    return get_mib_info(sigar->mib, parms);
}

/* wrapper around get_physical_stat() */
static int sigar_get_physical_stat(sigar_t *sigar, int *count)
{
    int status;
    unsigned int len;
    struct nmparms parms;

    len = sizeof(*count);
    parms.objid = ID_ifNumber;
    parms.buffer = count;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        return status;
    }

    len = sizeof(nmapi_phystat) * *count;

    if (sigar->ifconf_len < len) {
        sigar->ifconf_buf = realloc(sigar->ifconf_buf, len);
        sigar->ifconf_len = len;
    }

    if (get_physical_stat(sigar->ifconf_buf, &len) < 0) {
        return errno;
    }
    else {
        return SIGAR_OK;
    }
}

#define SIGAR_IF_NAMESIZE 16
/* hpux if_indextoname() does not work as advertised in 11.11 */
static int sigar_if_indextoname(sigar_t *sigar,
                                char *name,
                                int index)
{
    int i, status, count;
    nmapi_phystat *stat;

    if ((status = sigar_get_physical_stat(sigar, &count) != SIGAR_OK)) {
        return status;
    }

    for (i=0, stat = (nmapi_phystat *)sigar->ifconf_buf;
         i<count;
         i++, stat++)
    {
        if (stat->if_entry.ifIndex == index) {
            strncpy(name, stat->nm_device, SIGAR_IF_NAMESIZE);
            return SIGAR_OK;
        }
    }

    return ENXIO;
}

int sigar_net_route_list_get(sigar_t *sigar,
                             sigar_net_route_list_t *routelist)
{
    int status, count, i;
    unsigned int len;
    struct nmparms parms;
    mib_ipRouteEnt *routes;
    sigar_net_route_t *route;

    len = sizeof(count);
    parms.objid = ID_ipRouteNumEnt;
    parms.buffer = &count;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        return status;
    }

    len = count * sizeof(*routes);
    routes = malloc(len);

    parms.objid = ID_ipRouteTable;
    parms.buffer = routes;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        free(routes);
        return status;
    }

    routelist->size = routelist->number = 0;

    sigar_net_route_list_create(routelist);

    for (i=0; i<count; i++) {
        mib_ipRouteEnt *ent = &routes[i];

        SIGAR_NET_ROUTE_LIST_GROW(routelist);

        route = &routelist->data[routelist->number++];
        SIGAR_ZERO(route); /* XXX: other fields */
        
        sigar_net_address_set(route->destination,
                              ent->Dest);

        sigar_net_address_set(route->mask,
                              ent->Mask);

        sigar_net_address_set(route->gateway,
                              ent->NextHop);

        sigar_if_indextoname(sigar, route->ifname, ent->IfIndex);

        route->flags = SIGAR_RTF_UP;
        if ((ent->Dest == 0) &&
            (ent->Mask == 0))
        {
            route->flags |= SIGAR_RTF_GATEWAY;
        }
    }

    free(routes);
    
    return SIGAR_OK;
}

static int get_mib_ifstat(sigar_t *sigar,
                          const char *name,
                          mib_ifEntry *mib)
{
    int i, status, count;
    nmapi_phystat *stat;

    if ((status = sigar_get_physical_stat(sigar, &count) != SIGAR_OK)) {
        return status;
    }

    for (i=0, stat = (nmapi_phystat *)sigar->ifconf_buf;
         i<count;
         i++, stat++)
    {
        if (strEQ(stat->nm_device, name)) {
            memcpy(mib, &stat->if_entry, sizeof(*mib));
            return SIGAR_OK;
        }
    }

    return ENXIO;
}

int sigar_net_interface_stat_get(sigar_t *sigar, const char *name,
                                 sigar_net_interface_stat_t *ifstat)
{
    int status;
    mib_ifEntry mib;

    status = get_mib_ifstat(sigar, name, &mib);

    if (status != SIGAR_OK) {
        return status;
    }

    ifstat->rx_bytes    = mib.ifInOctets;
    ifstat->rx_packets  = mib.ifInUcastPkts + mib.ifInNUcastPkts;
    ifstat->rx_errors   = mib.ifInErrors;
    ifstat->rx_dropped  = mib.ifInDiscards;
    ifstat->rx_overruns = SIGAR_FIELD_NOTIMPL;
    ifstat->rx_frame    = SIGAR_FIELD_NOTIMPL;

    ifstat->tx_bytes      = mib.ifOutOctets;
    ifstat->tx_packets    = mib.ifOutUcastPkts + mib.ifOutNUcastPkts;
    ifstat->tx_errors     = mib.ifOutErrors;
    ifstat->tx_dropped    = mib.ifOutDiscards;
    ifstat->tx_overruns   = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_collisions = SIGAR_FIELD_NOTIMPL;
    ifstat->tx_carrier    = SIGAR_FIELD_NOTIMPL;

    ifstat->speed         = mib.ifSpeed;

    return SIGAR_OK;
}

static int net_conn_get_udp_listen(sigar_net_connection_walker_t *walker)
{
    sigar_t *sigar = walker->sigar;
    int flags = walker->flags;
    int status, count, i;
    unsigned int len;
    mib_udpLsnEnt *entries;
    struct nmparms parms;

    len = sizeof(count);
    parms.objid = ID_udpLsnNumEnt;
    parms.buffer = &count;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        return status;
    }

    if (count <= 0) {
        return ENOENT;
    }

    len =  count * sizeof(*entries);
    entries = malloc(len);
    parms.objid = ID_udpLsnTable;
    parms.buffer = entries;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        free(entries);
        return status;
    }

    for (i=0; i<count; i++) {
        sigar_net_connection_t conn;
        mib_udpLsnEnt *entry = &entries[i];

        SIGAR_ZERO(&conn);

        conn.type = SIGAR_NETCONN_UDP;

        conn.local_port  = (unsigned short)entry->LocalPort;
        conn.remote_port = 0;

        sigar_net_address_set(conn.local_address,
                              entry->LocalAddress);

        sigar_net_address_set(conn.remote_address, 0);

        conn.send_queue = conn.receive_queue = SIGAR_FIELD_NOTIMPL;

        if (walker->add_connection(walker, &conn) != SIGAR_OK) {
            break;
        }
    }

    free(entries);
    return SIGAR_OK;
}

static int net_conn_get_udp(sigar_net_connection_walker_t *walker)
{
    int status = SIGAR_OK;

    if (walker->flags & SIGAR_NETCONN_SERVER) {
        status = net_conn_get_udp_listen(walker);
    }

    return status;
}

#define IS_TCP_SERVER(state, flags) \
    ((flags & SIGAR_NETCONN_SERVER) && (state == TCLISTEN))

#define IS_TCP_CLIENT(state, flags) \
    ((flags & SIGAR_NETCONN_CLIENT) && (state != TCLISTEN))

static int net_conn_get_tcp(sigar_net_connection_walker_t *walker)
{
    sigar_t *sigar = walker->sigar;
    int flags = walker->flags;
    int status, count, i;
    unsigned int len;
    mib_tcpConnEnt *entries;
    struct nmparms parms;

    len = sizeof(count);
    parms.objid = ID_tcpConnNumEnt;
    parms.buffer = &count;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        return status;
    }

    if (count <= 0) {
        return ENOENT;
    }

    len =  count * sizeof(*entries);
    entries = malloc(len);
    parms.objid = ID_tcpConnTable;
    parms.buffer = entries;
    parms.len = &len;

    if ((status = sigar_get_mib_info(sigar, &parms)) != SIGAR_OK) {
        free(entries);
        return status;
    }

    for (i=0; i<count; i++) {
        sigar_net_connection_t conn;
        mib_tcpConnEnt *entry = &entries[i];
        int state = entry->State;

        if (!(IS_TCP_SERVER(state, flags) ||
              IS_TCP_CLIENT(state, flags)))
        {
            continue;
        }

        SIGAR_ZERO(&conn);

        switch (state) {
          case TCCLOSED:
            conn.state = SIGAR_TCP_CLOSE;
            break;
          case TCLISTEN:
            conn.state = SIGAR_TCP_LISTEN;
            break;
          case TCSYNSENT:
            conn.state = SIGAR_TCP_SYN_SENT;
            break;
          case TCSYNRECEIVE:
            conn.state = SIGAR_TCP_SYN_RECV;
            break;
          case TCESTABLISED:
            conn.state = SIGAR_TCP_ESTABLISHED;
            break;
          case TCFINWAIT1:
            conn.state = SIGAR_TCP_FIN_WAIT1;
            break;
          case TCFINWAIT2:
            conn.state = SIGAR_TCP_FIN_WAIT2;
            break;
          case TCCLOSEWAIT:
            conn.state = SIGAR_TCP_CLOSE_WAIT;
            break;
          case TCCLOSING:
            conn.state = SIGAR_TCP_CLOSING;
            break;
          case TCLASTACK:
            conn.state = SIGAR_TCP_LAST_ACK;
            break;
          case TCTIMEWAIT:
            conn.state = SIGAR_TCP_TIME_WAIT;
            break;
          case TCDELETETCB:
          default:
            conn.state = SIGAR_TCP_UNKNOWN;
            break;
        }

        conn.local_port  = (unsigned short)entry->LocalPort;
        conn.remote_port = (unsigned short)entry->RemPort;
        conn.type = SIGAR_NETCONN_TCP;

        sigar_net_address_set(conn.local_address, entry->LocalAddress);
        sigar_net_address_set(conn.remote_address, entry->RemAddress);

        conn.send_queue = conn.receive_queue = SIGAR_FIELD_NOTIMPL;

        if (walker->add_connection(walker, &conn) != SIGAR_OK) {
            break;
        }
    }

    free(entries);

    return SIGAR_OK;
}

int sigar_net_connection_walk(sigar_net_connection_walker_t *walker)
{
    int status;

    if (walker->flags & SIGAR_NETCONN_TCP) {
        status = net_conn_get_tcp(walker);

        if (status != SIGAR_OK) {
            return status;
        }
    }

    if (walker->flags & SIGAR_NETCONN_UDP) {
        status = net_conn_get_udp(walker);

        if (status != SIGAR_OK) {
            return status;
        }
    }

    return SIGAR_OK;
}

#define tcpsoff(x) sigar_offsetof(sigar_tcp_t, x)

static struct {
    unsigned int id;
    size_t offset;
} tcps_lu[] = {
#if 0
    { ID_tcpRtoAlgorithm, tcpsoff(xxx) },
    { ID_tcpRtoMin, tcpsoff(xxx) },
    { ID_tcpRtoMax, tcpsoff(xxx) },
    { ID_tcpMaxConn, tcpsoff(max_conn) },
#endif
    { ID_tcpActiveOpens, tcpsoff(active_opens) },
    { ID_tcpPassiveOpens, tcpsoff(passive_opens) },
    { ID_tcpAttemptFails, tcpsoff(attempt_fails) },
    { ID_tcpEstabResets, tcpsoff(estab_resets) },
    { ID_tcpCurrEstab, tcpsoff(curr_estab) },
    { ID_tcpInSegs, tcpsoff(in_segs) },
    { ID_tcpOutSegs, tcpsoff(out_segs) },
    { ID_tcpRetransSegs, tcpsoff(retrans_segs) },
    { ID_tcpInErrs, tcpsoff(in_errs) },
    { ID_tcpOutRsts, tcpsoff(out_rsts) }
};

SIGAR_DECLARE(int)
sigar_tcp_get(sigar_t *sigar,
              sigar_tcp_t *tcp)
{
    int i;

    for (i=0; i<sizeof(tcps_lu)/sizeof(tcps_lu[0]); i++) {
        struct nmparms parms;
        int val;
        unsigned int len = sizeof(val);
        parms.objid = tcps_lu[i].id;
        parms.buffer = &val;
        parms.len = &len;        

        if (sigar_get_mib_info(sigar, &parms) != SIGAR_OK) {
            val = -1;
        }

        *(sigar_uint64_t *)((char *)tcp + tcps_lu[i].offset) = val;
    }

    return SIGAR_OK;
}

int sigar_nfs_client_v2_get(sigar_t *sigar,
                            sigar_nfs_client_v2_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

int sigar_nfs_server_v2_get(sigar_t *sigar,
                            sigar_nfs_server_v2_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

int sigar_nfs_client_v3_get(sigar_t *sigar,
                            sigar_nfs_client_v3_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

int sigar_nfs_server_v3_get(sigar_t *sigar,
                            sigar_nfs_server_v3_t *nfs)
{
    return SIGAR_ENOTIMPL;
}

int sigar_proc_port_get(sigar_t *sigar, int protocol,
                        unsigned long port, sigar_pid_t *pid)
{
    return SIGAR_ENOTIMPL;
}


int sigar_os_sys_info_get(sigar_t *sigar,
                          sigar_sys_info_t *sysinfo)
{
    char *vendor_version, *arch;
    long cpu = sysconf(_SC_CPU_VERSION);

    switch (cpu) {
        case CPU_PA_RISC1_0:
            arch = "PA_RISC1.0";
            break;
        case CPU_PA_RISC1_1:
            arch = "PA_RISC1.1";
            break;
        case CPU_PA_RISC2_0:
            arch = "PA_RISC2.0";
            break;
#ifdef CPU_IA64_ARCHREV_0            
        case CPU_IA64_ARCHREV_0:
            arch = "ia64";
            break;
#endif
        default:
            arch = "unknown";
            break;
    }

    SIGAR_SSTRCPY(sysinfo->arch, arch);

    SIGAR_SSTRCPY(sysinfo->name, "HPUX");
    SIGAR_SSTRCPY(sysinfo->vendor, "Hewlett-Packard");

    if (strstr(sysinfo->version, ".11.")) {
        vendor_version = "11";
    }
    else {
        vendor_version = sysinfo->version;
    }

    SIGAR_SSTRCPY(sysinfo->vendor_version, vendor_version);

    snprintf(sysinfo->description,
             sizeof(sysinfo->description),
             "%s %s",
             sysinfo->vendor_name, sysinfo->vendor_version);

    return SIGAR_OK;
}
