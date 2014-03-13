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

#include "apr_arch_threadproc.h"
#include "apr_arch_file_io.h"
#include "apr_thread_proc.h"
#include "apr_signal.h"
#include "apr_file_io.h"
#include "apr_general.h"
#if APR_HAVE_SIGNAL_H
#include <signal.h>
#endif
#include <string.h>
#if APR_HAVE_SYS_WAIT
#include <sys/wait.h>
#endif

/* Windows only really support killing process, but that will do for now. 
 *
 * ### Actually, closing the input handle to the proc should also do fine 
 * for most console apps.  This definitely needs improvement...
 */
APR_DECLARE(apr_status_t) apr_proc_kill(apr_proc_t *proc, int signal)
{
    if (proc->hproc != NULL) {
        if (TerminateProcess(proc->hproc, signal) == 0) {
            return apr_get_os_error();
        }
        /* On unix, SIGKILL leaves a apr_proc_wait()able pid lying around, 
         * so we will leave hproc alone until the app calls apr_proc_wait().
         */
        return APR_SUCCESS;
    }
    return APR_EPROC_UNKNOWN;
}

void apr_signal_init(apr_pool_t *pglobal)
{
}

APR_DECLARE(const char *) apr_signal_description_get(int signum)
{
    return "unknown signal (not supported)";
}

APR_DECLARE(apr_status_t) apr_signal_block(int signum)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_signal_unblock(int signum)
{
    return APR_ENOTIMPL;
}
