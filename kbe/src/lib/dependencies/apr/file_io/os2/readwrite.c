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

#define INCL_DOS
#define INCL_DOSERRORS

#include "apr_arch_file_io.h"
#include "apr_file_io.h"
#include "apr_lib.h"
#include "apr_strings.h"

#include <malloc.h>

APR_DECLARE(apr_status_t) apr_file_read(apr_file_t *thefile, void *buf, apr_size_t *nbytes)
{
    ULONG rc = 0;
    ULONG bytesread;

    if (!thefile->isopen) {
        *nbytes = 0;
        return APR_EBADF;
    }

    if (thefile->buffered) {
        char *pos = (char *)buf;
        ULONG blocksize;
        ULONG size = *nbytes;

        apr_thread_mutex_lock(thefile->mutex);

        if (thefile->direction == 1) {
            int rv = apr_file_flush(thefile);

            if (rv != APR_SUCCESS) {
                apr_thread_mutex_unlock(thefile->mutex);
                return rv;
            }

            thefile->bufpos = 0;
            thefile->direction = 0;
            thefile->dataRead = 0;
        }

        while (rc == 0 && size > 0) {
            if (thefile->bufpos >= thefile->dataRead) {
                ULONG bytesread;
                rc = DosRead(thefile->filedes, thefile->buffer,
                             thefile->bufsize, &bytesread);

                if (bytesread == 0) {
                    if (rc == 0)
                        thefile->eof_hit = TRUE;
                    break;
                }

                thefile->dataRead = bytesread;
                thefile->filePtr += thefile->dataRead;
                thefile->bufpos = 0;
            }

            blocksize = size > thefile->dataRead - thefile->bufpos ? thefile->dataRead - thefile->bufpos : size;
            memcpy(pos, thefile->buffer + thefile->bufpos, blocksize);
            thefile->bufpos += blocksize;
            pos += blocksize;
            size -= blocksize;
        }

        *nbytes = rc == 0 ? pos - (char *)buf : 0;
        apr_thread_mutex_unlock(thefile->mutex);

        if (*nbytes == 0 && rc == 0 && thefile->eof_hit) {
            return APR_EOF;
        }

        return APR_FROM_OS_ERROR(rc);
    } else {
        if (thefile->pipe)
            DosResetEventSem(thefile->pipeSem, &rc);

        rc = DosRead(thefile->filedes, buf, *nbytes, &bytesread);

        if (rc == ERROR_NO_DATA && thefile->timeout != 0) {
            int rcwait = DosWaitEventSem(thefile->pipeSem, thefile->timeout >= 0 ? thefile->timeout / 1000 : SEM_INDEFINITE_WAIT);

            if (rcwait == 0) {
                rc = DosRead(thefile->filedes, buf, *nbytes, &bytesread);
            }
            else if (rcwait == ERROR_TIMEOUT) {
                *nbytes = 0;
                return APR_TIMEUP;
            }
        }

        if (rc) {
            *nbytes = 0;
            return APR_FROM_OS_ERROR(rc);
        }

        *nbytes = bytesread;
        
        if (bytesread == 0) {
            thefile->eof_hit = TRUE;
            return APR_EOF;
        }

        return APR_SUCCESS;
    }
}



APR_DECLARE(apr_status_t) apr_file_write(apr_file_t *thefile, const void *buf, apr_size_t *nbytes)
{
    ULONG rc = 0;
    ULONG byteswritten;

    if (!thefile->isopen) {
        *nbytes = 0;
        return APR_EBADF;
    }

    if (thefile->buffered) {
        char *pos = (char *)buf;
        int blocksize;
        int size = *nbytes;

        apr_thread_mutex_lock(thefile->mutex);

        if ( thefile->direction == 0 ) {
            /* Position file pointer for writing at the offset we are logically reading from */
            ULONG offset = thefile->filePtr - thefile->dataRead + thefile->bufpos;
            if (offset != thefile->filePtr)
                DosSetFilePtr(thefile->filedes, offset, FILE_BEGIN, &thefile->filePtr );
            thefile->bufpos = thefile->dataRead = 0;
            thefile->direction = 1;
        }

        while (rc == 0 && size > 0) {
            if (thefile->bufpos == thefile->bufsize) /* write buffer is full */
                /* XXX bug; - rc is double-transformed os->apr below */
                rc = apr_file_flush(thefile);

            blocksize = size > thefile->bufsize - thefile->bufpos ? thefile->bufsize - thefile->bufpos : size;
            memcpy(thefile->buffer + thefile->bufpos, pos, blocksize);
            thefile->bufpos += blocksize;
            pos += blocksize;
            size -= blocksize;
        }

        apr_thread_mutex_unlock(thefile->mutex);
        return APR_FROM_OS_ERROR(rc);
    } else {
        if (thefile->flags & APR_FOPEN_APPEND) {
            FILELOCK all = { 0, 0x7fffffff };
            ULONG newpos;
            rc = DosSetFileLocks(thefile->filedes, NULL, &all, -1, 0);

            if (rc == 0) {
                rc = DosSetFilePtr(thefile->filedes, 0, FILE_END, &newpos);

                if (rc == 0) {
                    rc = DosWrite(thefile->filedes, buf, *nbytes, &byteswritten);
                }

                DosSetFileLocks(thefile->filedes, &all, NULL, -1, 0);
            }
        } else {
            rc = DosWrite(thefile->filedes, buf, *nbytes, &byteswritten);
        }

        if (rc) {
            *nbytes = 0;
            return APR_FROM_OS_ERROR(rc);
        }

        *nbytes = byteswritten;
        return APR_SUCCESS;
    }
}



