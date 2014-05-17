/*
 * Copyright (c) 2004-2005, 2007-2008 Hyperic, Inc.
 * Copyright (c) 2009 SpringSource, Inc.
 * Copyright (c) 2010 VMware, Inc.
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

/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#ifndef WIN32
#  ifdef _AIX
#    define _LARGE_FILES
#  else
#    define _FILE_OFFSET_BITS 64
#    define _LARGEFILE64_SOURCE
#  endif
#endif

#include "sigar.h"


#pragma warning(disable:4996)
#pragma warning(disable:4819)
#pragma warning(disable:4049)
#pragma warning(disable:4217)
#pragma warning(disable:4013)
#pragma warning(disable:4018)
#pragma warning(disable:4244)
#pragma warning(disable:4133)
#pragma warning(disable:4307)
#pragma warning(disable:4293)

#ifndef WIN32
#if defined(__FreeBSD__) || defined(__OpenBSD__)
# include <sys/param.h>
# include <sys/mount.h>
#else
# include <sys/statvfs.h>
# define HAVE_STATVFS
#endif
#include <errno.h>

#define SIGAR_FS_BLOCKS_TO_BYTES(val, bsize) ((val * bsize) >> 1)

int sigar_statvfs(sigar_t *sigar,
                  const char *dirname,
                  sigar_file_system_usage_t *fsusage)
{
    sigar_uint64_t val, bsize;
#ifdef HAVE_STATVFS
    struct statvfs buf;
    int status =
# if defined(__sun) && !defined(_LP64)
        /* http://bugs.opensolaris.org/view_bug.do?bug_id=4462986 */
        statvfs(dirname, (void *)&buf);
# else
        statvfs(dirname, &buf);
# endif
#else
    struct statfs buf;
    int status = statfs(dirname, &buf);
#endif

    if (status != 0) {
        return errno;
    }

#ifdef HAVE_STATVFS
    bsize = buf.f_frsize / 512;
#else
    bsize = buf.f_bsize / 512;
#endif
    val = buf.f_blocks;
    fsusage->total = SIGAR_FS_BLOCKS_TO_BYTES(val, bsize);
    val = buf.f_bfree;
    fsusage->free  = SIGAR_FS_BLOCKS_TO_BYTES(val, bsize);
    val = buf.f_bavail;
    fsusage->avail = SIGAR_FS_BLOCKS_TO_BYTES(val, bsize);
    fsusage->used  = fsusage->total - fsusage->free;
    fsusage->files = buf.f_files;
    fsusage->free_files = buf.f_ffree;

    return SIGAR_OK;
}
#endif

/*
 * whittled down version of apr/file_info/{unix,win32}/filestat.c
 * to fillin sigar_fileattrs_t
 */
#include "sigar_fileinfo.h"
#include "sigar_log.h"

#ifndef SIGAR_ZERO
#define SIGAR_ZERO(s) \
    memset(s, '\0', sizeof(*(s)))
#endif

#ifdef WIN32
#include <windows.h>
sigar_uint64_t sigar_FileTimeToTime(FILETIME *ft);
#else
#include <string.h>
#endif

static const char* types[] = {
    "none",
    "regular",
    "directory",
    "character device",
    "block device",
    "pipe",
    "symbolic link",
    "socket",
    "unknown"
};

SIGAR_DECLARE(const char *)
sigar_file_attrs_type_string_get(sigar_file_type_e type)
{
    if ((type < SIGAR_FILETYPE_NOFILE) ||
        (type > SIGAR_FILETYPE_UNKFILE))
    {
        type = SIGAR_FILETYPE_UNKFILE;
    }

    return types[type];
}

static const sigar_uint64_t perm_modes[] = {
    SIGAR_UREAD, SIGAR_UWRITE, SIGAR_UEXECUTE,
    SIGAR_GREAD, SIGAR_GWRITE, SIGAR_GEXECUTE,
    SIGAR_WREAD, SIGAR_WWRITE, SIGAR_WEXECUTE
};

static const char perm_chars[] = "rwx";

SIGAR_DECLARE(char *)
sigar_file_attrs_permissions_string_get(sigar_uint64_t permissions,
                                        char *str)
{
    char *ptr = str;
    int i=0, j=0;

    for (i=0; i<9; i+=3) {
        for (j=0; j<3; j++) {
            if (permissions & perm_modes[i+j]) {
                *ptr = perm_chars[j];
            }
            else {
                *ptr = '-';
            }
            ptr++;
        }
    }

    *ptr = '\0';
    return str;
}

