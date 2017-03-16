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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#include "sigar.h"
#include "sigar_private.h"
#include "sigar_util.h"
#include "sigar_os.h"

#ifndef WIN32

#include <dirent.h>
#include <sys/stat.h>

char *sigar_uitoa(char *buf, unsigned int n, int *len)
{
    char *start = buf + UITOA_BUFFER_SIZE - 1;

    *start = 0;

    do {
	*--start = '0' + (n % 10);
        ++*len;
	n /= 10;
    } while (n);

    return start;
}

char *sigar_skip_line(char *buffer, int buflen)
{
    char *ptr = buflen ?
        (char *)memchr(buffer, '\n', buflen) : /* bleh */
        strchr(buffer, '\n');
    return ++ptr;
}

char *sigar_skip_token(char *p)
{
    while (sigar_isspace(*p)) p++;
    while (*p && !sigar_isspace(*p)) p++;
    return p;
}

char *sigar_skip_multiple_token(char *p, int count)
{
    int i;
    
    for (i = 0; i < count; i++) {
        p = sigar_skip_token(p);
    }

    return p;
}

char *sigar_getword(char **line, char stop)
{
    char *pos = *line;
    int len;
    char *res;

    while ((*pos != stop) && *pos) {
        ++pos;
    }

    len = pos - *line;
    res = malloc(len + 1);
    memcpy(res, *line, len);
    res[len] = 0;

    if (stop) {
        while (*pos == stop) {
            ++pos;
        }
    }

    *line = pos;

    return res;
}

/* avoiding sprintf */

char *sigar_proc_filename(char *buffer, int buflen,
                          sigar_pid_t bigpid,
                          const char *fname, int fname_len)
{
    int len = 0;
    char *ptr = buffer;
    unsigned int pid = (unsigned int)bigpid; /* XXX -- This isn't correct */
    char pid_buf[UITOA_BUFFER_SIZE];
    char *pid_str = sigar_uitoa(pid_buf, pid, &len);

    assert((unsigned int)buflen >=
           (SSTRLEN(PROCP_FS_ROOT) + UITOA_BUFFER_SIZE + fname_len + 1));

    memcpy(ptr, PROCP_FS_ROOT, SSTRLEN(PROCP_FS_ROOT));
    ptr += SSTRLEN(PROCP_FS_ROOT);

    memcpy(ptr, pid_str, len);
    ptr += len;

    memcpy(ptr, fname, fname_len);
    ptr += fname_len;
    *ptr = '\0';

    return buffer;
}

int sigar_proc_file2str(char *buffer, int buflen,
                        sigar_pid_t pid,
                        const char *fname,
                        int fname_len)
{
    int retval;

    buffer = sigar_proc_filename(buffer, buflen, pid,
                                 fname, fname_len);

    retval = sigar_file2str(buffer, buffer, buflen);

    if (retval != SIGAR_OK) {
        switch (retval) {
          case ENOENT:
            retval = ESRCH; /* no such process */
          default:
            break;
        }
    }

    return retval;
}

