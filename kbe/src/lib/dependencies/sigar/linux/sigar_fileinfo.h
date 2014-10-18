/*
 * Copyright (c) 2004-2005 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2003 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "sigar.h"

typedef enum {
    SIGAR_FILETYPE_NOFILE = 0,     /**< no file type determined */
    SIGAR_FILETYPE_REG,            /**< a regular file */
    SIGAR_FILETYPE_DIR,            /**< a directory */
    SIGAR_FILETYPE_CHR,            /**< a character device */
    SIGAR_FILETYPE_BLK,            /**< a block device */
    SIGAR_FILETYPE_PIPE,           /**< a FIFO / pipe */
    SIGAR_FILETYPE_LNK,            /**< a symbolic link */
    SIGAR_FILETYPE_SOCK,           /**< a [unix domain] socket */
    SIGAR_FILETYPE_UNKFILE         /**< a file of some other unknown type */
} sigar_file_type_e; 

#define SIGAR_UREAD       0x0400 /**< Read by user */
#define SIGAR_UWRITE      0x0200 /**< Write by user */
#define SIGAR_UEXECUTE    0x0100 /**< Execute by user */

#define SIGAR_GREAD       0x0040 /**< Read by group */
#define SIGAR_GWRITE      0x0020 /**< Write by group */
#define SIGAR_GEXECUTE    0x0010 /**< Execute by group */

#define SIGAR_WREAD       0x0004 /**< Read by others */
#define SIGAR_WWRITE      0x0002 /**< Write by others */
#define SIGAR_WEXECUTE    0x0001 /**< Execute by others */

typedef struct {
    /** The access permissions of the file.  Mimics Unix access rights. */
    sigar_uint64_t permissions;
    sigar_file_type_e type;
    /** The user id that owns the file */
    sigar_uid_t uid;
    /** The group id that owns the file */
    sigar_gid_t gid;
    /** The inode of the file. */
    sigar_uint64_t inode;
    /** The id of the device the file is on. */
    sigar_uint64_t device;
    /** The number of hard links to the file. */
    sigar_uint64_t nlink;
    /** The size of the file */
    sigar_uint64_t size;
    /** The time the file was last accessed */
    sigar_uint64_t atime;
    /** The time the file was last modified */
    sigar_uint64_t mtime;
    /** The time the file was last changed */
    sigar_uint64_t ctime;
} sigar_file_attrs_t;

typedef struct {
    sigar_uint64_t total;
    sigar_uint64_t files;
    sigar_uint64_t subdirs;
    sigar_uint64_t symlinks;
    sigar_uint64_t chrdevs;
    sigar_uint64_t blkdevs;
    sigar_uint64_t sockets;
    sigar_uint64_t disk_usage;
} sigar_dir_stat_t;

typedef sigar_dir_stat_t sigar_dir_usage_t;

SIGAR_DECLARE(const char *)
sigar_file_attrs_type_string_get(sigar_file_type_e type);

SIGAR_DECLARE(int) sigar_file_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs);

SIGAR_DECLARE(int) sigar_link_attrs_get(sigar_t *sigar,
                                        const char *file,
                                        sigar_file_attrs_t *fileattrs);

SIGAR_DECLARE(int)sigar_file_attrs_mode_get(sigar_uint64_t permissions);

SIGAR_DECLARE(char *)
sigar_file_attrs_permissions_string_get(sigar_uint64_t permissions,
                                       char *str);

SIGAR_DECLARE(int) sigar_dir_stat_get(sigar_t *sigar,
                                      const char *dir,
                                      sigar_dir_stat_t *dirstats);

SIGAR_DECLARE(int) sigar_dir_usage_get(sigar_t *sigar,
                                       const char *dir,
                                       sigar_dir_usage_t *dirusage);
