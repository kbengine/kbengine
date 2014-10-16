/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "apr_private.h"
#include "apr_arch_file_io.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_strings.h"
#include "apr_portable.h"
#include "apr_thread_mutex.h"
#if APR_HAVE_ERRNO_H
#include <errno.h>
#endif
#include <winbase.h>
#include <string.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include "apr_arch_misc.h"
#include "apr_arch_inherit.h"
#include <io.h>
#include <winioctl.h>

#if APR_HAS_UNICODE_FS
apr_status_t utf8_to_unicode_path(apr_wchar_t* retstr, apr_size_t retlen, 
                                  const char* srcstr)
{
    /* TODO: The computations could preconvert the string to determine
     * the true size of the retstr, but that's a memory over speed
     * tradeoff that isn't appropriate this early in development.
     *
     * Allocate the maximum string length based on leading 4 
     * characters of \\?\ (allowing nearly unlimited path lengths) 
     * plus the trailing null, then transform /'s into \\'s since
     * the \\?\ form doesn't allow '/' path seperators.
     *
     * Note that the \\?\ form only works for local drive paths, and
     * \\?\UNC\ is needed UNC paths.
     */
    apr_size_t srcremains = strlen(srcstr) + 1;
    apr_wchar_t *t = retstr;
    apr_status_t rv;

    /* This is correct, we don't twist the filename if it is will
     * definitely be shorter than 248 characters.  It merits some 
     * performance testing to see if this has any effect, but there
     * seem to be applications that get confused by the resulting
     * Unicode \\?\ style file names, especially if they use argv[0]
     * or call the Win32 API functions such as GetModuleName, etc.
     * Not every application is prepared to handle such names.
     * 
     * Note also this is shorter than MAX_PATH, as directory paths 
     * are actually limited to 248 characters. 
     *
     * Note that a utf-8 name can never result in more wide chars
     * than the original number of utf-8 narrow chars.
     */
    if (srcremains > 248) {
        if (srcstr[1] == ':' && (srcstr[2] == '/' || srcstr[2] == '\\')) {
            wcscpy (retstr, L"\\\\?\\");
            retlen -= 4;
            t += 4;
        }
        else if ((srcstr[0] == '/' || srcstr[0] == '\\')
              && (srcstr[1] == '/' || srcstr[1] == '\\')
              && (srcstr[2] != '?')) {
            /* Skip the slashes */
            srcstr += 2;
            srcremains -= 2;
            wcscpy (retstr, L"\\\\?\\UNC\\");
            retlen -= 8;
            t += 8;
        }
    }

    if ((rv = apr_conv_utf8_to_ucs2(srcstr, &srcremains, t, &retlen))) {
        return (rv == APR_INCOMPLETE) ? APR_EINVAL : rv;
    }
    if (srcremains) {
        return APR_ENAMETOOLONG;
    }
    for (; *t; ++t)
        if (*t == L'/')
            *t = L'\\';
    return APR_SUCCESS;
}

apr_status_t unicode_to_utf8_path(char* retstr, apr_size_t retlen,
                                  const apr_wchar_t* srcstr)
{
    /* Skip the leading 4 characters if the path begins \\?\, or substitute
     * // for the \\?\UNC\ path prefix, allocating the maximum string
     * length based on the remaining string, plus the trailing null.
     * then transform \\'s back into /'s since the \\?\ form never
     * allows '/' path seperators, and APR always uses '/'s.
     */
    apr_size_t srcremains = wcslen(srcstr) + 1;
    apr_status_t rv;
    char *t = retstr;
    if (srcstr[0] == L'\\' && srcstr[1] == L'\\' && 
        srcstr[2] == L'?'  && srcstr[3] == L'\\') {
        if (srcstr[4] == L'U' && srcstr[5] == L'N' && 
            srcstr[6] == L'C' && srcstr[7] == L'\\') {
            srcremains -= 8;
            srcstr += 8;
            retstr[0] = '\\';
            retstr[1] = '\\';
            retlen -= 2;
            t += 2;
        }
        else {
            srcremains -= 4;
            srcstr += 4;
        }
    }
        
    if ((rv = apr_conv_ucs2_to_utf8(srcstr, &srcremains, t, &retlen))) {
        return rv;
    }
    if (srcremains) {
        return APR_ENAMETOOLONG;
    }
    return APR_SUCCESS;
}
#endif