int sigar_proc_list_procfs_get(sigar_t *sigar,
                               sigar_proc_list_t *proclist)
{
    DIR *dirp = opendir("/proc");
    struct dirent *ent;
#ifdef HAVE_READDIR_R
    struct dirent dbuf;
#endif

    if (!dirp) {
        return errno;
    }

#ifdef HAVE_READDIR_R
    while (readdir_r(dirp, &dbuf, &ent) == 0) {
        if (ent == NULL) {
            break;
        }
#else
    while ((ent = readdir(dirp))) {
#endif
        if (!sigar_isdigit(*ent->d_name)) {
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

int sigar_proc_fd_count(sigar_t *sigar, sigar_pid_t pid,
                        sigar_uint64_t *total)
{
    DIR *dirp;
    struct dirent *ent;
#ifdef HAVE_READDIR_R
    struct dirent dbuf;
#endif
    char name[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(name, pid, "/fd");

    *total = 0;

    if (!(dirp = opendir(name))) {
        return errno;
    }

#ifdef HAVE_READDIR_R
    while (readdir_r(dirp, &dbuf, &ent) == 0) {
        if (ent == NULL) {
            break;
        }
#else
    while ((ent = readdir(dirp))) {
#endif
        if (!sigar_isdigit(*ent->d_name)) {
            continue;
        }

        (*total)++;
    }

    closedir(dirp);

    return SIGAR_OK;
}

int sigar_procfs_args_get(sigar_t *sigar, sigar_pid_t pid,
                          sigar_proc_args_t *procargs)
{
    char buffer[9086], *buf=NULL, *ptr;
    int fd, len, total=0;

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/cmdline");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        if (errno == ENOENT) {
            return ESRCH;
        }
        return errno;
    }

    buffer[0] = '\0';

    /* XXX: possible to get rid of some mallocs here.
     * but, unlikely this will be called often so it
     * might not even matter much.
     */
    while ((len = read(fd, buffer, sizeof(buffer)-1)) > 0) {
        if (len == 0) {
            break;
        }
        buf = realloc(buf, total+len+1);
        memcpy(buf+total, buffer, len);
        total += len;
    }

    close(fd);

    /* e.g. /proc/2/cmdline */
    if (total == 0) {
        procargs->number = 0;
        return SIGAR_OK;
    }

    buf[total] = '\0';
    ptr = buf;

    while (total > 0) {
        int alen = strlen(ptr)+1;
        char *arg = malloc(alen);

        SIGAR_PROC_ARGS_GROW(procargs);
        memcpy(arg, ptr, alen);

        procargs->data[procargs->number++] = arg;

        total -= alen;
        if (total > 0) {
            ptr += alen;
        }
    }

    free(buf);

    return SIGAR_OK;
}

#endif /* WIN32 */

/* from httpd/server/util.c */
char *sigar_strcasestr(const char *s1, const char *s2)
{
    char *p1, *p2;
    if (*s2 == '\0') {
        /* an empty s2 */
        return((char *)s1);
    }
    while(1) {
        for ( ; (*s1 != '\0') && (sigar_tolower(*s1) != sigar_tolower(*s2)); s1++);
        if (*s1 == '\0') {
            return(NULL);
        }
        /* found first character of s2, see if the rest matches */
        p1 = (char *)s1;
        p2 = (char *)s2;
        for (++p1, ++p2; sigar_tolower(*p1) == sigar_tolower(*p2); ++p1, ++p2) {
            if (*p1 == '\0') {
                /* both strings ended together */
                return((char *)s1);
            }
        }
        if (*p2 == '\0') {
            /* second string ended, a match */
            break;
        }
        /* didn't find a match here, try starting at next character in s1 */
        s1++;
    }
    return((char *)s1);
}

int sigar_mem_calc_ram(sigar_t *sigar, sigar_mem_t *mem)
{
    sigar_int64_t total = mem->total / 1024, diff;
    sigar_uint64_t lram = (mem->total / (1024 * 1024));
    int ram = (int)lram; /* must cast after division */
    int remainder = ram % 8;

    if (remainder > 0) {
        ram += (8 - remainder);
    }

    mem->ram = ram;

    diff = total - (mem->actual_free / 1024);
    mem->used_percent =
        (double)(diff * 100) / total;

    diff = total - (mem->actual_used / 1024);
    mem->free_percent =
        (double)(diff * 100) / total;

    return ram;
}

#ifndef WIN32

sigar_iodev_t *sigar_iodev_get(sigar_t *sigar,
                               const char *dirname)
{
    sigar_cache_entry_t *entry;
    struct stat sb;
    sigar_uint64_t id;
    sigar_file_system_list_t fslist;
    int i, status, is_dev=0;
    int debug = SIGAR_LOG_IS_DEBUG(sigar);
    char dev_name[SIGAR_FS_NAME_LEN];

    if (!sigar->fsdev) {
        sigar->fsdev = sigar_cache_new(15);
    }

    if (*dirname != '/') {
        snprintf(dev_name, sizeof(dev_name),
                 SIGAR_DEV_PREFIX "%s", dirname);
        dirname = dev_name;
        is_dev = 1;
    }
    else if (SIGAR_NAME_IS_DEV(dirname)) {
        is_dev = 1;
    }

    if (stat(dirname, &sb) < 0) {
        if (debug) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[iodev] stat(%s) failed",
                             dirname);
        }
        return NULL;
    }

    id = SIGAR_FSDEV_ID(sb);

    entry = sigar_cache_get(sigar->fsdev, id);

    if (entry->value != NULL) {
        return (sigar_iodev_t *)entry->value;
    }

    if (is_dev) {
        sigar_iodev_t *iodev;
        entry->value = iodev = malloc(sizeof(*iodev));
        SIGAR_ZERO(iodev);
        SIGAR_SSTRCPY(iodev->name, dirname);
        if (debug) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[iodev] %s is_dev=true", dirname);
        }
        return iodev;
    }

    status = sigar_file_system_list_get(sigar, &fslist);

    if (status != SIGAR_OK) {
        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "[iodev] file_system_list failed: %s",
                         sigar_strerror(sigar, status));
        return NULL;
    }

    for (i=0; i<fslist.number; i++) {
        sigar_file_system_t *fsp = &fslist.data[i];

        if (fsp->type == SIGAR_FSTYPE_LOCAL_DISK) {
            int retval = stat(fsp->dir_name, &sb);
            sigar_cache_entry_t *ent;

            if (retval < 0) {
                if (debug) {
                    sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                     "[iodev] inode stat(%s) failed",
                                     fsp->dir_name);
                }
                continue; /* cant cache w/o inode */
            }

            ent = sigar_cache_get(sigar->fsdev, SIGAR_FSDEV_ID(sb));
            if (ent->value) {
                continue; /* already cached */
            }

            if (SIGAR_NAME_IS_DEV(fsp->dev_name)) {
                sigar_iodev_t *iodev;
                ent->value = iodev = malloc(sizeof(*iodev));
                SIGAR_ZERO(iodev);
                iodev->is_partition = 1;
                SIGAR_SSTRCPY(iodev->name, fsp->dev_name);

                if (debug) {
                    sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                                     "[iodev] map %s -> %s",
                                     fsp->dir_name, iodev->name);
                }
            }
        }
    }

    sigar_file_system_list_destroy(sigar, &fslist);

    if (entry->value &&
        (((sigar_iodev_t *)entry->value)->name[0] != '\0'))
    {
        return (sigar_iodev_t *)entry->value;
    }
    else {
        return NULL;
    }
}
#endif

