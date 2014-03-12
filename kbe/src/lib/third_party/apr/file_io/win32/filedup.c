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

#include "apr_arch_file_io.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_strings.h"
#include <string.h>
#include "apr_arch_inherit.h"
#include <io.h> /* for [_open/_get]_osfhandle */


APR_DECLARE(apr_status_t) apr_file_dup(apr_file_t **new_file,
                                       apr_file_t *old_file, apr_pool_t *p)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    HANDLE hproc = GetCurrentProcess();
    HANDLE newhand = NULL;

    if (!DuplicateHandle(hproc, old_file->filehand, 
                         hproc, &newhand, 0, FALSE, 
                         DUPLICATE_SAME_ACCESS)) {
        return apr_get_os_error();
    }

    (*new_file) = (apr_file_t *) apr_pcalloc(p, sizeof(apr_file_t));
    (*new_file)->filehand = newhand;
    (*new_file)->flags = old_file->flags & ~(APR_STD_FLAGS | APR_INHERIT);
    (*new_file)->pool = p;
    (*new_file)->fname = apr_pstrdup(p, old_file->fname);
    (*new_file)->append = old_file->append;
    (*new_file)->buffered = FALSE;
    (*new_file)->ungetchar = old_file->ungetchar;

#if APR_HAS_THREADS
    if (old_file->mutex) {
        apr_thread_mutex_create(&((*new_file)->mutex),
                                APR_THREAD_MUTEX_DEFAULT, p);
    }
#endif

    apr_pool_cleanup_register((*new_file)->pool, (void *)(*new_file), file_cleanup,
                        apr_pool_cleanup_null);

#if APR_FILES_AS_SOCKETS
    /* Create a pollset with room for one descriptor. */
    /* ### check return codes */
    (void) apr_pollset_create(&(*new_file)->pollset, 1, p, 0);
#endif
    return APR_SUCCESS;
#endif /* !defined(_WIN32_WCE) */
}

APR_DECLARE(apr_status_t) apr_file_dup2(apr_file_t *new_file,
                                        apr_file_t *old_file, apr_pool_t *p)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    HANDLE hproc = GetCurrentProcess();
    HANDLE newhand = NULL;
    apr_int32_t newflags;
    int fd;

    if (new_file->flags & APR_STD_FLAGS)
    {
        if ((new_file->flags & APR_STD_FLAGS) == APR_STDERR_FLAG)
        {
            /* Flush stderr and unset its buffer, then commit the fd-based buffer.
             * This is typically a noop for Win2K/XP since services with NULL std
             * handles [but valid FILE *'s, oddly enough], but is required
             * for NT 4.0 and to use this code outside of services.
             */
            fflush(stderr);
            setvbuf(stderr, NULL, _IONBF, 0);
            if (!_isatty(2)) {
                _commit(2 /* stderr */);
            }

            /* Clone a handle can _close() without harming the source handle,
             * open an MSVCRT-based pseudo-fd for the file handle, then dup2
             * and close our temporary pseudo-fd once it's been duplicated.
             * This will incidently keep the FILE-based stderr in sync.
             * Note the apparently redundant _O_BINARY coersions are required.
             * Note the _dup2 will close the previous std Win32 handle.
             */
            if (!DuplicateHandle(hproc, old_file->filehand, hproc, &newhand,
                                 0, FALSE, DUPLICATE_SAME_ACCESS)) {
                return apr_get_os_error();
            }
            fd = _open_osfhandle((INT_PTR)newhand, _O_WRONLY | _O_BINARY);
            _dup2(fd, 2);
            _close(fd);
            _setmode(2, _O_BINARY);

            /* hPipeWrite was _close()'ed above, and _dup2()'ed
             * to fd 2 creating a new, inherited Win32 handle.
             * Recover that real handle from fd 2.  Note that
             * SetStdHandle(STD_ERROR_HANDLE, _get_osfhandle(2))
             * is implicit in the dup2() call above
             */
            newhand = (HANDLE)_get_osfhandle(2);
        }
        else if ((new_file->flags & APR_STD_FLAGS) == APR_STDOUT_FLAG) {
            /* For the process flow see the stderr case above */
            fflush(stdout);
            setvbuf(stdout, NULL, _IONBF, 0);
            if (!_isatty(1)) {
                _commit(1 /* stdout */);
            }

            if (!DuplicateHandle(hproc, old_file->filehand, hproc, &newhand,
                                 0, FALSE, DUPLICATE_SAME_ACCESS)) {
                return apr_get_os_error();
            }
            fd = _open_osfhandle((INT_PTR)newhand, _O_WRONLY | _O_BINARY);
            _dup2(fd, 1);
            _close(fd);
            _setmode(1, _O_BINARY);
            newhand = (HANDLE)_get_osfhandle(1);
        }
        else if ((new_file->flags & APR_STD_FLAGS) == APR_STDIN_FLAG) {
            /* For the process flow see the stderr case above */
            fflush(stdin);
            setvbuf(stdin, NULL, _IONBF, 0);

            if (!DuplicateHandle(hproc, old_file->filehand, hproc, &newhand,
                                 0, FALSE, DUPLICATE_SAME_ACCESS)) {
                return apr_get_os_error();
            }
            fd = _open_osfhandle((INT_PTR)newhand, _O_RDONLY | _O_BINARY);
            _dup2(fd, 0);
            _close(fd);
            _setmode(0, _O_BINARY);
            newhand = (HANDLE)_get_osfhandle(0);
        }
        newflags = (new_file->flags & APR_STD_FLAGS) 
                 | (old_file->flags & ~APR_STD_FLAGS) | APR_INHERIT;

        /* No need  to close the old file, _dup2() above did that for us */
    }
    else {
        if (!DuplicateHandle(hproc, old_file->filehand, 
                             hproc, &newhand, 0,
                             FALSE, DUPLICATE_SAME_ACCESS)) {
            return apr_get_os_error();
        }
        newflags = old_file->flags & ~(APR_STD_FLAGS | APR_INHERIT);

        if (new_file->filehand
                && (new_file->filehand != INVALID_HANDLE_VALUE)) {
            CloseHandle(new_file->filehand);
        }
    }

    new_file->flags = newflags;
    new_file->filehand = newhand;
    new_file->fname = apr_pstrdup(new_file->pool, old_file->fname);
    new_file->append = old_file->append;
    new_file->buffered = FALSE;
    new_file->ungetchar = old_file->ungetchar;