void *res_name_from_filename(const char *file, int global, apr_pool_t *pool)
{
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t *wpre, *wfile, *ch;
        apr_size_t n = strlen(file) + 1;
        apr_size_t r, d;

        if (apr_os_level >= APR_WIN_2000) {
            if (global)
                wpre = L"Global\\";
            else
                wpre = L"Local\\";
        }
        else
            wpre = L"";
        r = wcslen(wpre);

        if (n > 256 - r) {
            file += n - 256 - r;
            n = 256;
            /* skip utf8 continuation bytes */
            while ((*file & 0xC0) == 0x80) {
                ++file;
                --n;
            }
        }
        wfile = apr_palloc(pool, (r + n) * sizeof(apr_wchar_t));
        wcscpy(wfile, wpre);
        d = n;
        if (apr_conv_utf8_to_ucs2(file, &n, wfile + r, &d)) {
            return NULL;
        }
        for (ch = wfile + r; *ch; ++ch) {
            if (*ch == ':' || *ch == '/' || *ch == '\\')
                *ch = '_';
        }
        return wfile;
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        char *nfile, *ch;
        apr_size_t n = strlen(file) + 1;

#if !APR_HAS_UNICODE_FS
        apr_size_t r, d;
        char *pre;

        if (apr_os_level >= APR_WIN_2000) {
            if (global)
                pre = "Global\\";
            else
                pre = "Local\\";
        }
        else
            pre = "";
        r = strlen(pre);

        if (n > 256 - r) {
            file += n - 256 - r;
            n = 256;
        }
        nfile = apr_palloc(pool, (r + n) * sizeof(apr_wchar_t));
        memcpy(nfile, pre, r);
        memcpy(nfile + r, file, n);
#else
        const apr_size_t r = 0;
        if (n > 256) {
            file += n - 256;
            n = 256;
        }
        nfile = apr_pmemdup(pool, file, n);
#endif
        for (ch = nfile + r; *ch; ++ch) {
            if (*ch == ':' || *ch == '/' || *ch == '\\')
                *ch = '_';
        }
        return nfile;
    }
#endif
}

#if APR_HAS_UNICODE_FS
static apr_status_t make_sparse_file(apr_file_t *file)
{
    BY_HANDLE_FILE_INFORMATION info;
    apr_status_t rv;
    DWORD bytesread = 0;
    DWORD res;

    /* test */

    if (GetFileInformationByHandle(file->filehand, &info)
            && (info.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE))
        return APR_SUCCESS;

    if (file->pOverlapped) {
        file->pOverlapped->Offset     = 0;
        file->pOverlapped->OffsetHigh = 0;
    }

    if (DeviceIoControl(file->filehand, FSCTL_SET_SPARSE, NULL, 0, NULL, 0,
                        &bytesread, file->pOverlapped)) {
        rv = APR_SUCCESS;
    }
    else 
    {
        rv = apr_get_os_error();

        if (rv == APR_FROM_OS_ERROR(ERROR_IO_PENDING))
        {
            do {
                res = WaitForSingleObject(file->pOverlapped->hEvent, 
                                          (file->timeout > 0)
                                            ? (DWORD)(file->timeout/1000)
                                            : ((file->timeout == -1) 
                                                 ? INFINITE : 0));
            } while (res == WAIT_ABANDONED);

            if (res != WAIT_OBJECT_0) {
                CancelIo(file->filehand);
            }

            if (GetOverlappedResult(file->filehand, file->pOverlapped, 
                                    &bytesread, TRUE))
                rv = APR_SUCCESS;
            else
                rv = apr_get_os_error();
        }
    }
    return rv;
}
#endif

