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
#include "fsio.h"
#include "nks/dirio.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_strings.h"
#include "apr_errno.h"
#include "apr_hash.h"
#include "apr_thread_rwlock.h"

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif

#define APR_HAS_PSA

static apr_filetype_e filetype_from_mode(mode_t mode)
{
    apr_filetype_e type = APR_NOFILE;

    if (S_ISREG(mode))
        type = APR_REG;
    else if (S_ISDIR(mode))
        type = APR_DIR;
    else if (S_ISCHR(mode))
        type = APR_CHR;
    else if (S_ISBLK(mode))
        type = APR_BLK;
    else if (S_ISFIFO(mode))
        type = APR_PIPE;
    else if (S_ISLNK(mode))
        type = APR_LNK;
    else if (S_ISSOCK(mode))
        type = APR_SOCK;
    else
        type = APR_UNKFILE;
    return type;
}

static void fill_out_finfo(apr_finfo_t *finfo, struct stat *info,
                           apr_int32_t wanted)
{ 
    finfo->valid = APR_FINFO_MIN | APR_FINFO_IDENT | APR_FINFO_NLINK 
                    | APR_FINFO_OWNER | APR_FINFO_PROT;

    finfo->protection = apr_unix_mode2perms(info->st_mode);
    finfo->filetype = filetype_from_mode(info->st_mode);
    finfo->user = info->st_uid;
    finfo->group = info->st_gid;
    finfo->size = info->st_size;
    finfo->inode = info->st_ino;
    finfo->device = info->st_dev;
    finfo->nlink = info->st_nlink;

    apr_time_ansi_put(&finfo->atime, info->st_atime.tv_sec);
    apr_time_ansi_put(&finfo->mtime, info->st_mtime.tv_sec);
    apr_time_ansi_put(&finfo->ctime, info->st_ctime.tv_sec);

#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
#ifdef DEV_BSIZE
    finfo->csize = (apr_off_t)info->st_blocks * (apr_off_t)DEV_BSIZE;
#else
    finfo->csize = (apr_off_t)info->st_blocks * (apr_off_t)512;
#endif
    finfo->valid |= APR_FINFO_CSIZE;
#endif
}

apr_status_t apr_file_info_get_locked(apr_finfo_t *finfo, apr_int32_t wanted,
                                      apr_file_t *thefile)
{
    struct_stat info;

    if (thefile->buffered) {
        apr_status_t rv = apr_file_flush_locked(thefile);
        if (rv != APR_SUCCESS)
            return rv;
    }

    if (fstat(thefile->filedes, &info) == 0) {
        finfo->pool = thefile->pool;
        finfo->fname = thefile->fname;
        fill_out_finfo(finfo, &info, wanted);
        return (wanted & ~finfo->valid) ? APR_INCOMPLETE : APR_SUCCESS;
    }
    else {
        return errno;
    }
}

APR_DECLARE(apr_status_t) apr_file_info_get(apr_finfo_t *finfo, 
                                            apr_int32_t wanted,
                                            apr_file_t *thefile)
{
    struct stat info;

    if (thefile->buffered) {
        /* XXX: flush here is not mutex protected */
        apr_status_t rv = apr_file_flush(thefile);
        if (rv != APR_SUCCESS)
            return rv;
    }

    if (fstat(thefile->filedes, &info) == 0) {
        finfo->pool = thefile->pool;
        finfo->fname = thefile->fname;
        fill_out_finfo(finfo, &info, wanted);
        return (wanted & ~finfo->valid) ? APR_INCOMPLETE : APR_SUCCESS;
    }
    else {
        return errno;
    }
}