#if APR_HAS_THREADS
    if (old_file->mutex) {
        apr_thread_mutex_create(&(new_file->mutex),
                                APR_THREAD_MUTEX_DEFAULT, p);
    }
#endif

    return APR_SUCCESS;
#endif /* !defined(_WIN32_WCE) */
}

APR_DECLARE(apr_status_t) apr_file_setaside(apr_file_t **new_file,
                                            apr_file_t *old_file,
                                            apr_pool_t *p)
{
    *new_file = (apr_file_t *)apr_pmemdup(p, old_file, sizeof(apr_file_t));
    (*new_file)->pool = p;
    if (old_file->buffered) {
        (*new_file)->buffer = apr_palloc(p, old_file->bufsize);
        (*new_file)->bufsize = old_file->bufsize;
        if (old_file->direction == 1) {
            memcpy((*new_file)->buffer, old_file->buffer, old_file->bufpos);
        }
        else {
            memcpy((*new_file)->buffer, old_file->buffer, old_file->dataRead);
        }
    }
    if (old_file->mutex) {
        apr_thread_mutex_create(&((*new_file)->mutex),
                                APR_THREAD_MUTEX_DEFAULT, p);
        apr_thread_mutex_destroy(old_file->mutex);
    }
    if (old_file->fname) {
        (*new_file)->fname = apr_pstrdup(p, old_file->fname);
    }
    if (!(old_file->flags & APR_FOPEN_NOCLEANUP)) {
        apr_pool_cleanup_register(p, (void *)(*new_file), 
                                  file_cleanup,
                                  file_cleanup);
    }

    old_file->filehand = INVALID_HANDLE_VALUE;
    apr_pool_cleanup_kill(old_file->pool, (void *)old_file,
                          file_cleanup);

#if APR_FILES_AS_SOCKETS
    /* Create a pollset with room for one descriptor. */
    /* ### check return codes */
    (void) apr_pollset_create(&(*new_file)->pollset, 1, p, 0);
#endif
    return APR_SUCCESS;
}
