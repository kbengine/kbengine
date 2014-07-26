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
#include "apr_arch_file_io.h"
#include "apr_strings.h"
#include "apr_lib.h"
#include <ctype.h>

/* OS/2 Exceptions:
 *
 * Note that trailing spaces and trailing periods are never recorded
 * in the file system.
 *
 * Leading spaces and periods are accepted, however.
 * The * ? < > codes all have wildcard side effects
 * The " / \ : are exclusively component separator tokens 
 * The system doesn't accept | for any (known) purpose 
 * Oddly, \x7f _is_ acceptable ;)
 */

const char c_is_fnchar[256] =
{/* Reject all ctrl codes...                                         */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
 /*     "               *         /                      :   <   > ? */
    1,1,0,1,1,1,1,1,1,1,0,1,1,1,1,0, 1,1,1,1,1,1,1,1,1,1,0,1,0,1,0,0,
 /*                                                          \       */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
 /*                                                          |       */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,
 /* High bit codes are accepted                                      */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};


#define IS_SLASH(c) (c == '/' || c == '\\')


apr_status_t filepath_root_test(char *path, apr_pool_t *p)
{
    char drive = apr_toupper(path[0]);

    if (drive >= 'A' && drive <= 'Z' && path[1] == ':' && IS_SLASH(path[2]))
        return APR_SUCCESS;

    return APR_EBADPATH;
}


apr_status_t filepath_drive_get(char **rootpath, char drive, 
                                apr_int32_t flags, apr_pool_t *p)
{
    char path[APR_PATH_MAX];
    char *pos;
    ULONG rc;
    ULONG bufsize = sizeof(path) - 3;

    path[0] = drive;
    path[1] = ':';
    path[2] = '/';

    rc = DosQueryCurrentDir(apr_toupper(drive) - 'A', path+3, &bufsize);

    if (rc) {
        return APR_FROM_OS_ERROR(rc);
    }

    if (!(flags & APR_FILEPATH_NATIVE)) {
        for (pos=path; *pos; pos++) {
            if (*pos == '\\')
                *pos = '/';
        }
    }

    *rootpath = apr_pstrdup(p, path);
    return APR_SUCCESS;
}


apr_status_t filepath_root_case(char **rootpath, char *root, apr_pool_t *p)
{
    if (root[0] && apr_islower(root[0]) && root[1] == ':') {
        *rootpath = apr_pstrdup(p, root);
        (*rootpath)[0] = apr_toupper((*rootpath)[0]);
    }
    else {
       *rootpath = root;
    }
    return APR_SUCCESS;
}


APR_DECLARE(apr_status_t) apr_filepath_get(char **defpath, apr_int32_t flags,
                                           apr_pool_t *p)
{
    char path[APR_PATH_MAX];
    ULONG drive;
    ULONG drivemap;
    ULONG rv, pathlen = sizeof(path) - 3;
    char *pos;

    DosQueryCurrentDisk(&drive, &drivemap);
    path[0] = '@' + drive;
    strcpy(path+1, ":\\");
    rv = DosQueryCurrentDir(drive, path+3, &pathlen);

    *defpath = apr_pstrdup(p, path);

    if (!(flags & APR_FILEPATH_NATIVE)) {
        for (pos=*defpath; *pos; pos++) {
            if (*pos == '\\')
                *pos = '/';
        }
    }

    return APR_SUCCESS;
}    



APR_DECLARE(apr_status_t) apr_filepath_set(const char *path, apr_pool_t *p)
{
    ULONG rv = 0;

    if (path[1] == ':')
        rv = DosSetDefaultDisk(apr_toupper(path[0]) - '@');

    if (rv == 0)
        rv = DosSetCurrentDir(path);

    return APR_FROM_OS_ERROR(rv);
}