#ifdef HAVE_WRITEV

APR_DECLARE(apr_status_t) apr_file_writev(apr_file_t *thefile, const struct iovec *vec, apr_size_t nvec, apr_size_t *nbytes)
{
    int bytes;

    if (thefile->buffered) {
        apr_status_t rv = apr_file_flush(thefile);
        if (rv != APR_SUCCESS) {
            return rv;
        }
    }

    if ((bytes = writev(thefile->filedes, vec, nvec)) < 0) {
        *nbytes = 0;
        return errno;
    }
    else {
        *nbytes = bytes;
        return APR_SUCCESS;
    }
}
#endif



APR_DECLARE(apr_status_t) apr_file_putc(char ch, apr_file_t *thefile)
{
    ULONG rc;
    ULONG byteswritten;

    if (!thefile->isopen) {
        return APR_EBADF;
    }

    rc = DosWrite(thefile->filedes, &ch, 1, &byteswritten);

    if (rc) {
        return APR_FROM_OS_ERROR(rc);
    }
    
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_file_ungetc(char ch, apr_file_t *thefile)
{
    apr_off_t offset = -1;
    return apr_file_seek(thefile, APR_CUR, &offset);
}


APR_DECLARE(apr_status_t) apr_file_getc(char *ch, apr_file_t *thefile)
{
    ULONG rc;
    apr_size_t bytesread;

    if (!thefile->isopen) {
        return APR_EBADF;
    }

    bytesread = 1;
    rc = apr_file_read(thefile, ch, &bytesread);

    if (rc) {
        return rc;
    }
    
    if (bytesread == 0) {
        thefile->eof_hit = TRUE;
        return APR_EOF;
    }
    
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_file_puts(const char *str, apr_file_t *thefile)
{
    apr_size_t len;

    len = strlen(str);
    return apr_file_write(thefile, str, &len); 
}


APR_DECLARE(apr_status_t) apr_file_flush(apr_file_t *thefile)
{
    if (thefile->buffered) {
        ULONG written = 0;
        int rc = 0;

        if (thefile->direction == 1 && thefile->bufpos) {
            rc = DosWrite(thefile->filedes, thefile->buffer, thefile->bufpos, &written);
            thefile->filePtr += written;

            if (rc == 0)
                thefile->bufpos = 0;
        }

        return APR_FROM_OS_ERROR(rc);
    } else {
        /* There isn't anything to do if we aren't buffering the output
         * so just return success.
         */
        return APR_SUCCESS;
    }
}

APR_DECLARE(apr_status_t) apr_file_sync(apr_file_t *thefile)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_file_datasync(apr_file_t *thefile)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_file_gets(char *str, int len, apr_file_t *thefile)
{
    apr_size_t readlen;
    apr_status_t rv = APR_SUCCESS;
    int i;    

    for (i = 0; i < len-1; i++) {
        readlen = 1;
        rv = apr_file_read(thefile, str+i, &readlen);

        if (rv != APR_SUCCESS) {
            break;
        }

        if (readlen != 1) {
            rv = APR_EOF;
            break;
        }
        
        if (str[i] == '\n') {
            i++;
            break;
        }
    }
    str[i] = 0;
    if (i > 0) {
        /* we stored chars; don't report EOF or any other errors;
         * the app will find out about that on the next call
         */
        return APR_SUCCESS;
    }
    return rv;
}



APR_DECLARE_NONSTD(int) apr_file_printf(apr_file_t *fptr, 
                                        const char *format, ...)
{
    int cc;
    va_list ap;
    char *buf;
    int len;

    buf = malloc(HUGE_STRING_LEN);
    if (buf == NULL) {
        return 0;
    }
    va_start(ap, format);
    len = apr_vsnprintf(buf, HUGE_STRING_LEN, format, ap);
    cc = apr_file_puts(buf, fptr);
    va_end(ap);
    free(buf);
    return (cc == APR_SUCCESS) ? len : -1;
}



apr_status_t apr_file_check_read(apr_file_t *fd)
{
    int rc;

    if (!fd->pipe)
        return APR_SUCCESS; /* Not a pipe, assume no waiting */

    rc = DosWaitEventSem(fd->pipeSem, SEM_IMMEDIATE_RETURN);

    if (rc == ERROR_TIMEOUT)
        return APR_TIMEUP;

    return APR_FROM_OS_ERROR(rc);
}
