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

#include "apr_general.h"
#include "apr_errno.h"
#include "apr_file_io.h"
#include "apr_shm.h"
#include "apr_strings.h"
#include "apr_arch_file_io.h"
#include "limits.h"

typedef struct memblock_t {
    apr_size_t size;
    apr_size_t length;
} memblock_t;

struct apr_shm_t {
    apr_pool_t *pool;
    memblock_t *memblk;
    void       *usrmem;
    apr_size_t  size;
    apr_size_t  length;
    HANDLE      hMap;
    const char *filename;
};

static apr_status_t shm_cleanup(void* shm)
{
    apr_status_t rv = APR_SUCCESS;
    apr_shm_t *m = shm;
    
    if (!UnmapViewOfFile(m->memblk)) {
        rv = apr_get_os_error();
    }
    if (!CloseHandle(m->hMap)) {
        rv = rv != APR_SUCCESS ? rv : apr_get_os_error();
    }
    if (m->filename) {
        /* Remove file if file backed */
        apr_status_t rc = apr_file_remove(m->filename, m->pool);
        rv = rv != APR_SUCCESS ? rv : rc;
    }
    return rv;
}

/* See if the caller is able to create a map in the global namespace by
 * checking if the SE_CREATE_GLOBAL_NAME privilege is enabled.
 *
 * Prior to APR 1.5.0, named shared memory segments were always created
 * in the global segment.  However, with recent versions of Windows this
 * fails for unprivileged processes.  Thus, with older APR, named shared
 * memory segments can't be created by unprivileged processes on newer
 * Windows.
 *
 * By checking if the caller has the privilege, shm APIs can decide
 * whether to use the Global or Local namespace.
 *
 * If running on an SDK without the required API definitions *OR*
 * some processing failure occurs trying to check the privilege, fall
 * back to earlier behavior -- always try to use the Global namespace.
 */
#ifdef SE_CREATE_GLOBAL_NAME
static int can_create_global_maps(void)
{
    BOOL ok, has_priv;
    LUID priv_id;
    PRIVILEGE_SET privs;
    HANDLE hToken;

    ok = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
    if (!ok && GetLastError() == ERROR_NO_TOKEN) {
        /* no thread-specific access token, so try to get process access token
         */
        ok = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken);
    }

    if (ok) {
        ok = LookupPrivilegeValue(NULL, SE_CREATE_GLOBAL_NAME, &priv_id);
    }

    if (ok) {
        privs.PrivilegeCount = 1;
        privs.Control = PRIVILEGE_SET_ALL_NECESSARY;
        privs.Privilege[0].Luid = priv_id;
        privs.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
        ok = PrivilegeCheck(hToken, &privs, &has_priv);
    }

    if (ok && !has_priv) {
        return 0;
    }
    else {
        return 1;
    }
}
#else /* SE_CREATE_GLOBAL_NAME */
/* SDK definitions missing */
static int can_create_global_maps(void)
{
    return 1;
}
#endif /* SE_CREATE_GLOBAL_NAME */

APR_DECLARE(apr_status_t) apr_shm_create_ex(apr_shm_t **m,
                                            apr_size_t reqsize,
                                            const char *file,
                                            apr_pool_t *pool,
                                            apr_int32_t flags)
{
    static apr_size_t memblock = 0;
    HANDLE hMap, hFile;
    apr_status_t rv;
    apr_size_t size;
    apr_file_t *f;
    void *base;
    void *mapkey;
    DWORD err, sizelo, sizehi;

    reqsize += sizeof(memblock_t);

    if (!memblock)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        memblock = si.dwAllocationGranularity;
    }   

    /* Compute the granualar multiple of the pagesize */
    size = memblock * (1 + (reqsize - 1) / memblock);
    sizelo = (DWORD)size;
#ifdef _WIN64
    sizehi = (DWORD)(size >> 32);
#else
    sizehi = 0;
