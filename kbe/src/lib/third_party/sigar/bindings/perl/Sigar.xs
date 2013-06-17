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

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "sigar.h"
#include "sigar_fileinfo.h"
#include "sigar_format.h"

typedef sigar_t * Sigar;
typedef sigar_net_address_t Sigar__NetAddress;

/* generated list */
typedef sigar_uptime_t * Sigar__Uptime;
typedef sigar_mem_t * Sigar__Mem;
typedef sigar_proc_cred_t * Sigar__ProcCred;
typedef sigar_proc_fd_t * Sigar__ProcFd;
typedef sigar_dir_stat_t * Sigar__DirStat;
typedef sigar_proc_cred_name_t * Sigar__ProcCredName;
typedef sigar_file_attrs_t * Sigar__FileAttrs;
typedef sigar_cpu_t * Sigar__Cpu;
typedef sigar_cpu_info_t * Sigar__CpuInfo;
typedef sigar_cpu_perc_t * Sigar__CpuPerc;
typedef sigar_net_interface_config_t * Sigar__NetInterfaceConfig;
typedef sigar_swap_t * Sigar__Swap;
typedef sigar_proc_state_t * Sigar__ProcState;
typedef sigar_net_connection_t * Sigar__NetConnection;
typedef sigar_proc_exe_t * Sigar__ProcExe;
typedef sigar_proc_time_t * Sigar__ProcTime;
typedef sigar_proc_cpu_t * Sigar__ProcCpu;
typedef sigar_proc_mem_t * Sigar__ProcMem;
typedef sigar_file_system_t * Sigar__FileSystem;
typedef sigar_file_system_usage_t * Sigar__FileSystemUsage;
typedef sigar_disk_usage_t * Sigar__DiskUsage;
typedef sigar_proc_stat_t * Sigar__ProcStat;
typedef sigar_net_route_t * Sigar__NetRoute;
typedef sigar_net_interface_stat_t * Sigar__NetInterfaceStat;
typedef sigar_who_t * Sigar__Who;
typedef sigar_thread_cpu_t * Sigar__ThreadCpu;
typedef sigar_resource_limit_t * Sigar__ResourceLimit;
typedef sigar_net_info_t * Sigar__NetInfo;
typedef sigar_dir_usage_t * Sigar__DirUsage;
typedef sigar_sys_info_t * Sigar__SysInfo; 
typedef sigar_net_stat_t * Sigar__NetStat; 
typedef sigar_tcp_t * Sigar__Tcp;
typedef sigar_nfs_client_v2_t * Sigar__NfsClientV2;
typedef sigar_nfs_server_v2_t * Sigar__NfsServerV2;
typedef sigar_nfs_client_v3_t * Sigar__NfsClientV3;
typedef sigar_nfs_server_v3_t * Sigar__NfsServerV3;

/* Perl < 5.6 */
#ifndef aTHX_
#define aTHX_
#endif

#define SIGAR_CROAK(sigar, msg) \
    Perl_croak(aTHX_ msg " %s", sigar_strerror(sigar, status))

static SV *convert_2svav(char *data, unsigned long number,
                         int size, const char *classname)
{
    AV *av = newAV();
    unsigned long i;

    for (i=0; i<number; i++, data += size) {
        SV *sv = newSV(0);
        void *ent = safemalloc(size);

        memcpy(ent, data, size);
        sv_setref_pv(sv, classname, ent);
        av_push(av, sv);
    }

    return newRV_noinc((SV*)av);
}

static SV *convert_2strav(char **data, unsigned long number)
{
    unsigned long i;
    AV *av = newAV();

    av_extend(av, number - 1);

    for (i=0; i<number; i++) {
        av_push(av, newSVpv(data[i], 0));
    }

    return newRV_noinc((SV*)av);
}

static int proc_env_getall(void *data,
                           const char *key, int klen,
                           char *val, int vlen)
{
    HV *hv = (HV*)data;
    hv_store(hv, key, klen, newSVpv(val, vlen), 0);
    return SIGAR_OK;
}

typedef struct {
    char *key;
    int klen;
    SV *val;
} env_getvalue_t;

static int proc_env_getvalue(void *data,
                             const char *key, int klen,
                             char *val, int vlen)
{
    env_getvalue_t *get = (env_getvalue_t *)data;

    if ((get->klen == klen) &&
        (strEQ(get->key, key)))
    {
        get->val = newSVpv(val, vlen);
        return !SIGAR_OK; /* foundit; stop iterating */
    }

    return SIGAR_OK;
}

