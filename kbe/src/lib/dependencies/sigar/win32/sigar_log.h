/*
 * Copyright (c) 2004, 2006 Hyperic, Inc.
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

#ifndef SIGAR_LOG_H
#define SIGAR_LOG_H

#include <stdarg.h>

#define	SIGAR_LOG_FATAL  0
#define	SIGAR_LOG_ERROR  1
#define	SIGAR_LOG_WARN   2
#define	SIGAR_LOG_INFO   3
#define	SIGAR_LOG_DEBUG  4
#define	SIGAR_LOG_TRACE  5

#define SIGAR_LOG_IS_FATAL(sigar) \
    (sigar->log_level >= SIGAR_LOG_FATAL)

#define SIGAR_LOG_IS_ERROR(sigar) \
    (sigar->log_level >= SIGAR_LOG_ERROR)

#define SIGAR_LOG_IS_WARN(sigar) \
    (sigar->log_level >= SIGAR_LOG_WARN)

#define SIGAR_LOG_IS_INFO(sigar) \
    (sigar->log_level >= SIGAR_LOG_INFO)

#define SIGAR_LOG_IS_DEBUG(sigar) \
    (sigar->log_level >= SIGAR_LOG_DEBUG)

#define SIGAR_LOG_IS_TRACE(sigar) \
    (sigar->log_level >= SIGAR_LOG_TRACE)

#define SIGAR_STRINGIFY(n) #n

#define SIGAR_LOG_FILELINE \
   __FILE__ ":" SIGAR_STRINGIFY(__LINE__)

#if defined(__GNUC__)
#   if (__GNUC__ > 2)
#      define SIGAR_FUNC __func__
#   else
#      define SIGAR_FUNC __FUNCTION__
#   endif
#else
#   define SIGAR_FUNC SIGAR_LOG_FILELINE
#endif

typedef void (*sigar_log_impl_t)(sigar_t *, void *, int, char *);

SIGAR_DECLARE(void) sigar_log_printf(sigar_t *sigar, int level,
                                     const char *format, ...);

SIGAR_DECLARE(void) sigar_log(sigar_t *sigar, int level, char *message);

SIGAR_DECLARE(void) sigar_log_impl_set(sigar_t *sigar, void *data,
                                       sigar_log_impl_t impl);

SIGAR_DECLARE(void) sigar_log_impl_file(sigar_t *sigar, void *data,
                                        int level, char *message);

SIGAR_DECLARE(int) sigar_log_level_get(sigar_t *sigar);

SIGAR_DECLARE(void) sigar_log_level_set(sigar_t *sigar, int level);


#endif /* SIGAR_LOG_H */