#endif

    if (!file) {
        /* Do Anonymous, which must be passed as a duplicated handle */
#ifndef _WIN32_WCE
        hFile = INVALID_HANDLE_VALUE;
#endif
        mapkey = NULL;
    }
    else {
        int global;

        /* Do file backed, which is not an inherited handle 
         * While we could open APR_EXCL, it doesn't seem that Unix
         * ever did.  Ignore that error here, but fail later when
         * we discover we aren't the creator of the file map object.
         */
        rv = apr_file_open(&f, file,
                           APR_READ | APR_WRITE | APR_BINARY | APR_CREATE,
                           APR_UREAD | APR_UWRITE, pool);
        if ((rv != APR_SUCCESS)
                || ((rv = apr_os_file_get(&hFile, f)) != APR_SUCCESS)) {
            return rv;
        }
        rv = apr_file_trunc(f, size);

        /* res_name_from_filename turns file into a pseudo-name
         * without slashes or backslashes, and prepends the \global
         * or \local prefix on Win2K and later
         */
        if (flags & APR_SHM_NS_GLOBAL) {
            global = 1;
        }
        else if (flags & APR_SHM_NS_LOCAL) {
            global = 0;
        }
        else {
            global = can_create_global_maps();
        }
        mapkey = res_name_from_filename(file, global, pool);
    }

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
        hMap = CreateFileMappingW(hFile, NULL, PAGE_READWRITE, 
                                  sizehi, sizelo, mapkey);
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 
                                  sizehi, sizelo, mapkey);
    }
#endif
    err = apr_get_os_error();

    if (file) {
        apr_file_close(f);
    }

    if (hMap && APR_STATUS_IS_EEXIST(err)) {
        CloseHandle(hMap);
        return APR_EEXIST;
    }
    if (!hMap) {
        return err;
    }
    
    base = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE,
                         0, 0, size);
    if (!base) {
        CloseHandle(hMap);
        return apr_get_os_error();
    }
    
    *m = (apr_shm_t *) apr_palloc(pool, sizeof(apr_shm_t));
    (*m)->pool = pool;
    (*m)->hMap = hMap;
    (*m)->memblk = base;
    (*m)->size = size;

    (*m)->usrmem = (char*)base + sizeof(memblock_t);
    (*m)->length = reqsize - sizeof(memblock_t);;
    
    (*m)->memblk->length = (*m)->length;
    (*m)->memblk->size = (*m)->size;
    (*m)->filename = file ? apr_pstrdup(pool, file) : NULL;

    apr_pool_cleanup_register((*m)->pool, *m, 
                              shm_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_create(apr_shm_t **m,
                                         apr_size_t reqsize,
                                         const char *file,
                                         apr_pool_t *pool)
{
    return apr_shm_create_ex(m, reqsize, file, pool, 0);
}

APR_DECLARE(apr_status_t) apr_shm_destroy(apr_shm_t *m) 
{
    apr_status_t rv = shm_cleanup(m);
    apr_pool_cleanup_kill(m->pool, m, shm_cleanup);
    return rv;
}

APR_DECLARE(apr_status_t) apr_shm_remove(const char *filename,
                                         apr_pool_t *pool)
{
    return apr_file_remove(filename, pool);
}

static apr_status_t shm_attach_internal(apr_shm_t **m,
                                        const char *file,
                                        apr_pool_t *pool,
                                        int global)
{
    HANDLE hMap;
    void *mapkey;
    void *base;

    /* res_name_from_filename turns file into a pseudo-name
     * without slashes or backslashes, and prepends the \global
     * or local prefix on Win2K and later
     */
    mapkey = res_name_from_filename(file, global, pool);

#if APR_HAS_UNICODE_FS
    IF_WIN_OS_IS_UNICODE
    {
#ifndef _WIN32_WCE
        hMap = OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapkey);
#else
        /* The WCE 3.0 lacks OpenFileMapping. So we emulate one with
         * opening the existing shmem and reading its size from the header 
         */
        hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, 
                                  PAGE_READWRITE, 0, sizeof(apr_shm_t), mapkey);
#endif
    }
#endif
#if APR_HAS_ANSI_FS
    ELSE_WIN_OS_IS_ANSI
    {
        hMap = OpenFileMappingA(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapkey);
    }
