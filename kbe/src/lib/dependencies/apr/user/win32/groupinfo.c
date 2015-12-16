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

#include "apr_strings.h"
#include "apr_portable.h"
#include "apr_user.h"
#include "apr_private.h"
#if APR_HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

APR_DECLARE(apr_status_t) apr_gid_get(apr_gid_t *gid, 
                                      const char *groupname, apr_pool_t *p)
{
#ifdef _WIN32_WCE
    return APR_ENOTIMPL;
#else
    SID_NAME_USE sidtype;
    char anydomain[256];
    char *domain;
    DWORD sidlen = 0;
    DWORD domlen = sizeof(anydomain);
    DWORD rv;
    char *pos;

    if ((pos = strchr(groupname, '/'))) {
        domain = apr_pstrndup(p, groupname, pos - groupname);
        groupname = pos + 1;
    }
    else if ((pos = strchr(groupname, '\\'))) {
        domain = apr_pstrndup(p, groupname, pos - groupname);
        groupname = pos + 1;
    }
    else {
        domain = NULL;
    }
    /* Get nothing on the first pass ... need to size the sid buffer 
     */
    rv = LookupAccountName(domain, groupname, domain, &sidlen, 
                           anydomain, &domlen, &sidtype);
    if (sidlen) {
        /* Give it back on the second pass
         */
        *gid = apr_palloc(p, sidlen);
        domlen = sizeof(anydomain);
        rv = LookupAccountName(domain, groupname, *gid, &sidlen, 
                               anydomain, &domlen, &sidtype);
    }
    if (!sidlen || !rv) {
        return apr_get_os_error();
    }
    return APR_SUCCESS;
#endif
}

APR_DECLARE(apr_status_t) apr_gid_name_get(char **groupname, apr_gid_t groupid, apr_pool_t *p)
{
#ifdef _WIN32_WCE
    *groupname = apr_pstrdup(p, "Administrators");
#else
    SID_NAME_USE type;
    char name[MAX_PATH], domain[MAX_PATH];
    DWORD cbname = sizeof(name), cbdomain = sizeof(domain);
    if (!groupid)
        return APR_EINVAL;
    if (!LookupAccountSid(NULL, groupid, name, &cbname, domain, &cbdomain, &type))
        return apr_get_os_error();
    if (type != SidTypeGroup && type != SidTypeWellKnownGroup 
                             && type != SidTypeAlias)
        return APR_EINVAL;
    *groupname = apr_pstrdup(p, name);
#endif
    return APR_SUCCESS;
}
  
APR_DECLARE(apr_status_t) apr_gid_compare(apr_gid_t left, apr_gid_t right)
{
    if (!left || !right)
        return APR_EINVAL;
#ifndef _WIN32_WCE
    if (!IsValidSid(left) || !IsValidSid(right))
        return APR_EINVAL;
    if (!EqualSid(left, right))
        return APR_EMISMATCH;
#endif
    return APR_SUCCESS;
}
