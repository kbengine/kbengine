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
#include "apr_shm.h"
#include "apr_errno.h"
#include "apr_lib.h"
#include "apr_strings.h"
#include "apr_portable.h"

struct apr_shm_t {
    apr_pool_t *pool;
    void *memblock;
};

APR_DECLARE(apr_status_t) apr_shm_create(apr_shm_t **m,
                                         apr_size_t reqsize,
                                         const char *filename,
                                         apr_pool_t *pool)
{
    int rc;
    apr_shm_t *newm = (apr_shm_t *)apr_palloc(pool, sizeof(apr_shm_t));
    char *name = NULL;
    ULONG flags = PAG_COMMIT|PAG_READ|PAG_WRITE;

    newm->pool = pool;

    if (filename) {
        name = apr_pstrcat(pool, "\\SHAREMEM\\", filename, NULL);
    }

    if (name == NULL) {
        flags |= OBJ_GETTABLE;
    }

    rc = DosAllocSharedMem(&(newm->memblock), name, reqsize, flags);

    if (rc) {
        return APR_OS2_STATUS(rc);
    }

    *m = newm;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_create_ex(apr_shm_t **m, 
                                            apr_size_t reqsize, 
                                            const char *filename, 
                                            apr_pool_t *p,
                                            apr_int32_t flags)
{
    return apr_shm_create(m, reqsize, filename, p);
}

APR_DECLARE(apr_status_t) apr_shm_destroy(apr_shm_t *m)
{
    DosFreeMem(m->memblock);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_remove(const char *filename,
                                         apr_pool_t *pool)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_shm_attach(apr_shm_t **m,
                                         const char *filename,
                                         apr_pool_t *pool)
{
    int rc;
    apr_shm_t *newm = (apr_shm_t *)apr_palloc(pool, sizeof(apr_shm_t));
    char *name = NULL;
    ULONG flags = PAG_READ|PAG_WRITE;

    newm->pool = pool;
    name = apr_pstrcat(pool, "\\SHAREMEM\\", filename, NULL);

    rc = DosGetNamedSharedMem(&(newm->memblock), name, flags);

    if (rc) {
        return APR_FROM_OS_ERROR(rc);
    }

    *m = newm;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_attach_ex(apr_shm_t **m,
                                            const char *filename,
                                            apr_pool_t *pool,
                                            apr_int32_t flags)
{
    return apr_shm_attach(m, filename, pool);
}

APR_DECLARE(apr_status_t) apr_shm_detach(apr_shm_t *m)
{
    int rc = 0;

    if (m->memblock) {
        rc = DosFreeMem(m->memblock);
    }

    return APR_FROM_OS_ERROR(rc);
}

APR_DECLARE(void *) apr_shm_baseaddr_get(const apr_shm_t *m)
{
    return m->memblock;
}

APR_DECLARE(apr_size_t) apr_shm_size_get(const apr_shm_t *m)
{
    ULONG flags, size = 0x1000000;
    DosQueryMem(m->memblock, &size, &flags);
    return size;
}

APR_POOL_IMPLEMENT_ACCESSOR(shm)

APR_DECLARE(apr_status_t) apr_os_shm_get(apr_os_shm_t *osshm,
                                         apr_shm_t *shm)
{
    *osshm = shm->memblock;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_os_shm_put(apr_shm_t **m,
                                         apr_os_shm_t *osshm,
                                         apr_pool_t *pool)
{
    int rc;
    apr_shm_t *newm = (apr_shm_t *)apr_palloc(pool, sizeof(apr_shm_t));
    ULONG flags = PAG_COMMIT|PAG_READ|PAG_WRITE;

    newm->pool = pool;

    rc = DosGetSharedMem(&(newm->memblock), flags);

    if (rc) {
        return APR_FROM_OS_ERROR(rc);
    }

    *m = newm;
    return APR_SUCCESS;
}    

