/*
 * Copyright (c) 2004, 2006 Hyperic, Inc.
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

#define my_pread(fd, ptr, type, offset) \
    (pread(fd, ptr, sizeof(type), offset) == sizeof(type))

int sigar_proc_psinfo_get(sigar_t *sigar, sigar_pid_t pid)
{
    int fd, retval = SIGAR_OK;
    char buffer[BUFSIZ];
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

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/psinfo");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        return ESRCH;
    }

    if (!my_pread(fd, sigar->pinfo, psinfo_t, 0)) {
        retval = errno;
    }

    close(fd);

    return retval;
}

int sigar_proc_usage_get(sigar_t *sigar, prusage_t *prusage, sigar_pid_t pid)
{
    int fd, retval = SIGAR_OK;
    char buffer[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/usage");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        return ESRCH;
    }

    if (!my_pread(fd, prusage, prusage_t, 0)) {
        retval = errno;
    }

    close(fd);

    return retval;
}

int sigar_proc_status_get(sigar_t *sigar, pstatus_t *pstatus, sigar_pid_t pid)
{
    int fd, retval = SIGAR_OK;
    char buffer[BUFSIZ];

    (void)SIGAR_PROC_FILENAME(buffer, pid, "/status");

    if ((fd = open(buffer, O_RDONLY)) < 0) {
        return ESRCH;
    }

    if (!my_pread(fd, pstatus, pstatus_t, 0)) {
        retval = errno;
    }

    close(fd);

    return retval;
}
