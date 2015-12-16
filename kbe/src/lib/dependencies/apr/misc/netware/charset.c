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
#include "apr_strings.h"
#include "apr_portable.h"

/* static struct utsname sysinfo; */

/* XXX This needs to be fixed to produce the correct system language */

APR_DECLARE(const char*) apr_os_default_encoding (apr_pool_t *pool)
{
    return apr_pstrdup(pool, "CP1252");
}


APR_DECLARE(const char*) apr_os_locale_encoding (apr_pool_t *pool)
{
    return apr_os_default_encoding(pool);
}