apr_status_t file_cleanup(void *thefile)
{
    apr_file_t *file = thefile;
    apr_status_t flush_rv = APR_SUCCESS;

    if (file->filehand != INVALID_HANDLE_VALUE) {

        if (file->buffered) {
            /* XXX: flush here is not mutex protected */
            flush_rv = apr_file_flush((apr_file_t *)thefile);
        }

        /* In order to avoid later segfaults with handle 'reuse',
         * we must protect against the case that a dup2'ed handle
         * is being closed, and invalidate the corresponding StdHandle 
         * We also tell msvcrt when stdhandles are closed.
         */
        if (file->flags & APR_STD_FLAGS)
        {
            if ((file->flags & APR_STD_FLAGS) == APR_STDERR_FLAG) {
                _close(2);
                SetStdHandle(STD_ERROR_HANDLE, INVALID_HANDLE_VALUE);
            }
            else if ((file->flags & APR_STD_FLAGS) == APR_STDOUT_FLAG) {
                _close(1);
                SetStdHandle(STD_OUTPUT_HANDLE, INVALID_HANDLE_VALUE);
            }
            else if ((file->flags & APR_STD_FLAGS) == APR_STDIN_FLAG) {
                _close(0);
                SetStdHandle(STD_INPUT_HANDLE, INVALID_HANDLE_VALUE);
            }
        }
        else
            CloseHandle(file->filehand);

        file->filehand = INVALID_HANDLE_VALUE;
    }
    if (file->pOverlapped && file->pOverlapped->hEvent) {
        CloseHandle(file->pOverlapped->hEvent);
        file->pOverlapped = NULL;
    }
    return flush_rv;
}

