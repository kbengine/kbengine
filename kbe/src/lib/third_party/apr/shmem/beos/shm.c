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
#include <stdio.h>
#include <stdlib.h>
#include <kernel/OS.h>
#include "apr_portable.h"

struct apr_shm_t {
    apr_pool_t *pool;
    void *memblock;
    void *ptr;
    apr_size_t reqsize;
    apr_size_t avail;
    area_id aid;
};

APR_DECLARE(apr_status_t) apr_shm_create(apr_shm_t **m, 
                                         apr_size_t reqsize, 
                                         const char *filename, 
                                         apr_pool_t *p)
{
    apr_size_t pagesize;
    area_id newid;
    char *addr;
    char shname[B_OS_NAME_LENGTH];
    
    (*m) = (apr_shm_t *)apr_pcalloc(p, sizeof(apr_shm_t));
    /* we MUST allocate in pages, so calculate how big an area we need... */
    pagesize = ((reqsize + B_PAGE_SIZE - 1) / B_PAGE_SIZE) * B_PAGE_SIZE;
     
    if (!filename) {
        int num = 0;
        snprintf(shname, B_OS_NAME_LENGTH, "apr_shmem_%ld", find_thread(NULL));
        while (find_area(shname) >= 0)
            snprintf(shname, B_OS_NAME_LENGTH, "apr_shmem_%ld_%d",
                     find_thread(NULL), num++);
    }
    newid = create_area(filename ? filename : shname, 
                        (void*)&addr, B_ANY_ADDRESS,
                        pagesize, B_LAZY_LOCK, B_READ_AREA|B_WRITE_AREA);

    if (newid < 0)
        return errno;

    (*m)->pool = p;
    (*m)->aid = newid;
    (*m)->memblock = addr;
    (*m)->ptr = (void*)addr;
    (*m)->avail = pagesize; /* record how big an area we actually created... */
    (*m)->reqsize = reqsize;

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
    delete_area(m->aid);
    m->avail = 0;
    m->memblock = NULL;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_remove(const char *filename,
                                         apr_pool_t *pool)
{
    area_id deleteme = find_area(filename);
    
    if (deleteme == B_NAME_NOT_FOUND)
        return APR_EINVAL;

    delete_area(deleteme);
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_shm_attach(apr_shm_t **m,
                                         const char *filename,
                                         apr_pool_t *pool)
{
    area_info ai;
    thread_info ti;
    apr_shm_t *new_m;
    area_id deleteme = find_area(filename);

    if (deleteme == B_NAME_NOT_FOUND)
        return APR_EINVAL;

    new_m = (apr_shm_t*)apr_palloc(pool, sizeof(apr_shm_t*));
    if (new_m == NULL)
        return APR_ENOMEM;
    new_m->pool = pool;

    get_area_info(deleteme, &ai);
    get_thread_info(find_thread(NULL), &ti);

    if (ti.team != ai.team) {
        area_id narea;
        
        narea = clone_area(ai.name, &(ai.address), B_CLONE_ADDRESS,
                           B_READ_AREA|B_WRITE_AREA, ai.area);

        if (narea < B_OK)
            return narea;
            
        get_area_info(narea, &ai);
        new_m->aid = narea;
        new_m->memblock = ai.address;
        new_m->ptr = (void*)ai.address;
        new_m->avail = ai.size;
        new_m->reqsize = ai.size;
    }

    (*m) = new_m;
    
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
    delete_area(m->aid);
    return APR_SUCCESS;
}

APR_DECLARE(void *) apr_shm_baseaddr_get(const apr_shm_t *m)
{
    return m->memblock;
}

APR_DECLARE(apr_size_t) apr_shm_size_get(const apr_shm_t *m)
{
    return m->reqsize;
}

APR_POOL_IMPLEMENT_ACCESSOR(shm)

APR_DECLARE(apr_status_t) apr_os_shm_get(apr_os_shm_t *osshm,
                                         apr_shm_t *shm)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_os_shm_put(apr_shm_t **m,
                                         apr_os_shm_t *osshm,
                                         apr_pool_t *pool)
{
    return APR_ENOTIMPL;
}    