static const int perm_int[] = {
    400, 200, 100,
     40,  20,  10,
      4,   2,   1
};

SIGAR_DECLARE(int)sigar_file_attrs_mode_get(sigar_uint64_t permissions)
{
    int i=0;
    int perms = 0;

    /* no doubt there is some fancy bitshifting
     * to convert, but this works fine.
     */
    for (i=0; i<9; i++) {
        if (permissions & perm_modes[i]) {
            perms += perm_int[i];
        }
    }

    return perms;
}

#define IS_DOTDIR(dir) \
    ((dir[0] == '.') && (!dir[1] || ((dir[1] == '.') && !dir[2])))

#define DIR_STAT_WARN() \
    sigar_log_printf(sigar, SIGAR_LOG_WARN, \
                     "dir_stat: cannot stat `%s': %s", \
                     name, \
                     sigar_strerror(sigar, status))

#if defined(NETWARE)

int sigar_dir_stat_get(sigar_t *sigar,
                       const char *dir,
                       sigar_dir_stat_t *dirstats)
{
    return SIGAR_ENOTIMPL;
}

int sigar_file_attrs_get(sigar_t *sigar,
                         const char *file,
                         sigar_file_attrs_t *fileattrs)
{
    return SIGAR_ENOTIMPL;
}

int sigar_link_attrs_get(sigar_t *sigar,
                         const char *file,
                         sigar_file_attrs_t *fileattrs)
{
    return SIGAR_ENOTIMPL;
}

#elif defined(WIN32)

#include <accctrl.h>
#include <aclapi.h>

static void fillin_fileattrs(sigar_file_attrs_t *finfo,
                             WIN32_FILE_ATTRIBUTE_DATA *wininfo,
                             int linkinfo)
{
    DWORD *sizes = &wininfo->nFileSizeHigh;

    finfo->atime = sigar_FileTimeToTime(&wininfo->ftLastAccessTime) / 1000;
    finfo->ctime = sigar_FileTimeToTime(&wininfo->ftCreationTime) / 1000;
    finfo->mtime = sigar_FileTimeToTime(&wininfo->ftLastWriteTime) / 1000;

    finfo->size =
        (sigar_uint64_t)sizes[1] | ((sigar_uint64_t)sizes[0] << 32);

    if (linkinfo &&
        (wininfo->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)) {
        finfo->type = SIGAR_FILETYPE_LNK;
    }
    else if (wininfo->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        finfo->type = SIGAR_FILETYPE_DIR;
    }
    else {
        finfo->type = SIGAR_FILETYPE_REG;
    }
}

static sigar_uint64_t convert_perms(ACCESS_MASK acc, sigar_uint64_t scope)
{
    sigar_uint64_t perms = 0;
    if (acc & FILE_EXECUTE) {
        perms |= SIGAR_WEXECUTE;
    }
    if (acc & FILE_WRITE_DATA) {
        perms |= SIGAR_WWRITE;
    }
    if (acc & FILE_READ_DATA) {
        perms |= SIGAR_WREAD;
    }

    return (perms << scope);
}