APR_DECLARE(apr_status_t) apr_file_open(apr_file_t **new, const char *fname,
                                   apr_int32_t flag, apr_fileperms_t perm,
                                   apr_pool_t *pool)
{
    HANDLE handle = INVALID_HANDLE_VALUE;
    DWORD oflags = 0;
    DWORD createflags = 0;
    DWORD attributes = 0;
    DWORD sharemode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    apr_status_t rv;

    if (flag & APR_FOPEN_NONBLOCK) {
        return APR_ENOTIMPL;
    }
    if (flag & APR_FOPEN_READ) {
        oflags |= GENERIC_READ;
    }
    if (flag & APR_FOPEN_WRITE) {
        oflags |= GENERIC_WRITE;
    }
    if (flag & APR_WRITEATTRS) {
        oflags |= FILE_WRITE_ATTRIBUTES;
    }

    if (apr_os_level >= APR_WIN_NT) 
        sharemode |= FILE_SHARE_DELETE;

    if (flag & APR_FOPEN_CREATE) {
        if (flag & APR_FOPEN_EXCL) {
            /* only create new if file does not already exist */
            createflags = CREATE_NEW;
        } else if (flag & APR_FOPEN_TRUNCATE) {
            /* truncate existing file or create new */
            createflags = CREATE_ALWAYS;
        } else {
            /* open existing but create if necessary */
            createflags = OPEN_ALWAYS;
        }
    } else if (flag & APR_FOPEN_TRUNCATE) {
        /* only truncate if file already exists */
        createflags = TRUNCATE_EXISTING;
    } else {
        /* only open if file already exists */
        createflags = OPEN_EXISTING;
    }

    if ((flag & APR_FOPEN_EXCL) && !(flag & APR_FOPEN_CREATE)) {
        return APR_EACCES;
    }   
    
    if (flag & APR_FOPEN_DELONCLOSE) {
        attributes |= FILE_FLAG_DELETE_ON_CLOSE;
    }

    if (flag & APR_OPENLINK) {
       attributes |= FILE_FLAG_OPEN_REPARSE_POINT;
    }

    /* Without READ or WRITE, we fail unless apr called apr_file_open
     * internally with the private APR_OPENINFO flag.
     *
     * With the APR_OPENINFO flag on NT, use the option flag
     * FILE_FLAG_BACKUP_SEMANTICS to allow us to open directories.
     * See the static resolve_ident() fn in file_io/win32/filestat.c
     */
    if (!(flag & (APR_FOPEN_READ | APR_FOPEN_WRITE))) {
        if (flag & APR_OPENINFO) {
            if (apr_os_level >= APR_WIN_NT) {
                attributes |= FILE_FLAG_BACKUP_SEMANTICS;
            }
        }
        else {
            return APR_EACCES;
        }
        if (flag & APR_READCONTROL)
            oflags |= READ_CONTROL;
    }

    if (flag & APR_FOPEN_XTHREAD) {
        /* This win32 specific feature is required 
         * to allow multiple threads to work with the file.
         */
        attributes |= FILE_FLAG_OVERLAPPED;
    }

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wfname[APR_PATH_MAX];

        if (flag & APR_FOPEN_SENDFILE_ENABLED) {
            /* This feature is required to enable sendfile operations
             * against the file on Win32. Also implies APR_FOPEN_XTHREAD.
             */
            flag |= APR_FOPEN_XTHREAD;
            attributes |= FILE_FLAG_SEQUENTIAL_SCAN | FILE_FLAG_OVERLAPPED;
        }

        if ((rv = utf8_to_unicode_path(wfname, sizeof(wfname) 
                                             / sizeof(apr_wchar_t), fname)))
            return rv;
        handle = CreateFileW(wfname, oflags, sharemode,
                             NULL, createflags, attributes, 0);
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI {
        handle = CreateFileA(fname, oflags, sharemode,
                             NULL, createflags, attributes, 0);
        /* This feature is not supported on this platform. */
        flag &= ~APR_FOPEN_SENDFILE_ENABLED;
    }
#endif
    if (handle == INVALID_HANDLE_VALUE) {
        return apr_get_os_error();
    }

    (*new) = (apr_file_t *)apr_pcalloc(pool, sizeof(apr_file_t));
    (*new)->pool = pool;
    (*new)->filehand = handle;
    (*new)->fname = apr_pstrdup(pool, fname);
    (*new)->flags = flag;
    (*new)->timeout = -1;
    (*new)->ungetchar = -1;

    if (flag & APR_FOPEN_APPEND) {
        (*new)->append = 1;
        SetFilePointer((*new)->filehand, 0, NULL, FILE_END);
    }
    if (flag & APR_FOPEN_BUFFERED) {
        (*new)->buffered = 1;
        (*new)->buffer = apr_palloc(pool, APR_FILE_DEFAULT_BUFSIZE);
        (*new)->bufsize = APR_FILE_DEFAULT_BUFSIZE;
    }
    /* Need the mutex to handled buffered and O_APPEND style file i/o */
    if ((*new)->buffered || (*new)->append) {
        rv = apr_thread_mutex_create(&(*new)->mutex, 
                                     APR_THREAD_MUTEX_DEFAULT, pool);
        if (rv) {
            if (file_cleanup(*new) == APR_SUCCESS) {
                apr_pool_cleanup_kill(pool, *new, file_cleanup);
            }
            return rv;
        }
    }

#if APR_HAS_UNICODE_FS
    if ((apr_os_level >= APR_WIN_2000) && ((*new)->flags & APR_FOPEN_SPARSE)) {
        if ((rv = make_sparse_file(*new)) != APR_SUCCESS)
            /* The great mystery; do we close the file and return an error?
             * Do we add a new APR_INCOMPLETE style error saying opened, but
             * NOTSPARSE?  For now let's simply mark the file as not-sparse.
             */
            (*new)->flags &= ~APR_FOPEN_SPARSE;
    }
    else
#endif
        /* This feature is not supported on this platform. */
        (*new)->flags &= ~APR_FOPEN_SPARSE;

#if APR_FILES_AS_SOCKETS
    /* Create a pollset with room for one descriptor. */
    /* ### check return codes */
    (void) apr_pollset_create(&(*new)->pollset, 1, pool, 0);
#endif
    if (!(flag & APR_FOPEN_NOCLEANUP)) {
        apr_pool_cleanup_register((*new)->pool, (void *)(*new), file_cleanup,
                                  apr_pool_cleanup_null);
    }
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_file_close(apr_file_t *file)
{
    apr_status_t stat;
    if ((stat = file_cleanup(file)) == APR_SUCCESS) {
        apr_pool_cleanup_kill(file->pool, file, file_cleanup);

        if (file->mutex) {
            apr_thread_mutex_destroy(file->mutex);
        }

        return APR_SUCCESS;
    }
    return stat;
}

APR_DECLARE(apr_status_t) apr_file_remove(const char *path, apr_pool_t *pool)
{
#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wpath[APR_PATH_MAX];
        apr_status_t rv;
        if ((rv = utf8_to_unicode_path(wpath, sizeof(wpath) 
                                            / sizeof(apr_wchar_t), path))) {
            return rv;
        }
        if (DeleteFileW(wpath))
            return APR_SUCCESS;
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
        if (DeleteFile(path))
            return APR_SUCCESS;
#endif
    return apr_get_os_error();
}

APR_DECLARE(apr_status_t) apr_file_rename(const char *frompath,
                                          const char *topath,
                                          apr_pool_t *pool)
{
    IF_WIN_OS_IS_UNICODE
    {
#if APR_HAS_UNICODE_FS
        apr_wchar_t wfrompath[APR_PATH_MAX], wtopath[APR_PATH_MAX];
        apr_status_t rv;
        if ((rv = utf8_to_unicode_path(wfrompath,
                                       sizeof(wfrompath) / sizeof(apr_wchar_t),
                                       frompath))) {
            return rv;
        }
        if ((rv = utf8_to_unicode_path(wtopath,
                                       sizeof(wtopath) / sizeof(apr_wchar_t),
                                       topath))) {
            return rv;
        }
#ifndef _WIN32_WCE
        if (MoveFileExW(wfrompath, wtopath, MOVEFILE_REPLACE_EXISTING |
                                            MOVEFILE_COPY_ALLOWED))
#else
        if (MoveFileW(wfrompath, wtopath))
#endif
            return APR_SUCCESS;
#else
        if (MoveFileEx(frompath, topath, MOVEFILE_REPLACE_EXISTING |
                                         MOVEFILE_COPY_ALLOWED))
            return APR_SUCCESS;
#endif
    }
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        /* Windows 95 and 98 do not support MoveFileEx, so we'll use
         * the old MoveFile function.  However, MoveFile requires that
         * the new file not already exist...so we have to delete that
         * file if it does.  Perhaps we should back up the to-be-deleted
         * file in case something happens?
         */
        HANDLE handle = INVALID_HANDLE_VALUE;

        if ((handle = CreateFile(topath, GENERIC_WRITE, 0, 0,  
            OPEN_EXISTING, 0, 0 )) != INVALID_HANDLE_VALUE )
        {
            CloseHandle(handle);
            if (!DeleteFile(topath))
                return apr_get_os_error();
        }
        if (MoveFile(frompath, topath))
            return APR_SUCCESS;
    }        
#endif
    return apr_get_os_error();
}