double sigar_file_system_usage_calc_used(sigar_t *sigar,
                                         sigar_file_system_usage_t *fsusage)
{
    /* 
     * win32 will not convert __uint64 to double.
     * convert to KB then do unsigned long -> double.
     */
    sigar_uint64_t b_used = (fsusage->total - fsusage->free) / 1024;
    sigar_uint64_t b_avail = fsusage->avail / 1024;
    unsigned long utotal = b_used + b_avail;
    unsigned long used = b_used;

    if (utotal != 0) {
        unsigned long u100 = used * 100;
        double pct = u100 / utotal +
            ((u100 % utotal != 0) ? 1 : 0);
        return pct / 100;
    }

    return 0;
}

typedef struct {
    sigar_uint32_t eax;
    sigar_uint32_t ebx;
    sigar_uint32_t ecx;
    sigar_uint32_t edx;
} sigar_cpuid_t;

#if defined(__GNUC__) && !defined(__sun)

#  if defined(__i386__)
#  define SIGAR_HAS_CPUID
static void sigar_cpuid(sigar_uint32_t request, sigar_cpuid_t *id)
{
    /* derived from: */
    /* http://svn.red-bean.com/repos/minor/trunk/gc/barriers-ia-32.c */
    asm volatile ("mov %%ebx, %%esi\n\t"
                  "cpuid\n\t"
                  "xchgl %%ebx, %%esi"
                  : "=a" (id->eax),
                    "=S" (id->ebx),
                    "=c" (id->ecx),
                    "=d" (id->edx)
                  : "0" (request)
                  : "memory");
}
#  elif defined(__amd64__)
#  define SIGAR_HAS_CPUID
static void sigar_cpuid(sigar_uint32_t request,
                        sigar_cpuid_t *id)
{
    /* http://svn.red-bean.com/repos/minor/trunk/gc/barriers-amd64.c */
    asm volatile ("cpuid\n\t"
                  : "=a" (id->eax),
                    "=b" (id->ebx),
                    "=c" (id->ecx),
                    "=d" (id->edx)
                  : "0" (request)
                  : "memory");
}
#  endif
#elif defined(WIN32)
#  ifdef _M_X64
#  include <intrin.h>
#  define SIGAR_HAS_CPUID
static void sigar_cpuid(sigar_uint32_t request,
                        sigar_cpuid_t *id)
{
    sigar_uint32_t info[4];
    __cpuid(info, request); /* as of MSVC 7 */
    memcpy(id, &info[0], sizeof(info));
}
#  else
#  define SIGAR_HAS_CPUID
static void sigar_cpuid(sigar_uint32_t request,
                        sigar_cpuid_t *id)
{
    __asm {
        mov edi, id
        mov eax, [edi].eax
        mov ecx, [edi].ecx
        cpuid
        mov [edi].eax, eax
        mov [edi].ebx, ebx
        mov [edi].ecx, ecx
        mov [edi].edx, edx
    }
}
#  endif
#endif