static int get_security_info(sigar_t *sigar,
                             const char *file,
                             sigar_file_attrs_t *fileattrs)
{
    DWORD retval;
    PSID user = NULL, group = NULL, world = NULL;
    PACL dacl = NULL;
    PSECURITY_DESCRIPTOR pdesc = NULL;
    SECURITY_INFORMATION sinfo =
        OWNER_SECURITY_INFORMATION |
        GROUP_SECURITY_INFORMATION |        
        DACL_SECURITY_INFORMATION;
    TRUSTEE ident = {NULL, NO_MULTIPLE_TRUSTEE, TRUSTEE_IS_SID};
    ACCESS_MASK acc;
    SID_IDENTIFIER_AUTHORITY auth = SECURITY_WORLD_SID_AUTHORITY;

    retval = GetNamedSecurityInfo((char *)file,
                                  SE_FILE_OBJECT,
                                  sinfo,
                                  &user,
                                  &group,
                                  &dacl,
                                  NULL,
                                  &pdesc);

    if (retval != ERROR_SUCCESS) {
        return retval;
    }

    if (!AllocateAndInitializeSid(&auth, 1, SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0, &world))
    {
        world = NULL;
    }

    ident.TrusteeType = TRUSTEE_IS_USER;
    ident.ptstrName = user;
    if (GetEffectiveRightsFromAcl(dacl, &ident, &acc) == ERROR_SUCCESS) {
        fileattrs->permissions |= convert_perms(acc, 8);
    }

    ident.TrusteeType = TRUSTEE_IS_GROUP;
    ident.ptstrName = group;
    if (GetEffectiveRightsFromAcl(dacl, &ident, &acc) == ERROR_SUCCESS) {
        fileattrs->permissions |= convert_perms(acc, 4);
    }

    if (world) {
        ident.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ident.ptstrName = world;
        if (GetEffectiveRightsFromAcl(dacl, &ident, &acc) == ERROR_SUCCESS) {
            fileattrs->permissions |= convert_perms(acc, 0);
        }
    }

    if (world) {
        FreeSid(world);
    }

    LocalFree(pdesc);

    return SIGAR_OK;
}

static int fileattrs_get(sigar_t *sigar,
                         const char *file,
                         sigar_file_attrs_t *fileattrs,
                         int linkinfo)
{
    BY_HANDLE_FILE_INFORMATION info;
    WIN32_FILE_ATTRIBUTE_DATA attrs;
    HANDLE handle;
    DWORD flags;

    SIGAR_ZERO(fileattrs);

    if (!GetFileAttributesExA(file,
                              GetFileExInfoStandard,
                              &attrs))
    {
        return GetLastError();
    }

    fillin_fileattrs(fileattrs, &attrs, linkinfo);

    flags = fileattrs->type == SIGAR_FILETYPE_DIR ?
        FILE_FLAG_BACKUP_SEMANTICS :
        FILE_ATTRIBUTE_NORMAL;

    /**
     * We need to set dwDesiredAccess to 0 to work in cases where GENERIC_READ can fail.
     *
     * see: http://msdn.microsoft.com/en-us/library/aa363858(VS.85).aspx
     */
    handle = CreateFile(file,
                        0,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        flags,
                        NULL);
 
    if (handle != INVALID_HANDLE_VALUE) {
        if (GetFileInformationByHandle(handle, &info)) {
            fileattrs->inode =
                info.nFileIndexLow |
                (info.nFileIndexHigh << 32);
            fileattrs->device = info.dwVolumeSerialNumber;
            fileattrs->nlink  = info.nNumberOfLinks;
        }
        CloseHandle(handle);
    }

    get_security_info(sigar, file, fileattrs);

    return SIGAR_OK;
}

SIGAR_DECLARE(int) sigar_file_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs)
{
    return fileattrs_get(sigar, file, fileattrs, 0);
}

SIGAR_DECLARE(int) sigar_link_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs)
{
    return fileattrs_get(sigar, file, fileattrs, 1);
}

static __inline int file_type(char *file)
{
    WIN32_FILE_ATTRIBUTE_DATA attrs;

    if (!GetFileAttributesExA(file,
                              GetFileExInfoStandard,
                              &attrs))
    {
        return -1;
    }

    if (attrs.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
        return SIGAR_FILETYPE_LNK;
    }
    else if (attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        return SIGAR_FILETYPE_DIR;
    }
    else {
        return SIGAR_FILETYPE_REG;
    }
}