APR_DECLARE(apr_status_t) apr_file_perms_set(const char *fname, 
                                             apr_fileperms_t perms)
{
    mode_t mode = apr_unix_perms2mode(perms);

    if (chmod(fname, mode) == -1)
        return errno;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_file_attrs_set(const char *fname,
                                             apr_fileattrs_t attributes,
                                             apr_fileattrs_t attr_mask,
                                             apr_pool_t *pool)
{
    apr_status_t status;
    apr_finfo_t finfo;

    /* Don't do anything if we can't handle the requested attributes */
    if (!(attr_mask & (APR_FILE_ATTR_READONLY
                       | APR_FILE_ATTR_EXECUTABLE)))
        return APR_SUCCESS;

    status = apr_stat(&finfo, fname, APR_FINFO_PROT, pool);
    if (status)
        return status;

    /* ### TODO: should added bits be umask'd? */
    if (attr_mask & APR_FILE_ATTR_READONLY)
    {
        if (attributes & APR_FILE_ATTR_READONLY)
        {
            finfo.protection &= ~APR_UWRITE;
            finfo.protection &= ~APR_GWRITE;
            finfo.protection &= ~APR_WWRITE;
        }
        else
        {
            /* ### umask this! */
            finfo.protection |= APR_UWRITE;
            finfo.protection |= APR_GWRITE;
            finfo.protection |= APR_WWRITE;
        }
    }

    if (attr_mask & APR_FILE_ATTR_EXECUTABLE)
    {
        if (attributes & APR_FILE_ATTR_EXECUTABLE)
        {
            /* ### umask this! */
            finfo.protection |= APR_UEXECUTE;
            finfo.protection |= APR_GEXECUTE;
            finfo.protection |= APR_WEXECUTE;
        }
        else
        {
            finfo.protection &= ~APR_UEXECUTE;
            finfo.protection &= ~APR_GEXECUTE;
            finfo.protection &= ~APR_WEXECUTE;
        }
    }

    return apr_file_perms_set(fname, finfo.protection);
}

#ifndef APR_HAS_PSA
static apr_status_t stat_cache_cleanup(void *data)
{
    apr_pool_t *p = (apr_pool_t *)getGlobalPool();
    apr_hash_index_t *hi;
    apr_hash_t *statCache = (apr_hash_t*)data;
	char *key;
    apr_ssize_t keylen;
    NXPathCtx_t pathctx;

    for (hi = apr_hash_first(p, statCache); hi; hi = apr_hash_next(hi)) {
        apr_hash_this(hi, (const void**)&key, &keylen, (void**)&pathctx);

        if (pathctx) {
            NXFreePathContext(pathctx);
        }
    }

    return APR_SUCCESS;
}

int cstat (NXPathCtx_t ctx, char *path, struct stat *buf, unsigned long requestmap, apr_pool_t *p)
{
    apr_pool_t *gPool = (apr_pool_t *)getGlobalPool();
    apr_hash_t *statCache = NULL;
    apr_thread_rwlock_t *rwlock = NULL;

    NXPathCtx_t pathctx = 0;
    char *ptr = NULL, *tr;
    int len = 0, x;
    char *ppath;
    char *pinfo;

    if (ctx == 1) {

        /* If there isn't a global pool then just stat the file
           and return */
        if (!gPool) {
            char poolname[50];
    
            if (apr_pool_create(&gPool, NULL) != APR_SUCCESS) {
                return getstat(ctx, path, buf, requestmap);
            }
    
            setGlobalPool(gPool);
            apr_pool_tag(gPool, apr_pstrdup(gPool, "cstat_mem_pool"));
    
            statCache = apr_hash_make(gPool);
            apr_pool_userdata_set ((void*)statCache, "STAT_CACHE", stat_cache_cleanup, gPool);

            apr_thread_rwlock_create(&rwlock, gPool);
            apr_pool_userdata_set ((void*)rwlock, "STAT_CACHE_LOCK", apr_pool_cleanup_null, gPool);
        }
        else {
            apr_pool_userdata_get((void**)&statCache, "STAT_CACHE", gPool);
            apr_pool_userdata_get((void**)&rwlock, "STAT_CACHE_LOCK", gPool);
        }

        if (!gPool || !statCache || !rwlock) {
            return getstat(ctx, path, buf, requestmap);
        }
    
        for (x = 0,tr = path;*tr != '\0';tr++,x++) {
            if (*tr == '\\' || *tr == '/') {
                ptr = tr;
                len = x;
            }
            if (*tr == ':') {
                ptr = "\\";
                len = x;
            }
        }
    
        if (ptr) {
            ppath = apr_pstrndup (p, path, len);
            strlwr(ppath);
            if (ptr[1] != '\0') {
                ptr++;
            }
            /* If the path ended in a trailing slash then our result path
               will be a single slash. To avoid stat'ing the root with a
               slash, we need to make sure we stat the current directory
               with a dot */
            if (((*ptr == '/') || (*ptr == '\\')) && (*(ptr+1) == '\0')) {
                pinfo = apr_pstrdup (p, ".");
            }
            else {
                pinfo = apr_pstrdup (p, ptr);
            }
        }
    
        /* If we have a statCache then try to pull the information
           from the cache.  Otherwise just stat the file and return.*/
        if (statCache) {
            apr_thread_rwlock_rdlock(rwlock);
            pathctx = (NXPathCtx_t) apr_hash_get(statCache, ppath, APR_HASH_KEY_STRING);
            apr_thread_rwlock_unlock(rwlock);
            if (pathctx) {
                return getstat(pathctx, pinfo, buf, requestmap);
            }
            else {
                int err;

                err = NXCreatePathContext(0, ppath, 0, NULL, &pathctx);
                if (!err) {
                    apr_thread_rwlock_wrlock(rwlock);
                    apr_hash_set(statCache, apr_pstrdup(gPool,ppath) , APR_HASH_KEY_STRING, (void*)pathctx);
                    apr_thread_rwlock_unlock(rwlock);
                    return getstat(pathctx, pinfo, buf, requestmap);
                }
            }
        }
    }
    return getstat(ctx, path, buf, requestmap);
}
#endif

APR_DECLARE(apr_status_t) apr_stat(apr_finfo_t *finfo, 
                                   const char *fname, 
                                   apr_int32_t wanted, apr_pool_t *pool)
{
    struct stat info;
    int srv;
    NXPathCtx_t pathCtx = 0;

    getcwdpath(NULL, &pathCtx, CTX_ACTUAL_CWD);
#ifdef APR_HAS_PSA
	srv = getstat(pathCtx, (char*)fname, &info, ST_STAT_BITS|ST_NAME_BIT);
#else
    srv = cstat(pathCtx, (char*)fname, &info, ST_STAT_BITS|ST_NAME_BIT, pool);
#endif
    errno = srv;

    if (srv == 0) {
        finfo->pool = pool;
        finfo->fname = fname;
        fill_out_finfo(finfo, &info, wanted);
        if (wanted & APR_FINFO_LINK)
            wanted &= ~APR_FINFO_LINK;
        if (wanted & APR_FINFO_NAME) {
            finfo->name = apr_pstrdup(pool, info.st_name);
            finfo->valid |= APR_FINFO_NAME;
        }
        return (wanted & ~finfo->valid) ? APR_INCOMPLETE : APR_SUCCESS;
    }
    else {
#if !defined(ENOENT) || !defined(ENOTDIR)
#error ENOENT || ENOTDIR not defined; please see the
#error comments at this line in the source for a workaround.
        /*
         * If ENOENT || ENOTDIR is not defined in one of the your OS's
         * include files, APR cannot report a good reason why the stat()
         * of the file failed; there are cases where it can fail even though
         * the file exists.  This opens holes in Apache, for example, because
         * it becomes possible for someone to get a directory listing of a 
         * directory even though there is an index (eg. index.html) file in 
         * it.  If you do not have a problem with this, delete the above 
         * #error lines and start the compile again.  If you need to do this,
         * please submit a bug report to http://www.apache.org/bug_report.html
         * letting us know that you needed to do this.  Please be sure to 
         * include the operating system you are using.
         */
        /* WARNING: All errors will be handled as not found
         */
#if !defined(ENOENT) 
        return APR_ENOENT;
#else
        /* WARNING: All errors but not found will be handled as not directory
         */
        if (errno != ENOENT)
            return APR_ENOENT;
        else
            return errno;
#endif
#else /* All was defined well, report the usual: */
        return errno;
#endif
    }
}

APR_DECLARE(apr_status_t) apr_file_mtime_set(const char *fname,
                                              apr_time_t mtime,
                                              apr_pool_t *pool)
{
    apr_status_t status;
    apr_finfo_t finfo;

    status = apr_stat(&finfo, fname, APR_FINFO_ATIME, pool);
    if (status) {
        return status;
    }

#ifdef HAVE_UTIMES
    {
      struct timeval tvp[2];
    
      tvp[0].tv_sec = apr_time_sec(finfo.atime);
      tvp[0].tv_usec = apr_time_usec(finfo.atime);
      tvp[1].tv_sec = apr_time_sec(mtime);
      tvp[1].tv_usec = apr_time_usec(mtime);
      
      if (utimes(fname, tvp) == -1) {
        return errno;
      }
    }
#elif defined(HAVE_UTIME)
    {
      struct utimbuf buf;
      
      buf.actime = (time_t) (finfo.atime / APR_USEC_PER_SEC);
      buf.modtime = (time_t) (mtime / APR_USEC_PER_SEC);
      
      if (utime(fname, &buf) == -1) {
        return errno;
      }
    }
#else
    return APR_ENOTIMPL;
#endif

    return APR_SUCCESS;
}