APR_DECLARE(apr_status_t) apr_file_link(const char *from_path, 
                                           const char *to_path)
{
    apr_status_t rv = APR_SUCCESS;

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        apr_wchar_t wfrom_path[APR_PATH_MAX];
        apr_wchar_t wto_path[APR_PATH_MAX];

        if ((rv = utf8_to_unicode_path(wfrom_path,
                                       sizeof(wfrom_path) / sizeof(apr_wchar_t),
                                       from_path)))
            return rv;
        if ((rv = utf8_to_unicode_path(wto_path,
                                       sizeof(wto_path) / sizeof(apr_wchar_t),
                                       to_path)))
            return rv;

        if (!CreateHardLinkW(wto_path, wfrom_path, NULL))
                return apr_get_os_error();
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI {
        if (!CreateHardLinkA(to_path, from_path, NULL))
                return apr_get_os_error();
    }
#endif
    return rv;
}

APR_DECLARE(apr_status_t) apr_os_file_get(apr_os_file_t *thefile,
                                          apr_file_t *file)
{
    *thefile = file->filehand;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_file_put(apr_file_t **file,
                                          apr_os_file_t *thefile,
                                          apr_int32_t flags,
                                          apr_pool_t *pool)
{
    (*file) = apr_pcalloc(pool, sizeof(apr_file_t));
    (*file)->pool = pool;
    (*file)->filehand = *thefile;
    (*file)->ungetchar = -1; /* no char avail */
    (*file)->timeout = -1;
    (*file)->flags = flags;

    if (flags & APR_FOPEN_APPEND) {
        (*file)->append = 1;
    }
    if (flags & APR_FOPEN_BUFFERED) {
        (*file)->buffered = 1;
        (*file)->buffer = apr_palloc(pool, APR_FILE_DEFAULT_BUFSIZE);
        (*file)->bufsize = APR_FILE_DEFAULT_BUFSIZE;
    }

    if ((*file)->append || (*file)->buffered) {
        apr_status_t rv;
        rv = apr_thread_mutex_create(&(*file)->mutex, 
                                     APR_THREAD_MUTEX_DEFAULT, pool);
        if (rv) {
            return rv;
        }
    }

#if APR_FILES_AS_SOCKETS
    /* Create a pollset with room for one descriptor. */
    /* ### check return codes */
    (void) apr_pollset_create(&(*file)->pollset, 1, pool, 0);
#endif
    /* Should we be testing if thefile is a handle to 
     * a PIPE and set up the mechanics appropriately?
     *
     *  (*file)->pipe;
     */
    return APR_SUCCESS;
}    

APR_DECLARE(apr_status_t) apr_file_eof(apr_file_t *fptr)
{
    if (fptr->eof_hit == 1) {
        return APR_EOF;
    }
    return APR_SUCCESS;
}   

APR_DECLARE(apr_status_t) apr_file_open_flags_stderr(apr_file_t **thefile, 
                                                     apr_int32_t flags, 
                                                     apr_pool_t *pool)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    apr_os_file_t file_handle;

    apr_set_os_error(APR_SUCCESS);
    file_handle = GetStdHandle(STD_ERROR_HANDLE);
    if (!file_handle)
        file_handle = INVALID_HANDLE_VALUE;

    return apr_os_file_put(thefile, &file_handle,
                           flags | APR_FOPEN_WRITE | APR_STDERR_FLAG, pool);
#endif
}