static int dir_stat_get(sigar_t *sigar,
                        const char *dir,
                        sigar_dir_stat_t *dirstats,
                        int recurse)
{
    int status;
    char name[SIGAR_PATH_MAX+1];
    int len = strlen(dir);
    int max = sizeof(name)-len-1;
    char *ptr = name;
    WIN32_FIND_DATA data;
    HANDLE handle;
    DWORD error;
    char delim;

    if (file_type((char *)dir) != SIGAR_FILETYPE_DIR) {
        return ERROR_NO_MORE_FILES;
    }

    strncpy(name, dir, sizeof(name));
    ptr += len;
    if (strchr(dir, '/')) {
        delim = '/';
    }
    else {
        delim = '\\';
    }
    if (name[len] != delim) {
        *ptr++ = delim;
        len++;
        max--;
    }

    /* e.g. "C:\sigar\*" */
    name[len] = '*';
    name[len+1] = '\0';

    handle = FindFirstFile(name, &data);
    if (handle == INVALID_HANDLE_VALUE) {
        return GetLastError();
    }

    do {
        /* skip '.' and '..' */
        if (IS_DOTDIR(data.cFileName)) {
            continue;
        }

        dirstats->disk_usage +=
            (data.nFileSizeHigh * (MAXDWORD+1)) +
            data.nFileSizeLow;

        /* e.g. "C:\sigar\lib" */
        strncpy(ptr, data.cFileName, max);
        ptr[max] = '\0';

        switch (file_type(name)) {
          case -1:
            break;
          case SIGAR_FILETYPE_REG:
            ++dirstats->files;
            break;
          case SIGAR_FILETYPE_DIR:
            ++dirstats->subdirs;
            if (recurse) {
                status = 
                    dir_stat_get(sigar, name,
                                 dirstats, recurse);
                if (status != SIGAR_OK) {
                    DIR_STAT_WARN();
                }
            }
            break;
          case SIGAR_FILETYPE_LNK:
            ++dirstats->symlinks;
            break;
          case SIGAR_FILETYPE_CHR:
            ++dirstats->chrdevs;
            break;
          case SIGAR_FILETYPE_BLK:
            ++dirstats->blkdevs;
            break;
          case SIGAR_FILETYPE_SOCK:
            ++dirstats->sockets;
            break;
          default:
            ++dirstats->total;
        }
    } while (FindNextFile(handle, &data));

    error = GetLastError();

    FindClose(handle);

    if (error != ERROR_NO_MORE_FILES) {
        return error;
    }

    dirstats->total =
        dirstats->files +
        dirstats->subdirs +
        dirstats->symlinks +
        dirstats->chrdevs +
        dirstats->blkdevs +
        dirstats->sockets;

    return SIGAR_OK;
}

#else

#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

static sigar_file_type_e filetype_from_mode(mode_t mode)
{
    sigar_file_type_e type;

    switch (mode & S_IFMT) {
    case S_IFREG:
        type = SIGAR_FILETYPE_REG;  break;
    case S_IFDIR:
        type = SIGAR_FILETYPE_DIR;  break;
    case S_IFLNK:
        type = SIGAR_FILETYPE_LNK;  break;
    case S_IFCHR:
        type = SIGAR_FILETYPE_CHR;  break;
    case S_IFBLK:
        type = SIGAR_FILETYPE_BLK;  break;
#if defined(S_IFFIFO)
    case S_IFFIFO:
        type = SIGAR_FILETYPE_PIPE; break;
#endif
#if !defined(BEOS) && defined(S_IFSOCK)
    case S_IFSOCK:
        type = SIGAR_FILETYPE_SOCK; break;
#endif

    default:
	/* Work around missing S_IFxxx values above
         * for Linux et al.
         */
#if !defined(S_IFFIFO) && defined(S_ISFIFO)
    	if (S_ISFIFO(mode)) {
            type = SIGAR_FILETYPE_PIPE;
	} else
#endif
#if !defined(BEOS) && !defined(S_IFSOCK) && defined(S_ISSOCK)
    	if (S_ISSOCK(mode)) {
            type = SIGAR_FILETYPE_SOCK;
	} else
#endif
        type = SIGAR_FILETYPE_UNKFILE;
    }
    return type;
}

static sigar_uint64_t sigar_unix_mode2perms(mode_t mode)
{
    sigar_uint64_t perms = 0;

    if (mode & S_IRUSR)
        perms |= SIGAR_UREAD;
    if (mode & S_IWUSR)
        perms |= SIGAR_UWRITE;
    if (mode & S_IXUSR)
        perms |= SIGAR_UEXECUTE;

    if (mode & S_IRGRP)
        perms |= SIGAR_GREAD;
    if (mode & S_IWGRP)
        perms |= SIGAR_GWRITE;
    if (mode & S_IXGRP)
        perms |= SIGAR_GEXECUTE;

    if (mode & S_IROTH)
        perms |= SIGAR_WREAD;
    if (mode & S_IWOTH)
        perms |= SIGAR_WWRITE;
    if (mode & S_IXOTH)
        perms |= SIGAR_WEXECUTE;

    return perms;
}