#endif

    if (!hMap) {
        return apr_get_os_error();
    }
    
    base = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (!base) {
        CloseHandle(hMap);
        return apr_get_os_error();
    }
    
    *m = (apr_shm_t *) apr_palloc(pool, sizeof(apr_shm_t));
    (*m)->pool = pool;
    (*m)->memblk = base;
    /* Real (*m)->mem->size could be recovered with VirtualQuery */
    (*m)->size = (*m)->memblk->size;
#if _WIN32_WCE
    /* Reopen with real size  */
    UnmapViewOfFile(base);
    CloseHandle(hMap);

    hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, 
                              PAGE_READWRITE, 0, (*m)->size, mapkey);
    if (!hMap) {
        return apr_get_os_error();
    }
    base = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (!base) {
        CloseHandle(hMap);
        return apr_get_os_error();
    }    
#endif
    (*m)->hMap = hMap;
    (*m)->length = (*m)->memblk->length;
    (*m)->usrmem = (char*)base + sizeof(memblock_t);
    (*m)->filename = NULL;

    apr_pool_cleanup_register((*m)->pool, *m, 
                              shm_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_attach_ex(apr_shm_t **m,
                                            const char *file,
                                            apr_pool_t *pool,
                                            apr_int32_t flags)
{
    apr_status_t rv;
    int can_create_global;
    int try_global_local[3] = {-1, -1, -1};
    int cur;

    if (!file) {
        return APR_EINVAL;
    }

    if (flags & APR_SHM_NS_LOCAL) {
        try_global_local[0] = 0; /* only search local */
    }
    else if (flags & APR_SHM_NS_GLOBAL) {
        try_global_local[0] = 1; /* only search global */
    }
    else {
        can_create_global = can_create_global_maps();
        if (!can_create_global) { /* unprivileged process */
            try_global_local[0] = 0; /* search local before global */
            try_global_local[1] = 1;
        }
        else {
            try_global_local[0] = 1; /* search global before local */
            try_global_local[1] = 0;
        }
    }

    for (cur = 0; try_global_local[cur] != -1; cur++) {
        rv = shm_attach_internal(m, file, pool, try_global_local[cur]);
        if (!APR_STATUS_IS_ENOENT(rv)) {
            break;
        }
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_shm_attach(apr_shm_t **m,
                                         const char *file,
                                         apr_pool_t *pool)
{
    return apr_shm_attach_ex(m, file, pool, 0);
}

APR_DECLARE(apr_status_t) apr_shm_detach(apr_shm_t *m)
{
    apr_status_t rv = shm_cleanup(m);
    apr_pool_cleanup_kill(m->pool, m, shm_cleanup);
    return rv;
}

APR_DECLARE(void *) apr_shm_baseaddr_get(const apr_shm_t *m)
{
    return m->usrmem;
}

APR_DECLARE(apr_size_t) apr_shm_size_get(const apr_shm_t *m)
{
    return m->length;
}

APR_POOL_IMPLEMENT_ACCESSOR(shm)

APR_DECLARE(apr_status_t) apr_os_shm_get(apr_os_shm_t *osshm,
                                         apr_shm_t *shm)
{
    *osshm = shm->hMap;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_shm_put(apr_shm_t **m,
                                         apr_os_shm_t *osshm,
                                         apr_pool_t *pool)
{
    void* base;
    base = MapViewOfFile(*osshm, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (!base) {
        return apr_get_os_error();
    }
    
    *m = (apr_shm_t *) apr_palloc(pool, sizeof(apr_shm_t));
    (*m)->pool = pool;
    (*m)->hMap = *osshm;
    (*m)->memblk = base;
    (*m)->usrmem = (char*)base + sizeof(memblock_t);
    /* Real (*m)->mem->size could be recovered with VirtualQuery */
    (*m)->size = (*m)->memblk->size;
    (*m)->length = (*m)->memblk->length;
    (*m)->filename = NULL;

    apr_pool_cleanup_register((*m)->pool, *m, 
                              shm_cleanup, apr_pool_cleanup_null);
    return APR_SUCCESS;
}    