#define XS_SIGAR_CONST_IV(name) \
    (void)newCONSTSUB(stash, #name, newSViv(SIGAR_##name))

#define XS_SIGAR_CONST_PV(name) \
    (void)newCONSTSUB(stash, #name, newSVpv(SIGAR_##name, 0))

static void boot_Sigar_constants(pTHX)
{
    HV *stash = gv_stashpv("Sigar", TRUE);
    XS_SIGAR_CONST_IV(IFF_UP);
    XS_SIGAR_CONST_IV(IFF_BROADCAST);
    XS_SIGAR_CONST_IV(IFF_DEBUG);
    XS_SIGAR_CONST_IV(IFF_LOOPBACK);
    XS_SIGAR_CONST_IV(IFF_POINTOPOINT);
    XS_SIGAR_CONST_IV(IFF_NOTRAILERS);
    XS_SIGAR_CONST_IV(IFF_RUNNING);
    XS_SIGAR_CONST_IV(IFF_NOARP);
    XS_SIGAR_CONST_IV(IFF_PROMISC);
    XS_SIGAR_CONST_IV(IFF_ALLMULTI);
    XS_SIGAR_CONST_IV(IFF_MULTICAST);

    XS_SIGAR_CONST_IV(NETCONN_CLIENT);
    XS_SIGAR_CONST_IV(NETCONN_SERVER);
    XS_SIGAR_CONST_IV(NETCONN_TCP);
    XS_SIGAR_CONST_IV(NETCONN_UDP);
    XS_SIGAR_CONST_IV(NETCONN_RAW);
    XS_SIGAR_CONST_IV(NETCONN_UNIX);

    XS_SIGAR_CONST_IV(TCP_ESTABLISHED);
    XS_SIGAR_CONST_IV(TCP_SYN_SENT);
    XS_SIGAR_CONST_IV(TCP_SYN_RECV);
    XS_SIGAR_CONST_IV(TCP_FIN_WAIT1);
    XS_SIGAR_CONST_IV(TCP_FIN_WAIT2);
    XS_SIGAR_CONST_IV(TCP_TIME_WAIT);
    XS_SIGAR_CONST_IV(TCP_CLOSE);
    XS_SIGAR_CONST_IV(TCP_CLOSE_WAIT);
    XS_SIGAR_CONST_IV(TCP_LAST_ACK);
    XS_SIGAR_CONST_IV(TCP_LISTEN);
    XS_SIGAR_CONST_IV(TCP_CLOSING);
    XS_SIGAR_CONST_IV(TCP_IDLE);
    XS_SIGAR_CONST_IV(TCP_BOUND);
    XS_SIGAR_CONST_IV(TCP_UNKNOWN);

    XS_SIGAR_CONST_PV(NULL_HWADDR);
}

MODULE = Sigar   PACKAGE = Sigar

PROTOTYPES: disable

INCLUDE: Sigar_generated.xs

BOOT:
    boot_Sigar_constants(aTHX);

MODULE = Sigar   PACKAGE = Sigar   PREFIX = sigar_

Sigar
new(classname)
    char *classname

    PREINIT:
    int status;

    CODE:
    if ((status = sigar_open(&RETVAL)) != SIGAR_OK) {
        classname = classname; /* -Wall */
        SIGAR_CROAK(RETVAL, "open");
    }

    OUTPUT:
    RETVAL

void
DESTROY(sigar)
    Sigar sigar

    CODE:
    (void)sigar_close(sigar);

char *
format_size(size)
    UV size

    PREINIT:
    char buffer[56];

    CODE:
    RETVAL = sigar_format_size(size, buffer);

    OUTPUT:
    RETVAL

char *
net_interface_flags_string(size)
    UV size

    PREINIT:
    char buffer[1024];

    CODE:
    RETVAL = sigar_net_interface_flags_to_string(size, buffer);

    OUTPUT:
    RETVAL

char *
sigar_fqdn(sigar)
    Sigar sigar

    PREINIT:
    char fqdn[SIGAR_FQDN_LEN];

    CODE:
    sigar_fqdn_get(sigar, fqdn, sizeof(fqdn));
    RETVAL = fqdn;

    OUTPUT:
    RETVAL

SV *
loadavg(sigar)
    Sigar sigar

    PREINIT:
    sigar_loadavg_t loadavg;
    int status;
    unsigned long i;
    AV *av;

    CODE:
    status = sigar_loadavg_get(sigar, &loadavg);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "loadavg");
    }

    av = newAV();
    av_extend(av, 2);

    for (i=0; i<3; i++) {
        av_push(av, newSVnv(loadavg.loadavg[i]));
    }

    RETVAL = newRV_noinc((SV*)av);

    OUTPUT:
    RETVAL

SV *
file_system_list(sigar)
    Sigar sigar

    PREINIT:
    sigar_file_system_list_t fslist;
    int status;

    CODE:
    status = sigar_file_system_list_get(sigar, &fslist);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "fslist");
    }

    RETVAL = convert_2svav((char *)&fslist.data[0],
                           fslist.number,
                           sizeof(*fslist.data),
                           "Sigar::FileSystem");

    sigar_file_system_list_destroy(sigar, &fslist);

    OUTPUT:
    RETVAL