static void copy_stat_info(sigar_file_attrs_t *fileattrs,
                           struct stat *info)
{
    fileattrs->permissions = sigar_unix_mode2perms(info->st_mode);
    fileattrs->type        = filetype_from_mode(info->st_mode);
    fileattrs->uid         = info->st_uid;
    fileattrs->gid         = info->st_gid;
    fileattrs->size        = info->st_size;
    fileattrs->inode       = info->st_ino;
    fileattrs->device      = info->st_dev;
    fileattrs->nlink       = info->st_nlink;
    fileattrs->atime       = info->st_atime;
    fileattrs->mtime       = info->st_mtime;
    fileattrs->ctime       = info->st_ctime;
    fileattrs->atime *= 1000;
    fileattrs->mtime *= 1000;
    fileattrs->ctime *= 1000;
}

int sigar_file_attrs_get(sigar_t *sigar,
                         const char *file,
                         sigar_file_attrs_t *fileattrs)
{
    struct stat info;

    if (stat(file, &info) == 0) {
        copy_stat_info(fileattrs, &info);
        return SIGAR_OK;
    }
    else {
        return errno;
    }
}

int sigar_link_attrs_get(sigar_t *sigar,
                         const char *file,
                         sigar_file_attrs_t *fileattrs)
{
    struct stat info;

    if (lstat(file, &info) == 0) {
        copy_stat_info(fileattrs, &info);
        return SIGAR_OK;
    }
    else {
        return errno;
    }
}

static int dir_stat_get(sigar_t *sigar,
                        const char *dir,
                        sigar_dir_stat_t *dirstats,
                        int recurse)
{
    int status;
    char name[SIGAR_PATH_MAX+1];
    int len = strlen(dir);
    int max = sizeof(name)-len-1;
    char *ptr = name;
    DIR *dirp = opendir(dir);
    struct dirent *ent;
    struct stat info;
#ifdef HAVE_READDIR_R
    struct dirent dbuf;
#endif

    if (!dirp) {
        return errno;
    }

    strncpy(name, dir, sizeof(name));
    ptr += len;
    if (name[len] != '/') {
        *ptr++ = '/';
        len++;
        max--;
    }

#ifdef HAVE_READDIR_R
    while (readdir_r(dirp, &dbuf, &ent) == 0) {
        if (ent == NULL) {
            break;
        }
#else
    while ((ent = readdir(dirp))) {
#endif
        /* skip '.' and '..' */
        if (IS_DOTDIR(ent->d_name)) {
            continue;
        }

        strncpy(ptr, ent->d_name, max);
        ptr[max] = '\0';

        if (lstat(name, &info) != 0) {
            continue;
        }

        dirstats->disk_usage += info.st_size;

        switch (filetype_from_mode(info.st_mode)) {
          case SIGAR_FILETYPE_REG:
            ++dirstats->files;
            break;
          case SIGAR_FILETYPE_DIR:
            ++dirstats->subdirs;
            if (recurse) {
                status = 
                    dir_stat_get(sigar, name,
                                 dirstats, recurse);
                if (status != SIGAR_OK) {
                    DIR_STAT_WARN();
                }
            }
            break;
          case SIGAR_FILETYPE_LNK:
            ++dirstats->symlinks;
            break;
          case SIGAR_FILETYPE_CHR:
            ++dirstats->chrdevs;
            break;
          case SIGAR_FILETYPE_BLK:
            ++dirstats->blkdevs;
            break;
          case SIGAR_FILETYPE_SOCK:
            ++dirstats->sockets;
            break;
          default:
            ++dirstats->total;
        }
    }

    dirstats->total =
        dirstats->files +
        dirstats->subdirs +
        dirstats->symlinks +
        dirstats->chrdevs +
        dirstats->blkdevs +
        dirstats->sockets;

    closedir(dirp);

    return SIGAR_OK;
}

#endif

SIGAR_DECLARE(int) sigar_dir_stat_get(sigar_t *sigar,
                                      const char *dir,
                                      sigar_dir_stat_t *dirstats)
{
    SIGAR_ZERO(dirstats);
    return dir_stat_get(sigar, dir, dirstats, 0);
}

SIGAR_DECLARE(int) sigar_dir_usage_get(sigar_t *sigar,
                                       const char *dir,
                                       sigar_dir_usage_t *dirusage)
{
    SIGAR_ZERO(dirusage);
    return dir_stat_get(sigar, dir, dirusage, 1);
}
