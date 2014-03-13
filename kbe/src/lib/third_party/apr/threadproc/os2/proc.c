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

#define INCL_DOS
#define INCL_DOSERRORS

#include "apr_arch_threadproc.h"
#include "apr_arch_file_io.h"
#include "apr_private.h"
#include "apr_thread_proc.h"
#include "apr_file_io.h"
#include "apr_general.h"
#include "apr_lib.h"
#include "apr_portable.h"
#include "apr_strings.h"
#include "apr_signal.h"
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <process.h>
#include <stdlib.h>

/* Heavy on no'ops, here's what we want to pass if there is APR_NO_FILE
 * requested for a specific child handle;
 */
static apr_file_t no_file = { NULL, -1, };

APR_DECLARE(apr_status_t) apr_procattr_create(apr_procattr_t **new, apr_pool_t *pool)
{
    (*new) = (apr_procattr_t *)apr_palloc(pool, 
              sizeof(apr_procattr_t));

    if ((*new) == NULL) {
        return APR_ENOMEM;
    }
    (*new)->pool = pool;
    (*new)->parent_in = NULL;
    (*new)->child_in = NULL;
    (*new)->parent_out = NULL;
    (*new)->child_out = NULL;
    (*new)->parent_err = NULL;
    (*new)->child_err = NULL;
    (*new)->currdir = NULL; 
    (*new)->cmdtype = APR_PROGRAM;
    (*new)->detached = FALSE;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_io_set(apr_procattr_t *attr,
                                              apr_int32_t in,
                                              apr_int32_t out,
                                              apr_int32_t err)
{
    apr_status_t rv;

    if ((in != APR_NO_PIPE) && (in != APR_NO_FILE)) {
        /* APR_CHILD_BLOCK maps to APR_WRITE_BLOCK, while
         * APR_PARENT_BLOCK maps to APR_READ_BLOCK, so transpose 
         * the CHILD/PARENT blocking flags for the stdin pipe.
         * stdout/stderr map to the correct mode by default.
         */
        if (in == APR_CHILD_BLOCK)
            in = APR_READ_BLOCK;
        else if (in == APR_PARENT_BLOCK)
            in = APR_WRITE_BLOCK;

        if ((rv = apr_file_pipe_create_ex(&attr->child_in, &attr->parent_in,
                                          in, attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_in);
        if (rv != APR_SUCCESS)
            return rv;
    }
    else if (in == APR_NO_FILE)
        attr->child_in = &no_file;

    if ((out != APR_NO_PIPE) && (out != APR_NO_FILE)) {
        if ((rv = apr_file_pipe_create_ex(&attr->parent_out, &attr->child_out,
                                          out, attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_out);
        if (rv != APR_SUCCESS)
            return rv;
    }
    else if (out == APR_NO_FILE)
        attr->child_out = &no_file;

    if ((err != APR_NO_PIPE) && (err != APR_NO_FILE)) {
        if ((rv = apr_file_pipe_create_ex(&attr->parent_err, &attr->child_err,
                                          err, attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_err);
        if (rv != APR_SUCCESS)
            return rv;
    }
    else if (err == APR_NO_FILE)
        attr->child_err = &no_file;

    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_child_in_set(apr_procattr_t *attr, apr_file_t *child_in,
                                   apr_file_t *parent_in)
{
    apr_status_t rv;

    if (attr->child_in == NULL && attr->parent_in == NULL
            && child_in == NULL && parent_in == NULL)
        if ((rv = apr_file_pipe_create(&attr->child_in, &attr->parent_in,
                                       attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_in);

    if (child_in != NULL && rv == APR_SUCCESS) {
        if (attr->child_in && (attr->child_in->filedes != -1))
            rv = apr_file_dup2(attr->child_in, child_in, attr->pool);
        else {
            attr->child_in = NULL;
            if ((rv = apr_file_dup(&attr->child_in, child_in, attr->pool))
                    == APR_SUCCESS)
                rv = apr_file_inherit_set(attr->child_in);
        }
    }

    if (parent_in != NULL && rv == APR_SUCCESS) {
        rv = apr_file_dup(&attr->parent_in, parent_in, attr->pool);
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_procattr_child_out_set(apr_procattr_t *attr, apr_file_t *child_out,
                                                     apr_file_t *parent_out)
{
    apr_status_t rv;

    if (attr->child_out == NULL && attr->parent_out == NULL
           && child_out == NULL && parent_out == NULL)
        if ((rv = apr_file_pipe_create(&attr->parent_out, &attr->child_out,
                                       attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_out);

    if (child_out != NULL && rv == APR_SUCCESS) {
        if (attr->child_out && (attr->child_out->filedes != -1))
            rv = apr_file_dup2(attr->child_out, child_out, attr->pool);
        else {
            attr->child_out = NULL;
            if ((rv = apr_file_dup(&attr->child_out, child_out, attr->pool))
                    == APR_SUCCESS)
                rv = apr_file_inherit_set(attr->child_out);
        }
    }
  
    if (parent_out != NULL && rv == APR_SUCCESS) {
        rv = apr_file_dup(&attr->parent_out, parent_out, attr->pool);
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_procattr_child_err_set(apr_procattr_t *attr, apr_file_t *child_err,
                                                     apr_file_t *parent_err)
{
    apr_status_t rv;

    if (attr->child_err == NULL && attr->parent_err == NULL
           && child_err == NULL && parent_err == NULL)
        if ((rv = apr_file_pipe_create(&attr->parent_err, &attr->child_err,
                                       attr->pool)) == APR_SUCCESS)
            rv = apr_file_inherit_unset(attr->parent_err);

    if (child_err != NULL && rv == APR_SUCCESS) {
        if (attr->child_err && (attr->child_err->filedes != -1))
            rv = apr_file_dup2(attr->child_err, child_err, attr->pool);
        else {
            attr->child_err = NULL;
            if ((rv = apr_file_dup(&attr->child_err, child_err, attr->pool))
                    == APR_SUCCESS)
                rv = apr_file_inherit_set(attr->child_err);
        }
    }
  
    if (parent_err != NULL && rv == APR_SUCCESS) {
        rv = apr_file_dup(&attr->parent_err, parent_err, attr->pool);
    }

    return rv;
}

APR_DECLARE(apr_status_t) apr_procattr_dir_set(apr_procattr_t *attr, const char *dir)
{
    attr->currdir = apr_pstrdup(attr->pool, dir);
    if (attr->currdir) {
        return APR_SUCCESS;
    }
    return APR_ENOMEM;
}

APR_DECLARE(apr_status_t) apr_procattr_cmdtype_set(apr_procattr_t *attr,
                                                   apr_cmdtype_e cmd) 
{
    attr->cmdtype = cmd;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_detach_set(apr_procattr_t *attr, apr_int32_t detach) 
{
    attr->detached = detach;
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_fork(apr_proc_t *proc, apr_pool_t *pool)
{
    int pid;
    
    if ((pid = fork()) < 0) {
        return errno;
    }
    else if (pid == 0) {
        proc->pid = pid;
        proc->in = NULL; 
        proc->out = NULL; 
        proc->err = NULL; 
        return APR_INCHILD;
    }
    proc->pid = pid;
    proc->in = NULL; 
    proc->out = NULL; 
    proc->err = NULL; 
    return APR_INPARENT;
}



/* quotes in the string are doubled up.
 * Used to escape quotes in args passed to OS/2's cmd.exe
 */
static char *double_quotes(apr_pool_t *pool, const char *str)
{
    int num_quotes = 0;
    int len = 0;
    char *quote_doubled_str, *dest;
    
    while (str[len]) {
        num_quotes += str[len++] == '\"';
    }
    
    quote_doubled_str = apr_palloc(pool, len + num_quotes + 1);
    dest = quote_doubled_str;
    
    while (*str) {
        if (*str == '\"')
            *(dest++) = '\"';
        *(dest++) = *(str++);
    }
    
    *dest = 0;
    return quote_doubled_str;
}



APR_DECLARE(apr_status_t) apr_procattr_child_errfn_set(apr_procattr_t *attr,
                                                       apr_child_errfn_t *errfn)
{
    /* won't ever be called on this platform, so don't save the function pointer */
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_procattr_error_check_set(apr_procattr_t *attr,
                                                       apr_int32_t chk)
{
    /* won't ever be used on this platform, so don't save the flag */
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_procattr_addrspace_set(apr_procattr_t *attr,
                                                       apr_int32_t addrspace)
{
    /* won't ever be used on this platform, so don't save the flag */
    return APR_SUCCESS;
}



APR_DECLARE(apr_status_t) apr_proc_create(apr_proc_t *proc, const char *progname,
                                          const char * const *args,
                                          const char * const *env,
                                          apr_procattr_t *attr, apr_pool_t *pool)
{
    int i, arg, numargs, cmdlen;
    apr_status_t status;
    const char **newargs;
    char savedir[300];
    HFILE save_in, save_out, save_err, dup;
    int criticalsection = FALSE;
    char *extension, *newprogname, *extra_arg = NULL, *cmdline, *cmdline_pos;
    char interpreter[1024];
    char error_object[260];
    apr_file_t *progfile;
    int env_len, e;
    char *env_block, *env_block_pos;
    RESULTCODES rescodes;

    proc->in = attr->parent_in;
    proc->err = attr->parent_err;
    proc->out = attr->parent_out;

    /* Prevent other threads from running while these process-wide resources are modified */
    if (attr->child_in || attr->child_out || attr->child_err || attr->currdir) {
        criticalsection = TRUE;
        DosEnterCritSec();
    }

    if (attr->child_in) {
        save_in = -1;
        DosDupHandle(STDIN_FILENO, &save_in);
        dup = STDIN_FILENO;
        if (attr->child_in->filedes == -1)
            DosClose(dup);
        else
            DosDupHandle(attr->child_in->filedes, &dup);
    }
    
    if (attr->child_out) {
        save_out = -1;
        DosDupHandle(STDOUT_FILENO, &save_out);
        dup = STDOUT_FILENO;
        if (attr->child_out->filedes == -1)
            DosClose(dup);
        else
            DosDupHandle(attr->child_out->filedes, &dup);
    }
    
    if (attr->child_err) {
        save_err = -1;
        DosDupHandle(STDERR_FILENO, &save_err);
        dup = STDERR_FILENO;
        if (attr->child_err->filedes == -1)
            DosClose(dup);
        else
            DosDupHandle(attr->child_err->filedes, &dup);
    }

    apr_signal(SIGCHLD, SIG_DFL); /*not sure if this is needed or not */

    if (attr->currdir != NULL) {
        _getcwd2(savedir, sizeof(savedir));
        
        if (_chdir2(attr->currdir) < 0) {
            if (criticalsection)
                DosExitCritSec();
            return errno;
        }
    }

    interpreter[0] = 0;
    extension = strrchr(progname, '.');

    if (extension == NULL || strchr(extension, '/') || strchr(extension, '\\'))
        extension = "";

    /* ### how to handle APR_PROGRAM_ENV and APR_PROGRAM_PATH? */

    if (attr->cmdtype == APR_SHELLCMD ||
        attr->cmdtype == APR_SHELLCMD_ENV ||
        strcasecmp(extension, ".cmd") == 0) {
        strcpy(interpreter, "#!" SHELL_PATH);
        extra_arg = "/C";
    } else if (stricmp(extension, ".exe") != 0) {
        status = apr_file_open(&progfile, progname, APR_READ|APR_BUFFERED, 0, pool);

        if (status != APR_SUCCESS && APR_STATUS_IS_ENOENT(status)) {
            progname = apr_pstrcat(pool, progname, ".exe", NULL);
        }

        if (status == APR_SUCCESS) {
            status = apr_file_gets(interpreter, sizeof(interpreter), progfile);

            if (status == APR_SUCCESS) {
                if (interpreter[0] == '#' && interpreter[1] == '!') {
                    /* delete CR/LF & any other whitespace off the end */
                    int end = strlen(interpreter) - 1;

                    while (end >= 0 && apr_isspace(interpreter[end])) {
                        interpreter[end] = '\0';
                        end--;
                    }

                    if (interpreter[2] != '/' && interpreter[2] != '\\' && interpreter[3] != ':') {
                        char buffer[300];

                        if (DosSearchPath(SEARCH_ENVIRONMENT, "PATH", interpreter+2, buffer, sizeof(buffer)) == 0) {
                            strcpy(interpreter+2, buffer);
                        } else {
                            strcat(interpreter, ".exe");
                            if (DosSearchPath(SEARCH_ENVIRONMENT, "PATH", interpreter+2, buffer, sizeof(buffer)) == 0) {
                                strcpy(interpreter+2, buffer);
                            }
                        }
                    }
                } else {
                    interpreter[0] = 0;
                }
            }

            apr_file_close(progfile);
        }
    }

    i = 0;

    while (args && args[i]) {
        i++;
    }

    newargs = (const char **)apr_palloc(pool, sizeof (char *) * (i + 4));
    numargs = 0;

    if (interpreter[0])
        newargs[numargs++] = interpreter + 2;
    if (extra_arg)
        newargs[numargs++] = "/c";

    newargs[numargs++] = newprogname = apr_pstrdup(pool, progname);
    arg = 1;

    while (args && args[arg]) {
        newargs[numargs++] = args[arg++];
    }

    newargs[numargs] = NULL;

    for (i=0; newprogname[i]; i++)
        if (newprogname[i] == '/')
            newprogname[i] = '\\';

    cmdlen = 0;

    for (i=0; i<numargs; i++)
        cmdlen += strlen(newargs[i]) + 3;

    cmdline = apr_palloc(pool, cmdlen + 2);
    cmdline_pos = cmdline;

    for (i=0; i<numargs; i++) {
        const char *a = newargs[i];

        if (strpbrk(a, "&|<>\" "))
            a = apr_pstrcat(pool, "\"", double_quotes(pool, a), "\"", NULL);

        if (i)
            *(cmdline_pos++) = ' ';

        strcpy(cmdline_pos, a);
        cmdline_pos += strlen(cmdline_pos);
    }

    *(++cmdline_pos) = 0; /* Add required second terminator */
    cmdline_pos = strchr(cmdline, ' ');

    if (cmdline_pos) {
        *cmdline_pos = 0;
        cmdline_pos++;
    }

    /* Create environment block from list of envariables */
    if (env) {
        for (env_len=1, e=0; env[e]; e++)
            env_len += strlen(env[e]) + 1;

        env_block = apr_palloc(pool, env_len);
        env_block_pos = env_block;

        for (e=0; env[e]; e++) {
            strcpy(env_block_pos, env[e]);
            env_block_pos += strlen(env_block_pos) + 1;
        }

        *env_block_pos = 0; /* environment block is terminated by a double null */
    } else
        env_block = NULL;

    status = DosExecPgm(error_object, sizeof(error_object),
                        attr->detached ? EXEC_BACKGROUND : EXEC_ASYNCRESULT,
                        cmdline, env_block, &rescodes, cmdline);

    proc->pid = rescodes.codeTerminate;

    if (attr->currdir != NULL) {
        chdir(savedir);
    }

    if (attr->child_in) {
        if (attr->child_in->filedes != -1) {
            apr_file_close(attr->child_in);
        }

        dup = STDIN_FILENO;
        DosDupHandle(save_in, &dup);
        DosClose(save_in);
    }
    
    if (attr->child_out) {
        if  (attr->child_out->filedes != -1) {
            apr_file_close(attr->child_out);
        }

        dup = STDOUT_FILENO;
        DosDupHandle(save_out, &dup);
        DosClose(save_out);
    }
    
    if (attr->child_err) {
        if (attr->child_err->filedes != -1) {
            apr_file_close(attr->child_err);
        }

        dup = STDERR_FILENO;
        DosDupHandle(save_err, &dup);
        DosClose(save_err);
    }

    if (criticalsection)
        DosExitCritSec();

    return status;
}



static void proces_result_codes(RESULTCODES codes, 
                                int *exitcode, 
                                apr_exit_why_e *exitwhy)
{
    int result = 0;
    apr_exit_why_e why = APR_PROC_EXIT;

    switch (codes.codeTerminate) {
    case TC_EXIT:        /* Normal exit */
        why = APR_PROC_EXIT;
        result = codes.codeResult;
        break;

    case TC_HARDERROR:   /* Hard error halt */
        why = APR_PROC_SIGNAL;
        result = SIGSYS;
        break;

    case TC_KILLPROCESS: /* Was killed by a DosKillProcess() */
        why = APR_PROC_SIGNAL;
        result = SIGKILL;
        break;

    case TC_TRAP:        /* TRAP in 16 bit code */
    case TC_EXCEPTION:   /* Threw an exception (32 bit code) */
        why = APR_PROC_SIGNAL;

        switch (codes.codeResult | XCPT_FATAL_EXCEPTION) {
        case XCPT_ACCESS_VIOLATION:
            result = SIGSEGV;
            break;

        case XCPT_ILLEGAL_INSTRUCTION:
            result = SIGILL;
            break;

        case XCPT_FLOAT_DIVIDE_BY_ZERO:
        case XCPT_INTEGER_DIVIDE_BY_ZERO:
            result = SIGFPE;
            break;

        default:
            result = codes.codeResult;
            break;
        }
    }

    if (exitcode) {
        *exitcode = result;
    }

    if (exitwhy) {
        *exitwhy = why;
    }
}



APR_DECLARE(apr_status_t) apr_proc_wait_all_procs(apr_proc_t *proc,
                                                  int *exitcode,
                                                  apr_exit_why_e *exitwhy,
                                                  apr_wait_how_e waithow,
                                                  apr_pool_t *p)
{
    RESULTCODES codes;
    ULONG rc;
    PID pid;

    rc = DosWaitChild(DCWA_PROCESSTREE, waithow == APR_WAIT ? DCWW_WAIT : DCWW_NOWAIT, &codes, &pid, 0);

    if (rc == 0) {
        proc->pid = pid;
        proces_result_codes(codes, exitcode, exitwhy);
        return APR_CHILD_DONE;
    } else if (rc == ERROR_CHILD_NOT_COMPLETE) {
        return APR_CHILD_NOTDONE;
    }

    return APR_OS2_STATUS(rc);
} 



APR_DECLARE(apr_status_t) apr_proc_wait(apr_proc_t *proc,
                                        int *exitcode, apr_exit_why_e *exitwhy,
                                        apr_wait_how_e waithow)
{
    RESULTCODES codes;
    ULONG rc;
    PID pid;
    rc = DosWaitChild(DCWA_PROCESS, waithow == APR_WAIT ? DCWW_WAIT : DCWW_NOWAIT, &codes, &pid, proc->pid);

    if (rc == 0) {
        proces_result_codes(codes, exitcode, exitwhy);
        return APR_CHILD_DONE;
    } else if (rc == ERROR_CHILD_NOT_COMPLETE) {
        return APR_CHILD_NOTDONE;
    }

    return APR_OS2_STATUS(rc);
} 



APR_DECLARE(apr_status_t) apr_proc_detach(int daemonize)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_procattr_user_set(apr_procattr_t *attr, 
                                                const char *username,
                                                const char *password)
{
    return APR_ENOTIMPL;
}

APR_DECLARE(apr_status_t) apr_procattr_group_set(apr_procattr_t *attr,
                                                 const char *groupname)
{
    return APR_ENOTIMPL;
}