#define INTEL_ID 0x756e6547
#define AMD_ID   0x68747541

int sigar_cpu_core_count(sigar_t *sigar)
{
#if defined(SIGAR_HAS_CPUID)
    sigar_cpuid_t id;

    if (sigar->lcpu == -1) {
        sigar->lcpu = 1;

        sigar_cpuid(0, &id);

        if ((id.ebx == INTEL_ID) || (id.ebx == AMD_ID)) {
            sigar_cpuid(1, &id);

            if (id.edx & (1<<28)) {
                sigar->lcpu = (id.ebx & 0x00FF0000) >> 16;
            }
        }

        sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                         "[cpu] %d cores per socket", sigar->lcpu);
    }

    return sigar->lcpu;
#elif defined(__sun) || defined(__hpux) || defined(_AIX)
    return 1;
#else
    sigar->lcpu = 1;
    return sigar->lcpu;
#endif
}

int sigar_cpu_core_rollup(sigar_t *sigar)
{
#ifdef SIGAR_HAS_CPUID
    int log_rollup =
        SIGAR_LOG_IS_DEBUG(sigar) &&
        (sigar->lcpu == -1);

    (void)sigar_cpu_core_count(sigar);

    if (sigar->cpu_list_cores) {
        if (log_rollup && (sigar->lcpu > 1)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[cpu] treating cores as-is");
        }
    }
    else {
        if (log_rollup && (sigar->lcpu > 1)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "[cpu] rolling up cores to sockets");
            return 1;
        }
    }
#endif
    return 0;
}

#define IS_CPU_R(p) \
   ((*p == '(') && (*(p+1) == 'R') && (*(p+2) == ')'))

typedef struct {
    char *name;  /* search */
    int len;
    char *rname; /* replace */
    int rlen;
} cpu_model_str_t;

/* to later replace 's' with 'r' */
#define CPU_MODEL_ENT_R(s, r) \
    { s, sizeof(s)-1, r, sizeof(r) }

