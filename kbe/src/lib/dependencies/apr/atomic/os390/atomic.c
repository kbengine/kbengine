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


#include "apr.h"
#include "apr_atomic.h"

#include <stdlib.h>

apr_status_t apr_atomic_init(apr_pool_t *p)
{
    return APR_SUCCESS;
}

apr_uint32_t apr_atomic_add32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    apr_uint32_t old, new_val; 

    old = *mem;   /* old is automatically updated on cs failure */
    do {
        new_val = old + val;
    } while (__cs(&old, (cs_t *)mem, new_val));
    return old;
}

void apr_atomic_sub32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
     apr_uint32_t old, new_val;

     old = *mem;   /* old is automatically updated on cs failure */
     do {
         new_val = old - val;
     } while (__cs(&old, (cs_t *)mem, new_val));
}

apr_uint32_t apr_atomic_inc32(volatile apr_uint32_t *mem)
{
    return apr_atomic_add32(mem, 1);
}

int apr_atomic_dec32(volatile apr_uint32_t *mem)
{
    apr_uint32_t old, new_val; 

    old = *mem;   /* old is automatically updated on cs failure */
    do {
        new_val = old - 1;
    } while (__cs(&old, (cs_t *)mem, new_val)); 

    return new_val != 0;
}

apr_uint32_t apr_atomic_read32(volatile apr_uint32_t *mem)
{
    return *mem;
}

void apr_atomic_set32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    *mem = val;
}

apr_uint32_t apr_atomic_cas32(volatile apr_uint32_t *mem, apr_uint32_t swap, 
                              apr_uint32_t cmp)
{
    apr_uint32_t old = cmp;
    
    __cs(&old, (cs_t *)mem, swap);
    return old; /* old is automatically updated from mem on cs failure */
}

#if APR_SIZEOF_VOIDP == 4
void *apr_atomic_casptr(volatile void **mem_ptr,
                        void *swap_ptr,
                        const void *cmp_ptr)
{
     __cs1(&cmp_ptr,     /* automatically updated from mem on __cs1 failure  */
           mem_ptr,      /* set from swap when __cs1 succeeds                */
           &swap_ptr);
     return (void *)cmp_ptr;
}
#elif APR_SIZEOF_VOIDP == 8
void *apr_atomic_casptr(volatile void **mem_ptr,
                        void *swap_ptr,
                        const void *cmp_ptr)
{
     __csg(&cmp_ptr,     /* automatically updated from mem on __csg failure  */
           mem_ptr,      /* set from swap when __csg succeeds                */
           &swap_ptr);  
     return (void *)cmp_ptr;
}
#else
#error APR_SIZEOF_VOIDP value not supported
#endif /* APR_SIZEOF_VOIDP */

apr_uint32_t apr_atomic_xchg32(volatile apr_uint32_t *mem, apr_uint32_t val)
{
    apr_uint32_t old, new_val; 

    old = *mem;   /* old is automatically updated on cs failure */
    do {
        new_val = val;
    } while (__cs(&old, (cs_t *)mem, new_val)); 

    return old;
}

APR_DECLARE(void*) apr_atomic_xchgptr(volatile void **mem_ptr, void *new_ptr)
{
    void *old_ptr;

    old_ptr = *(void **)mem_ptr; /* old is automatically updated on cs failure */
#if APR_SIZEOF_VOIDP == 4
    do {
    } while (__cs1(&old_ptr, mem_ptr, &new_ptr)); 
#elif APR_SIZEOF_VOIDP == 8
    do { 
    } while (__csg(&old_ptr, mem_ptr, &new_ptr)); 
#else
#error APR_SIZEOF_VOIDP value not supported
#endif /* APR_SIZEOF_VOIDP */

    return old_ptr;
}