SV *
cpu_info_list(sigar)
    Sigar sigar

    PREINIT:
    sigar_cpu_info_list_t cpu_infos;
    int status;

    CODE:
    status = sigar_cpu_info_list_get(sigar, &cpu_infos);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "cpu_infos");
    }

    RETVAL = convert_2svav((char *)&cpu_infos.data[0],
                           cpu_infos.number,
                           sizeof(*cpu_infos.data),
                           "Sigar::CpuInfo");

    sigar_cpu_info_list_destroy(sigar, &cpu_infos);

    OUTPUT:
    RETVAL

SV * 
cpu_list(sigar) 
    Sigar sigar 

    PREINIT: 
    sigar_cpu_list_t cpus; 
    int status; 

    CODE: 
    status = sigar_cpu_list_get(sigar, &cpus); 

    if (status != SIGAR_OK) { 
        SIGAR_CROAK(sigar, "cpu_list"); 
    } 

    RETVAL = convert_2svav((char *)&cpus.data[0], 
                           cpus.number, 
                           sizeof(*cpus.data), 
                           "Sigar::Cpu"); 

    sigar_cpu_list_destroy(sigar, &cpus); 

    OUTPUT: 
    RETVAL

SV *
proc_list(sigar)
    Sigar sigar

    PREINIT:
    sigar_proc_list_t proclist;
    int status;
    unsigned long i;
    AV *av;

    CODE:
    status = sigar_proc_list_get(sigar, &proclist);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "proc_list");
    }

    av = newAV();
    av_extend(av, proclist.number - 1);

    for (i=0; i<proclist.number; i++) {
        av_push(av, newSViv((IV)proclist.data[i]));
    }

    RETVAL = newRV_noinc((SV*)av);

    sigar_proc_list_destroy(sigar, &proclist);

    OUTPUT:
    RETVAL

SV *
proc_args(sigar, pid)
    Sigar sigar
    sigar_pid_t pid

    PREINIT:
    sigar_proc_args_t procargs;
    int status;

    CODE:
    status = sigar_proc_args_get(sigar, pid, &procargs);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "proc_args");
    }

    RETVAL = convert_2strav(procargs.data, procargs.number);

    sigar_proc_args_destroy(sigar, &procargs);

    OUTPUT:
    RETVAL

SV *
proc_env(sigar, pid, key=NULL)
    Sigar sigar
    sigar_pid_t pid
    SV *key;

    PREINIT:
    env_getvalue_t get;
    sigar_proc_env_t procenv;
    int status;
    HV *hv = Nullhv;

    CODE:
    if (key == NULL) {
        procenv.type = SIGAR_PROC_ENV_ALL;
        procenv.env_getter = proc_env_getall;
        procenv.data = hv = newHV();
    }
    else {
        STRLEN len = get.klen;
        procenv.type = SIGAR_PROC_ENV_KEY;
        procenv.env_getter = proc_env_getvalue;
        procenv.data = &get;
        get.val = &PL_sv_undef;
        get.key = SvPV(key, len);
        procenv.key = get.key;
        procenv.klen = get.klen;
    }

    status = sigar_proc_env_get(sigar, pid, &procenv);

    if (status != SIGAR_OK) {
        if (hv) {
            SvREFCNT_dec((SV*)hv);
        }
        SIGAR_CROAK(sigar, "proc_env");
    }

    if (key == NULL) {
        RETVAL = newRV_noinc((SV*)hv);
    }
    else {
        RETVAL = get.val;
    }

    OUTPUT:
    RETVAL

SV *
net_interface_list(sigar)
    Sigar sigar

    PREINIT:
    sigar_net_interface_list_t iflist;
    int status;

    CODE:
    status = sigar_net_interface_list_get(sigar, &iflist);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "net_interface_list");
    }

    RETVAL = convert_2strav(iflist.data, iflist.number);

    sigar_net_interface_list_destroy(sigar, &iflist);

    OUTPUT:
    RETVAL

SV *
net_connection_list(sigar, flags)
    Sigar sigar
    int flags

    PREINIT:
    sigar_net_connection_list_t conn_list;
    int status;

    CODE:
    status = sigar_net_connection_list_get(sigar, &conn_list, flags);

    if (status != SIGAR_OK) {
        SIGAR_CROAK(sigar, "net_connection_list");
    }

    RETVAL = convert_2svav((char *)&conn_list.data[0],
                           conn_list.number,
                           sizeof(*conn_list.data),
                           "Sigar::NetConnection");

    sigar_net_connection_list_destroy(sigar, &conn_list);

    OUTPUT:
    RETVAL
