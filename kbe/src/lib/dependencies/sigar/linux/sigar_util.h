/*
 * Copyright (c) 2004-2008 Hyperic, Inc.
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

#ifndef SIGAR_UTIL_H
#define SIGAR_UTIL_H

/* most of this is crap for dealing with linux /proc */
#define UITOA_BUFFER_SIZE \
    (sizeof(int) * 3 + 1)

#define SSTRLEN(s) \
    (sizeof(s)-1)

#define sigar_strtoul(ptr) \
    strtoul(ptr, &ptr, 10)

#define sigar_strtoull(ptr) \
    strtoull(ptr, &ptr, 10)

#define sigar_isspace(c) \
    (isspace(((unsigned char)(c))))

#define sigar_isdigit(c) \
    (isdigit(((unsigned char)(c))))

#define sigar_isalpha(c) \
    (isalpha(((unsigned char)(c))))

#define sigar_isupper(c) \
    (isupper(((unsigned char)(c))))

#define sigar_tolower(c) \
    (tolower(((unsigned char)(c))))

#ifdef WIN32
#define sigar_fileno _fileno
#define sigar_isatty _isatty
#define sigar_write  _write
#else
#define sigar_fileno fileno
#define sigar_isatty isatty
#define sigar_write  write
#endif

#ifndef PROC_FS_ROOT
#define PROC_FS_ROOT "/proc/"
#endif

#ifndef PROCP_FS_ROOT
#define PROCP_FS_ROOT "/proc/"
#endif

sigar_int64_t sigar_time_now_millis(void);

char *sigar_uitoa(char *buf, unsigned int n, int *len);

int sigar_inet_ntoa(sigar_t *sigar,
                    sigar_uint32_t address,
                    char *addr_str);

struct hostent *sigar_gethostbyname(const char *name,
                                    sigar_hostent_t *data);

char *sigar_skip_line(char *buffer, int buflen);
char *sigar_skip_token(char *p);
char *sigar_skip_multiple_token(char *p, int count);

char *sigar_getword(char **line, char stop);

char *sigar_strcasestr(const char *s1, const char *s2);

int sigar_file2str(const char *fname, char *buffer, int buflen);

int sigar_proc_file2str(char *buffer, int buflen,
                        sigar_pid_t pid,
                        const char *fname,
                        int fname_len);

#define SIGAR_PROC_FILE2STR(buffer, pid, fname) \
    sigar_proc_file2str(buffer, sizeof(buffer), \
                        pid, fname, SSTRLEN(fname))

#define SIGAR_PROC_FILENAME(buffer, pid, fname) \
    sigar_proc_filename(buffer, sizeof(buffer), \
                        pid, fname, SSTRLEN(fname))

#define SIGAR_SKIP_SPACE(ptr) \
    while (sigar_isspace(*ptr)) ++ptr

char *sigar_proc_filename(char *buffer, int buflen,
                          sigar_pid_t pid,
                          const char *fname, int fname_len);

int sigar_proc_list_procfs_get(sigar_t *sigar,
                               sigar_proc_list_t *proclist);

int sigar_proc_fd_count(sigar_t *sigar, sigar_pid_t pid,
                        sigar_uint64_t *total);

/* linux + freebsd */
int sigar_procfs_args_get(sigar_t *sigar, sigar_pid_t pid,
                          sigar_proc_args_t *procargs);

int sigar_mem_calc_ram(sigar_t *sigar, sigar_mem_t *mem);

int sigar_statvfs(sigar_t *sigar,
                  const char *dirname,
                  sigar_file_system_usage_t *fsusage);

double sigar_file_system_usage_calc_used(sigar_t *sigar,
                                         sigar_file_system_usage_t *fs);

#define SIGAR_DEV_PREFIX "/dev/"

#define SIGAR_NAME_IS_DEV(dev) \
    strnEQ(dev, SIGAR_DEV_PREFIX, SSTRLEN(SIGAR_DEV_PREFIX))

typedef struct {
    char name[256];
    int is_partition;
    sigar_disk_usage_t disk;
} sigar_iodev_t;

sigar_iodev_t *sigar_iodev_get(sigar_t *sigar,
                               const char *dirname);

int sigar_cpu_core_count(sigar_t *sigar);

/* e.g. VM guest may have 1 virtual ncpu on multicore hosts */
#define sigar_cpu_socket_count(sigar) \
    (sigar->ncpu < sigar->lcpu) ? sigar->ncpu : \
    (sigar->ncpu / sigar->lcpu)

int sigar_cpu_core_rollup(sigar_t *sigar);

void sigar_cpu_model_adjust(sigar_t *sigar, sigar_cpu_info_t *info);

int sigar_cpu_mhz_from_model(char *model);

char *sigar_get_self_path(sigar_t *sigar);

#if defined(__sun) || defined(__FreeBSD__)

#define SIGAR_HAS_DLINFO_MODULES
#include <dlfcn.h>
#include <link.h>

int sigar_dlinfo_modules(sigar_t *sigar, sigar_proc_modules_t *procmods);
#endif

typedef struct sigar_cache_entry_t sigar_cache_entry_t;

struct sigar_cache_entry_t {
    sigar_cache_entry_t *next;
    sigar_uint64_t id;
    void *value;
};

typedef struct {
    sigar_cache_entry_t **entries;
    unsigned int count, size;
    void (*free_value)(void *ptr);
} sigar_cache_t;

sigar_cache_t *sigar_cache_new(int size);

sigar_cache_entry_t *sigar_cache_get(sigar_cache_t *table,
                                     sigar_uint64_t key);

sigar_cache_entry_t *sigar_cache_find(sigar_cache_t *table,
                                      sigar_uint64_t key);

void sigar_cache_destroy(sigar_cache_t *table);

#endif /* SIGAR_UTIL_H */
