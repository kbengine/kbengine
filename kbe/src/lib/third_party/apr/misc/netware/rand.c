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

#define APR_WANT_MEMFUNC
#include "apr_want.h"
#include "apr_general.h"
#include "apr_private.h"

#if APR_HAS_RANDOM

#include <nks/plat.h>

static int NXSeedRandomInternal( size_t width, void *seed )
{
    static int init = 0;
    int                        *s = (int *) seed;
    union { int x; char y[4]; } u;

    if (!init) {
        srand(NXGetSystemTick());
        init = 1;
    }
 
    if (width > 3)
    {
        do
        {
            *s++ = rand();
        }
        while ((width -= 4) > 3);
    }
 
    if (width > 0)
    {
        char *p = (char *) s;

        u.x = rand();
 
        while (width > 0)
           *p++ = u.y[width--];
    }
 
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_generate_random_bytes(unsigned char *buf, 
                                                    apr_size_t length)
{
    if (NXSeedRandom(length, buf) != 0) {
        return NXSeedRandomInternal (length, buf);
    }
    return APR_SUCCESS;
}



#endif /* APR_HAS_RANDOM */
