/*
 * Copyright (c) 2006-2007 Hyperic, Inc.
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

#ifndef SIGAR_PTQL_H
#define SIGAR_PTQL_H

#define SIGAR_PTQL_MALFORMED_QUERY -1

typedef struct sigar_ptql_query_t sigar_ptql_query_t;

#define SIGAR_PTQL_ERRMSG_SIZE 1024

typedef struct {
    char message[SIGAR_PTQL_ERRMSG_SIZE];
} sigar_ptql_error_t;

typedef int (*sigar_ptql_re_impl_t)(void *, char *, char *);

SIGAR_DECLARE(void) sigar_ptql_re_impl_set(sigar_t *sigar, void *data,
                                           sigar_ptql_re_impl_t impl);

SIGAR_DECLARE(int) sigar_ptql_query_create(sigar_ptql_query_t **query,
                                           char *ptql,
                                           sigar_ptql_error_t *error);

SIGAR_DECLARE(int) sigar_ptql_query_match(sigar_t *sigar,
                                          sigar_ptql_query_t *query,
                                          sigar_pid_t pid);

SIGAR_DECLARE(int) sigar_ptql_query_destroy(sigar_ptql_query_t *query);

SIGAR_DECLARE(int) sigar_ptql_query_find_process(sigar_t *sigar,
                                                 sigar_ptql_query_t *query,
                                                 sigar_pid_t *pid);

SIGAR_DECLARE(int) sigar_ptql_query_find(sigar_t *sigar,
                                         sigar_ptql_query_t *query,
                                         sigar_proc_list_t *proclist);

#endif /*SIGAR_PTQL_H*/