#define CPU_MODEL_ENT(s) \
    CPU_MODEL_ENT_R(s, s)

/* after the vendor part of the string is removed,
 * looking for startsWith the entries below
 * to remove the crap after the model name, see
 * ../exp/intel_amd_cpu_models.txt
 */
static const cpu_model_str_t cpu_models[] = {
    /* intel */
    CPU_MODEL_ENT("Xeon"),
    CPU_MODEL_ENT_R("XEON", "Xeon"),
    CPU_MODEL_ENT("Pentium III"),
    CPU_MODEL_ENT("Pentium II"),
    CPU_MODEL_ENT_R("Pentium(R) III", "Pentium III"),
    CPU_MODEL_ENT_R("Pentium(R) 4", "Pentium 4"),
    CPU_MODEL_ENT_R("Pentium(R) M", "Pentium M"),
    CPU_MODEL_ENT("Pentium Pro"),
    CPU_MODEL_ENT("Celeron"),

    /* amd */
    CPU_MODEL_ENT("Opteron"),
    CPU_MODEL_ENT("Athlon"),
    CPU_MODEL_ENT("Duron"),
    CPU_MODEL_ENT_R("K6(tm)-III", "K6 III"),
    CPU_MODEL_ENT_R("K6(tm) 3D+", "K6 3D+"),
    { NULL }
};

/* common to win32 and linux */
void sigar_cpu_model_adjust(sigar_t *sigar, sigar_cpu_info_t *info)
{
    int len, i;
    char model[128], *ptr=model, *end;

    memcpy(model, info->model, sizeof(model));

    /* trim leading and trailing spaces */
    len = strlen(model);
    end = &model[len-1];
    while (*ptr == ' ') ++ptr;
    while (*end == ' ') *end-- = '\0';

    /* remove vendor from model name */
    len = strlen(info->vendor);
    if (strnEQ(ptr, info->vendor, len)) {
        ptr += len;
        if (IS_CPU_R(ptr)) {
            ptr += 3; /* remove (R) */
        }
        while (*ptr == ' ') ++ptr;
    }

    if (*ptr == '-') {
        ++ptr; /* e.g. was AMD-K6... */
    }

    for (i=0; cpu_models[i].name; i++) {
        const cpu_model_str_t *cpu_model = &cpu_models[i];

        if (strnEQ(ptr, cpu_model->name, cpu_model->len)) {
            memcpy(info->model, cpu_model->rname, cpu_model->rlen);
            return;
        }
    }

    strcpy(info->model, ptr);
}

/* attempt to derive MHz from model name
 * currently works for certain intel strings
 * see exp/intel_amd_cpu_models.txt
 */ 
int sigar_cpu_mhz_from_model(char *model)
{
    int mhz = SIGAR_FIELD_NOTIMPL;
    char *ptr = model;

    while (*ptr && (ptr = strchr(ptr, ' '))) {
        while(*ptr && !sigar_isdigit(*ptr)) {
            ptr++;
        }
        mhz = sigar_strtoul(ptr);

        if (*ptr == '.') {
            /* e.g. "2.40GHz" */
            ++ptr;
            mhz *= 100;
            mhz += sigar_strtoul(ptr);
            break;
        }
        else if (strnEQ(ptr, "GHz", 3) ||
                 strnEQ(ptr, "MHz", 3))
        {
            /* e.g. "1500MHz" */
            break;
        }
        else {
            mhz = SIGAR_FIELD_NOTIMPL;
        }
    }

    if (mhz != SIGAR_FIELD_NOTIMPL) {
        if (strnEQ(ptr, "GHz", 3)) {
            mhz *= 10;
        }
    }

    return mhz;
}

