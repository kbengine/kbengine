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

#include "apr_private.h"
#include "apr_file_io.h" /* prototype of apr_mkstemp() */
#include "apr_strings.h" /* prototype of apr_mkstemp() */
#include "apr_arch_file_io.h" /* prototype of apr_mkstemp() */
#include "apr_portable.h" /* for apr_os_file_put() */
#include "apr_arch_inherit.h"

#include <stdlib.h> /* for mkstemp() - Single Unix */

APR_DECLARE(apr_status_t) apr_file_mktemp(apr_file_t **fp, char *template, apr_int32_t flags, apr_pool_t *p)
{
    int fd;
    apr_status_t rv;

    flags = (!flags) ? APR_FOPEN_CREATE | APR_FOPEN_READ | APR_FOPEN_WRITE |
                       APR_FOPEN_DELONCLOSE : flags & ~APR_FOPEN_EXCL;

    fd = mkstemp(template);
    if (fd == -1) {
        return errno;
    }
    /* We need to reopen the file to get rid of the o_excl flag.
     * Otherwise file locking will not allow the file to be shared.
     */
    close(fd);
    if ((rv = apr_file_open(fp, template, flags|APR_FOPEN_NOCLEANUP,
                            APR_UREAD | APR_UWRITE, p)) == APR_SUCCESS) {


	if (!(flags & APR_FOPEN_NOCLEANUP)) {
            int flags;

            if ((flags = fcntl((*fp)->filedes, F_GETFD)) == -1)
                return errno;

            flags |= FD_CLOEXEC;
            if (fcntl((*fp)->filedes, F_SETFD, flags) == -1)
                return errno;

	    apr_pool_cleanup_register((*fp)->pool, (void *)(*fp),
				      apr_unix_file_cleanup,
				      apr_unix_child_file_cleanup);
	}
    }

    return rv;
}

