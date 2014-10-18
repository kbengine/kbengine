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
#include "apr_lib.h"
#include "apr_errno.h"
#include <malloc.h>
#include "apr_arch_atime.h"
#include "apr_arch_misc.h"

/*
 * read_with_timeout() 
 * Uses async i/o to emulate unix non-blocking i/o with timeouts.
 */
static apr_status_t read_with_timeout(apr_file_t *file, void *buf, apr_size_t len_in, apr_size_t *nbytes)
{
    apr_status_t rv;
    DWORD res;
    DWORD len = (DWORD)len_in;
    DWORD bytesread = 0;

    /* Handle the zero timeout non-blocking case */
    if (file->timeout == 0) {
        /* Peek at the pipe. If there is no data available, return APR_EAGAIN.
         * If data is available, go ahead and read it.
         */
        if (file->pipe) {
            DWORD bytes;
            if (!PeekNamedPipe(file->filehand, NULL, 0, NULL, &bytes, NULL)) {
                rv = apr_get_os_error();
                if (rv == APR_FROM_OS_ERROR(ERROR_BROKEN_PIPE)) {
                    rv = APR_EOF;
                }
                *nbytes = 0;
                return rv;
            }
            else {
                if (bytes == 0) {
                    *nbytes = 0;
                    return APR_EAGAIN;
                }
                if (len > bytes) {
                    len = bytes;
                }
            }
        }
        else {
            /* ToDo: Handle zero timeout non-blocking file i/o 
             * This is not needed until an APR application needs to
             * timeout file i/o (which means setting file i/o non-blocking)
             */
        }
    }

    if (file->pOverlapped && !file->pipe) {
        file->pOverlapped->Offset     = (DWORD)file->filePtr;
        file->pOverlapped->OffsetHigh = (DWORD)(file->filePtr >> 32);
    }

    if (ReadFile(file->filehand, buf, len, 
                 &bytesread, file->pOverlapped)) {
        rv = APR_SUCCESS;
    }
    else {
        rv = apr_get_os_error();
        if (rv == APR_FROM_OS_ERROR(ERROR_IO_PENDING)) {
            /* Wait for the pending i/o, timeout converted from us to ms
             * Note that we loop if someone gives up the event, since
             * folks suggest that WAIT_ABANDONED isn't actually a result
             * but an alert that ownership of the event has passed from
             * one owner to a new proc/thread.
             */
            do {
                res = WaitForSingleObject(file->pOverlapped->hEvent, 
                                          (file->timeout > 0)
                                            ? (DWORD)(file->timeout/1000)
                                            : ((file->timeout == -1) 
                                                 ? INFINITE : 0));
            } while (res == WAIT_ABANDONED);

            /* There is one case that represents entirely
             * successful operations, otherwise we will cancel
             * the operation in progress.
             */
            if (res != WAIT_OBJECT_0) {
                CancelIo(file->filehand);
            }

            /* Ignore any failures above.  Attempt to complete
             * the overlapped operation and use only _its_ result.
             * For example, CancelIo or WaitForSingleObject can
             * fail if the handle is closed, yet the read may have
             * completed before we attempted to CancelIo...
             */
            if (GetOverlappedResult(file->filehand, file->pOverlapped, 
                                    &bytesread, TRUE)) {
                rv = APR_SUCCESS;
            }
            else {
                rv = apr_get_os_error();
                if (((rv == APR_FROM_OS_ERROR(ERROR_IO_INCOMPLETE))
                        || (rv == APR_FROM_OS_ERROR(ERROR_OPERATION_ABORTED)))
                    && (res == WAIT_TIMEOUT))
                    rv = APR_TIMEUP;
            }
        }
        if (rv == APR_FROM_OS_ERROR(ERROR_BROKEN_PIPE)) {
            /* Assume ERROR_BROKEN_PIPE signals an EOF reading from a pipe */
            rv = APR_EOF;
        } else if (rv == APR_FROM_OS_ERROR(ERROR_HANDLE_EOF)) {
            /* Did we hit EOF reading from the handle? */
            rv = APR_EOF;
        }
    }
    
    /* OK and 0 bytes read ==> end of file */
    if (rv == APR_SUCCESS && bytesread == 0)
        rv = APR_EOF;
    
    if (rv == APR_SUCCESS && file->pOverlapped && !file->pipe) {
        file->filePtr += bytesread;
    }
    *nbytes = bytesread;
    return rv;
}