#if !defined(WIN32) && !defined(NETWARE)
#include <netdb.h>
#include <rpc/rpc.h>
#include <rpc/pmap_prot.h>
#include <rpc/pmap_clnt.h>
#ifdef SIGAR_HPUX
#include <nfs/nfs.h>
#endif
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) || defined(__sun) || defined(DARWIN)
#include <arpa/inet.h>
#endif
#if defined(__sun) || defined(SIGAR_HPUX)
#include <rpc/clnt_soc.h>
#endif
#if defined(_AIX) || defined(SIGAR_HPUX) || defined(__OpenBSD__) || defined(__NetBSD__)
#include <sys/socket.h>
#endif

static enum clnt_stat get_sockaddr(struct sockaddr_in *addr, char *host)
{
    register struct hostent *hp;
    sigar_hostent_t data;

    memset(addr, 0, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;

    if ((addr->sin_addr.s_addr = inet_addr(host)) == -1) {
        if (!(hp = sigar_gethostbyname(host, &data))) {
            return RPC_UNKNOWNHOST;
        }
        memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);
    }

    return RPC_SUCCESS;
}

char *sigar_rpc_strerror(int err)
{
    return (char *)clnt_sperrno(err);
}

SIGAR_DECLARE(int) sigar_rpc_ping(char *host,
                                  int protocol,
                                  unsigned long program,
                                  unsigned long version)
{
    CLIENT *client;
    struct sockaddr_in addr;
    int sock;
    struct timeval timeout;
    unsigned short port = 0;
    enum clnt_stat rpc_stat; 

    rpc_stat = get_sockaddr(&addr, host);
    if (rpc_stat != RPC_SUCCESS) {
        return rpc_stat;
    }

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    addr.sin_port = htons(port);
    sock = RPC_ANYSOCK;
    
    if (protocol == SIGAR_NETCONN_UDP) {
        client =
            clntudp_create(&addr, program, version,
                           timeout, &sock);
    }
    else if (protocol == SIGAR_NETCONN_TCP) {
        client =
            clnttcp_create(&addr, program, version,
                           &sock, 0, 0);
    }
    else {
        return RPC_UNKNOWNPROTO;
    }

    if (!client) {
        return rpc_createerr.cf_stat;
    }

    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    rpc_stat = clnt_call(client, NULLPROC, (xdrproc_t)xdr_void, NULL,
                         (xdrproc_t)xdr_void, NULL, timeout);

    clnt_destroy(client);

    return rpc_stat;
}
#endif

int sigar_file2str(const char *fname, char *buffer, int buflen)
{
    int len, status;
    int fd = open(fname, O_RDONLY);

    if (fd < 0) {
        return ENOENT;
    }

    if ((len = read(fd, buffer, buflen)) < 0) {
        status = errno;
    }
    else {
        status = SIGAR_OK;
        buffer[len] = '\0';
    }
    close(fd);

    return status;
}

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#ifdef WIN32
#   define rindex strrchr
#endif

static int proc_module_get_self(void *data, char *name, int len)
{
    sigar_t *sigar = (sigar_t *)data;
    char *ptr = rindex(name, '/');

    if (!ptr) {
        return SIGAR_OK;
    }

    if (strnEQ(ptr+1, "libsigar-", 9)) {
        int offset = ptr - name;

        sigar->self_path = sigar_strdup(name);
        *(sigar->self_path + offset) = '\0'; /* chop libsigar-*.so */

        if (SIGAR_LOG_IS_DEBUG(sigar)) {
            sigar_log_printf(sigar, SIGAR_LOG_DEBUG,
                             "detected sigar-lib='%s'",
                             sigar->self_path);
        }

        return !SIGAR_OK; /* break loop */
    }

    return SIGAR_OK;
}