APR_DECLARE(apr_status_t) apr_file_open_flags_stdout(apr_file_t **thefile, 
                                                     apr_int32_t flags,
                                                     apr_pool_t *pool)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    apr_os_file_t file_handle;

    apr_set_os_error(APR_SUCCESS);
    file_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!file_handle)
        file_handle = INVALID_HANDLE_VALUE;

    return apr_os_file_put(thefile, &file_handle,
                           flags | APR_FOPEN_WRITE | APR_STDOUT_FLAG, pool);
#endif
}

APR_DECLARE(apr_status_t) apr_file_open_flags_stdin(apr_file_t **thefile, 
                                                    apr_int32_t flags,
                                                    apr_pool_t *pool)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    apr_os_file_t file_handle;

    apr_set_os_error(APR_SUCCESS);
    file_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (!file_handle)
        file_handle = INVALID_HANDLE_VALUE;

    return apr_os_file_put(thefile, &file_handle,
                           flags | APR_FOPEN_READ | APR_STDIN_FLAG, pool);
#endif
}

APR_DECLARE(apr_status_t) apr_file_open_stderr(apr_file_t **thefile, apr_pool_t *pool)
{
    return apr_file_open_flags_stderr(thefile, 0, pool);
}

APR_DECLARE(apr_status_t) apr_file_open_stdout(apr_file_t **thefile, apr_pool_t *pool)
{
    return apr_file_open_flags_stdout(thefile, 0, pool);
}

APR_DECLARE(apr_status_t) apr_file_open_stdin(apr_file_t **thefile, apr_pool_t *pool)
{
    return apr_file_open_flags_stdin(thefile, 0, pool);
}

APR_POOL_IMPLEMENT_ACCESSOR(file);

APR_IMPLEMENT_INHERIT_SET(file, flags, pool, file_cleanup)
 
APR_IMPLEMENT_INHERIT_UNSET(file, flags, pool, file_cleanup)