APR_DECLARE(apr_status_t) apr_file_read(apr_file_t *thefile, void *buf, apr_size_t *len)
{
    apr_status_t rv;
    DWORD bytes_read = 0;

    if (*len <= 0) {
        *len = 0;
        return APR_SUCCESS;
    }

    /* If the file is open for xthread support, allocate and
     * initialize the overlapped and io completion event (hEvent). 
     * Threads should NOT share an apr_file_t or its hEvent.
     */
    if ((thefile->flags & APR_FOPEN_XTHREAD) && !thefile->pOverlapped ) {
        thefile->pOverlapped = (OVERLAPPED*) apr_pcalloc(thefile->pool, 
                                                         sizeof(OVERLAPPED));
        thefile->pOverlapped->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!thefile->pOverlapped->hEvent) {
            rv = apr_get_os_error();
            return rv;
        }
    }

    /* Handle the ungetchar if there is one */
    if (thefile->ungetchar != -1) {
        bytes_read = 1;
        *(char *)buf = (char)thefile->ungetchar;
        buf = (char *)buf + 1;
        (*len)--;
        thefile->ungetchar = -1;
        if (*len == 0) {
            *len = bytes_read;
            return APR_SUCCESS;
        }
    }
    if (thefile->buffered) {
        char *pos = (char *)buf;
        apr_size_t blocksize;
        apr_size_t size = *len;

        if (thefile->flags & APR_FOPEN_XTHREAD) {
            apr_thread_mutex_lock(thefile->mutex);
        }

        if (thefile->direction == 1) {
            rv = apr_file_flush(thefile);
            if (rv != APR_SUCCESS) {
                if (thefile->flags & APR_FOPEN_XTHREAD) {
                    apr_thread_mutex_unlock(thefile->mutex);
                }
                return rv;
            }
            thefile->bufpos = 0;
            thefile->direction = 0;
            thefile->dataRead = 0;
        }

        rv = 0;
        while (rv == 0 && size > 0) {
            if (thefile->bufpos >= thefile->dataRead) {
                apr_size_t read;
                rv = read_with_timeout(thefile, thefile->buffer, 
                                       thefile->bufsize, &read);
                if (read == 0) {
                    if (rv == APR_EOF)
                        thefile->eof_hit = TRUE;
                    break;
                }
                else {
                    thefile->dataRead = read;
                    thefile->filePtr += thefile->dataRead;
                    thefile->bufpos = 0;
                }
            }

            blocksize = size > thefile->dataRead - thefile->bufpos ? thefile->dataRead - thefile->bufpos : size;
            memcpy(pos, thefile->buffer + thefile->bufpos, blocksize);
            thefile->bufpos += blocksize;
            pos += blocksize;
            size -= blocksize;
        }

        *len = pos - (char *)buf;
        if (*len) {
            rv = APR_SUCCESS;
        }

        if (thefile->flags & APR_FOPEN_XTHREAD) {
            apr_thread_mutex_unlock(thefile->mutex);
        }
    } else {  
        /* Unbuffered i/o */
        apr_size_t nbytes;
        rv = read_with_timeout(thefile, buf, *len, &nbytes);
        if (rv == APR_EOF)
            thefile->eof_hit = TRUE;
        *len = nbytes;
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_file_write(apr_file_t *thefile, const void *buf, apr_size_t *nbytes)
{
    apr_status_t rv;
    DWORD bwrote;

    /* If the file is open for xthread support, allocate and
     * initialize the overlapped and io completion event (hEvent). 
     * Threads should NOT share an apr_file_t or its hEvent.
     */
    if ((thefile->flags & APR_FOPEN_XTHREAD) && !thefile->pOverlapped ) {
        thefile->pOverlapped = (OVERLAPPED*) apr_pcalloc(thefile->pool, 
                                                         sizeof(OVERLAPPED));
        thefile->pOverlapped->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (!thefile->pOverlapped->hEvent) {
            rv = apr_get_os_error();
            return rv;
        }
    }

    if (thefile->buffered) {
        char *pos = (char *)buf;
        apr_size_t blocksize;
        apr_size_t size = *nbytes;

        if (thefile->flags & APR_FOPEN_XTHREAD) {
            apr_thread_mutex_lock(thefile->mutex);
        }

        if (thefile->direction == 0) {
            /* Position file pointer for writing at the offset we are logically reading from */
            apr_off_t offset = thefile->filePtr - thefile->dataRead + thefile->bufpos;
            DWORD offlo = (DWORD)offset;
            LONG  offhi = (LONG)(offset >> 32);
            if (offset != thefile->filePtr)
                SetFilePointer(thefile->filehand, offlo, &offhi, FILE_BEGIN);
            thefile->bufpos = thefile->dataRead = 0;
            thefile->direction = 1;
        }

        rv = 0;
        while (rv == 0 && size > 0) {
            if (thefile->bufpos == thefile->bufsize)   /* write buffer is full */
                rv = apr_file_flush(thefile);

            blocksize = size > thefile->bufsize - thefile->bufpos ? 
                                     thefile->bufsize - thefile->bufpos : size;
            memcpy(thefile->buffer + thefile->bufpos, pos, blocksize);
            thefile->bufpos += blocksize;
            pos += blocksize;
            size -= blocksize;
        }

        if (thefile->flags & APR_FOPEN_XTHREAD) {
            apr_thread_mutex_unlock(thefile->mutex);
        }
        return rv;
    } else {
        if (!thefile->pipe) {
            apr_off_t offset = 0;
            apr_status_t rc;
            if (thefile->append) {
                /* apr_file_lock will mutex the file across processes.
                 * The call to apr_thread_mutex_lock is added to avoid
                 * a race condition between LockFile and WriteFile 
                 * that occasionally leads to deadlocked threads.
                 */
                apr_thread_mutex_lock(thefile->mutex);
                rc = apr_file_lock(thefile, APR_FLOCK_EXCLUSIVE);
                if (rc != APR_SUCCESS) {
                    apr_thread_mutex_unlock(thefile->mutex);
                    return rc;
                }
                rc = apr_file_seek(thefile, APR_END, &offset);
                if (rc != APR_SUCCESS) {
                    apr_thread_mutex_unlock(thefile->mutex);
                    return rc;
                }
            }
            if (thefile->pOverlapped) {
                thefile->pOverlapped->Offset     = (DWORD)thefile->filePtr;
                thefile->pOverlapped->OffsetHigh = (DWORD)(thefile->filePtr >> 32);
            }
            rv = WriteFile(thefile->filehand, buf, (DWORD)*nbytes, &bwrote,
                           thefile->pOverlapped);
            if (thefile->append) {
                apr_file_unlock(thefile);
                apr_thread_mutex_unlock(thefile->mutex);
            }
        }
        else {
            rv = WriteFile(thefile->filehand, buf, (DWORD)*nbytes, &bwrote,
                           thefile->pOverlapped);
        }
        if (rv) {
            *nbytes = bwrote;
            rv = APR_SUCCESS;
        }
        else {
            (*nbytes) = 0;
            rv = apr_get_os_error();

            /* XXX: This must be corrected, per the apr_file_read logic!!! */
            if (rv == APR_FROM_OS_ERROR(ERROR_IO_PENDING)) {
 
                DWORD timeout_ms;

                if (thefile->timeout == 0) {
                    timeout_ms = 0;
                }
                else if (thefile->timeout < 0) {
                    timeout_ms = INFINITE;
                }
                else {
                    timeout_ms = (DWORD)(thefile->timeout / 1000);
                }
	       
                rv = WaitForSingleObject(thefile->pOverlapped->hEvent, timeout_ms);
                switch (rv) {
                    case WAIT_OBJECT_0:
                        GetOverlappedResult(thefile->filehand, thefile->pOverlapped, 
                                            &bwrote, TRUE);
                        *nbytes = bwrote;
                        rv = APR_SUCCESS;
                        break;
                    case WAIT_TIMEOUT:
                        rv = (timeout_ms == 0) ? APR_EAGAIN : APR_TIMEUP;
                        break;
                    case WAIT_FAILED:
                        rv = apr_get_os_error();
                        break;
                    default:
                        break;
                }
                if (rv != APR_SUCCESS) {
                    if (apr_os_level >= APR_WIN_98)
                        CancelIo(thefile->filehand);
                }
            }
        }
        if (rv == APR_SUCCESS && thefile->pOverlapped && !thefile->pipe) {
            thefile->filePtr += *nbytes;
        }
    }
    return rv;
}
/* ToDo: Write for it anyway and test the oslevel!
 * Too bad WriteFileGather() is not supported on 95&98 (or NT prior to SP2)
 */
APR_DECLARE(apr_status_t) apr_file_writev(apr_file_t *thefile,
                                     const struct iovec *vec,
                                     apr_size_t nvec, 
                                     apr_size_t *nbytes)
{
    apr_status_t rv = APR_SUCCESS;
    apr_size_t i;
    apr_size_t bwrote = 0;
    char *buf;

    *nbytes = 0;
    for (i = 0; i < nvec; i++) {
        buf = vec[i].iov_base;
        bwrote = vec[i].iov_len;
        rv = apr_file_write(thefile, buf, &bwrote);
        *nbytes += bwrote;
        if (rv != APR_SUCCESS) {
            break;
        }
    }
    return rv;
}

APR_DECLARE(apr_status_t) apr_file_putc(char ch, apr_file_t *thefile)
{
    apr_size_t len = 1;

    return apr_file_write(thefile, &ch, &len);
}

APR_DECLARE(apr_status_t) apr_file_ungetc(char ch, apr_file_t *thefile)
{
    thefile->ungetchar = (unsigned char) ch;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_file_getc(char *ch, apr_file_t *thefile)
{
    apr_status_t rc;
    apr_size_t bread;

    bread = 1;
    rc = apr_file_read(thefile, ch, &bread);

    if (rc) {
        return rc;
    }
    
    if (bread == 0) {
        thefile->eof_hit = TRUE;
        return APR_EOF;
    }
    return APR_SUCCESS; 
}

APR_DECLARE(apr_status_t) apr_file_puts(const char *str, apr_file_t *thefile)
{
    apr_size_t len = strlen(str);

    return apr_file_write(thefile, str, &len);
}

APR_DECLARE(apr_status_t) apr_file_gets(char *str, int len, apr_file_t *thefile)
{
    apr_size_t readlen;
    apr_status_t rv = APR_SUCCESS;
    int i;    

    for (i = 0; i < len-1; i++) {
        readlen = 1;
        rv = apr_file_read(thefile, str+i, &readlen);

        if (rv != APR_SUCCESS && rv != APR_EOF)
            return rv;

        if (readlen == 0) {
            /* If we have bytes, defer APR_EOF to the next call */
            if (i > 0)
                rv = APR_SUCCESS;
            break;
        }
        
        if (str[i] == '\n') {
            i++; /* don't clobber this char below */
            break;
        }
    }
    str[i] = 0;
    return rv;
}

APR_DECLARE(apr_status_t) apr_file_flush(apr_file_t *thefile)
{
    if (thefile->buffered) {
        DWORD numbytes, written = 0;
        apr_status_t rc = 0;
        char *buffer;
        apr_size_t bytesleft;

        if (thefile->direction == 1 && thefile->bufpos) {
            buffer = thefile->buffer;
            bytesleft = thefile->bufpos;           

            do {
                if (bytesleft > APR_DWORD_MAX) {
                    numbytes = APR_DWORD_MAX;
                }
                else {
                    numbytes = (DWORD)bytesleft;
                }

                if (!WriteFile(thefile->filehand, buffer, numbytes, &written, NULL)) {
                    rc = apr_get_os_error();
                    thefile->filePtr += written;
                    break;
                }

                thefile->filePtr += written;
                bytesleft -= written;
                buffer += written;

            } while (bytesleft > 0);

            if (rc == 0)
                thefile->bufpos = 0;
        }

        return rc;
    }

    /* There isn't anything to do if we aren't buffering the output
     * so just return success.
     */
    return APR_SUCCESS; 
}

APR_DECLARE(apr_status_t) apr_file_sync(apr_file_t *thefile){
    apr_status_t rv;

    rv = apr_file_flush(thefile);
    if (rv != APR_SUCCESS) {
        return rv;
    }

    if (!FlushFileBuffers(thefile->filehand)) {
        rv = apr_get_os_error();
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_file_datasync(apr_file_t *thefile){
    return apr_file_sync(thefile);
}

struct apr_file_printf_data {
    apr_vformatter_buff_t vbuff;
    apr_file_t *fptr;
    char *buf;
};

static int file_printf_flush(apr_vformatter_buff_t *buff)
{
    struct apr_file_printf_data *data = (struct apr_file_printf_data *)buff;

    if (apr_file_write_full(data->fptr, data->buf,
                            data->vbuff.curpos - data->buf, NULL)) {
        return -1;
    }

    data->vbuff.curpos = data->buf;
    return 0;
}

APR_DECLARE_NONSTD(int) apr_file_printf(apr_file_t *fptr, 
                                        const char *format, ...)
{
    struct apr_file_printf_data data;
    va_list ap;
    int count;

    data.buf = malloc(HUGE_STRING_LEN);
    if (data.buf == NULL) {
        return 0;
    }
    data.vbuff.curpos = data.buf;
    data.vbuff.endpos = data.buf + HUGE_STRING_LEN;
    data.fptr = fptr;
    va_start(ap, format);
    count = apr_vformatter(file_printf_flush,
                           (apr_vformatter_buff_t *)&data, format, ap);
    /* apr_vformatter does not call flush for the last bits */
    if (count >= 0) file_printf_flush((apr_vformatter_buff_t *)&data);

    va_end(ap);

    free(data.buf);
    return count;
}