char *sigar_get_self_path(sigar_t *sigar)
{
    if (!sigar->self_path) {
        sigar_proc_modules_t procmods;
        char *self_path = getenv("SIGAR_PATH");

        if (self_path) {
            sigar->self_path = sigar_strdup(self_path);
            return sigar->self_path;
        }

        procmods.module_getter = proc_module_get_self;
        procmods.data = sigar;

        sigar_proc_modules_get(sigar,
                               sigar_pid_get(sigar),
                               &procmods);

        if (!sigar->self_path) {
            /* dont try again */
            sigar->self_path = sigar_strdup(".");
        }
    }

    return sigar->self_path;
}

#ifdef SIGAR_HAS_DLINFO_MODULES

static int sigar_dlinfo_get(sigar_t *sigar, const char *func,
                            void **handle, Link_map **map)
{
    Dl_info dli;

    if (!dladdr((void *)((uintptr_t)sigar_dlinfo_get), &dli)) {
        sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                         "[%s] dladdr(%s) = %s",
                         func, SIGAR_FUNC, dlerror());
        return ESRCH;
    }

    if (!(*handle = dlopen(dli.dli_fname, RTLD_LAZY))) {
        sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                         "[%s] dlopen(%s) = %s",
                         func, dli.dli_fname, dlerror());
        return ESRCH;
    }

    dlinfo(*handle, RTLD_DI_LINKMAP, map);

    if (!map) {
        sigar_log_printf(sigar, SIGAR_LOG_ERROR,
                         "[%s] dlinfo = %s",
                         func, dlerror());
        return ESRCH;
    }

    return SIGAR_OK;
}

int sigar_dlinfo_modules(sigar_t *sigar, sigar_proc_modules_t *procmods)
{
    int status;
    void *handle;
    Link_map *map;

    status = sigar_dlinfo_get(sigar, SIGAR_FUNC, &handle, &map);
    if (status != SIGAR_OK) {
        return status;
    }

    while (map->l_prev != NULL) {
        map = map->l_prev;
    }

    do {
        int status = 
            procmods->module_getter(procmods->data,
                                    (char *)map->l_name,
                                    strlen(map->l_name));

        if (status != SIGAR_OK) {
            /* not an error; just stop iterating */
            break;
        }
    } while ((map = map->l_next));

    dlclose(handle);

    return SIGAR_OK;
}
#endif

SIGAR_DECLARE(void) sigar_log_printf(sigar_t *sigar, int level,
                                     const char *format, ...)
{
    va_list args;
    char buffer[8192];

    if (level > sigar->log_level) {
        return;
    }

    if (!sigar->log_impl) {
        return;
    }

    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    sigar->log_impl(sigar, sigar->log_data, level, buffer);
}

SIGAR_DECLARE(void) sigar_log(sigar_t *sigar, int level, char *message)
{
    if (level > sigar->log_level) {
        return;
    }

    if (!sigar->log_impl) {
        return;
    }

    sigar->log_impl(sigar, sigar->log_data, level, message);
}

SIGAR_DECLARE(void) sigar_log_impl_set(sigar_t *sigar, void *data,
                                       sigar_log_impl_t impl)
{
    sigar->log_data = data;
    sigar->log_impl = impl;
}

SIGAR_DECLARE(int) sigar_log_level_get(sigar_t *sigar)
{
    return sigar->log_level;
}

static const char *log_levels[] = {
    "FATAL",
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG",
    "TRACE"
};

SIGAR_DECLARE(const char *) sigar_log_level_string_get(sigar_t *sigar)
{
    return log_levels[sigar->log_level];
}

SIGAR_DECLARE(void) sigar_log_level_set(sigar_t *sigar, int level)
{
    sigar->log_level = level;
}

SIGAR_DECLARE(void) sigar_log_impl_file(sigar_t *sigar, void *data,
                                        int level, char *message)
{
    FILE *fp = (FILE*)data;
    fprintf(fp, "[%s] %s\n", log_levels[level], message);
}

#ifndef WIN32
sigar_int64_t sigar_time_now_millis(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((tv.tv_sec * SIGAR_USEC) + tv.tv_usec) / SIGAR_MSEC;
}
#endif
