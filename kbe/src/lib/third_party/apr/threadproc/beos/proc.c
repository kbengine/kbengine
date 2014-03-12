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
#include "apr_strings.h"

/* Heavy on no'ops, here's what we want to pass if there is APR_NO_FILE
 * requested for a specific child handle;
 */
static apr_file_t no_file = { NULL, -1, };

struct send_pipe {
	int in;
	int out;
	int err;
};

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
    (*new)->detached = 0;
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

APR_DECLARE(apr_status_t) apr_procattr_dir_set(apr_procattr_t *attr, 
                                               const char *dir) 
{
    char * cwd;
    if (dir[0] != '/') {
        cwd = (char*)malloc(sizeof(char) * PATH_MAX);
        getcwd(cwd, PATH_MAX);
        attr->currdir = (char *)apr_pstrcat(attr->pool, cwd, "/", dir, NULL);
        free(cwd);
    } else {
        attr->currdir = (char *)apr_pstrdup(attr->pool, dir);
    }
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
		/* This is really ugly...
		 * The semantics of BeOS's fork() are that areas (used for shared
		 * memory) get COW'd :-( The only way we can make shared memory
		 * work across fork() is therefore to find any areas that have
		 * been created and then clone them into our address space.
         * Thankfully only COW'd areas have the lock variable set at
         * anything but 0, so we can use that to find the areas we need to
         * copy. Of course what makes it even worse is that the loop through
         * the area's will go into an infinite loop, eating memory and then
         * eventually segfault unless we know when we reach then end of the
         * "original" areas and stop. Why? Well, we delete the area and then
         * add another to the end of the list...
		 */
		area_info ai;
		int32 cookie = 0;
        area_id highest = 0;
		
        while (get_next_area_info(0, &cookie, &ai) == B_OK)
            if (ai.area	> highest)
                highest = ai.area;
        cookie = 0;
        while (get_next_area_info(0, &cookie, &ai) == B_OK) {
            if (ai.area > highest)
                break;
            if (ai.lock > 0) {
                area_id original = find_area(ai.name);
                delete_area(ai.area);
                clone_area(ai.name, &ai.address, B_CLONE_ADDRESS,
                           ai.protection, original);
            }
        }
		
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

APR_DECLARE(apr_status_t) apr_proc_create(apr_proc_t *new, const char *progname, 
                                          const char * const *args,
                                          const char * const *env, 
                                          apr_procattr_t *attr, 
                                          apr_pool_t *pool)
{
    int i=0,nargs=0;
    char **newargs = NULL;
    thread_id newproc, sender;
    struct send_pipe *sp;        
    char * dir = NULL;
	    
    sp = (struct send_pipe *)apr_palloc(pool, sizeof(struct send_pipe));

    new->in = attr->parent_in;
    new->err = attr->parent_err;
    new->out = attr->parent_out;
    sp->in  = attr->child_in  ? attr->child_in->filedes  : FILENO_STDIN;
    sp->out = attr->child_out ? attr->child_out->filedes : FILENO_STDOUT;
    sp->err = attr->child_err ? attr->child_err->filedes : FILENO_STDERR;

    i = 0;
    while (args && args[i]) {
        i++;
    }

	newargs = (char**)malloc(sizeof(char *) * (i + 4));
	newargs[0] = strdup("/boot/home/config/bin/apr_proc_stub");
    if (attr->currdir == NULL) {
        /* we require the directory , so use a temp. variable */
        dir = malloc(sizeof(char) * PATH_MAX);
        getcwd(dir, PATH_MAX);
        newargs[1] = strdup(dir);
        free(dir);
    } else {
        newargs[1] = strdup(attr->currdir);
    }
    newargs[2] = strdup(progname);
    i=0;nargs = 3;

    while (args && args[i]) {
        newargs[nargs] = strdup(args[i]);
        i++;nargs++;
    }
    newargs[nargs] = NULL;

    /* ### we should be looking at attr->cmdtype in here... */

    newproc = load_image(nargs, (const char**)newargs, (const char**)env);

    /* load_image copies the data so now we can free it... */
    while (--nargs >= 0)
        free (newargs[nargs]);
    free(newargs);
        
    if (newproc < B_NO_ERROR) {
        return errno;
    }

    resume_thread(newproc);

    if (attr->child_in && (attr->child_in->filedes != -1)) {
        apr_file_close(attr->child_in);
    }
    if (attr->child_out && (attr->child_in->filedes != -1)) {
        apr_file_close(attr->child_out);
    }
    if (attr->child_err && (attr->child_in->filedes != -1)) {
        apr_file_close(attr->child_err);
    }

    send_data(newproc, 0, (void*)sp, sizeof(struct send_pipe));
    new->pid = newproc;

    /* before we go charging on we need the new process to get to a 
     * certain point.  When it gets there it'll let us know and we
     * can carry on. */
    receive_data(&sender, (void*)NULL,0);
    
    return APR_SUCCESS;
}

APR_DECLARE(apr_status_t) apr_proc_wait_all_procs(apr_proc_t *proc,
                                                  int *exitcode,
                                                  apr_exit_why_e *exitwhy,
                                                  apr_wait_how_e waithow, 
                                                  apr_pool_t *p)
{
    proc->pid = -1;
    return apr_proc_wait(proc, exitcode, exitwhy, waithow);
} 

APR_DECLARE(apr_status_t) apr_proc_wait(apr_proc_t *proc,
                                        int *exitcode, 
                                        apr_exit_why_e *exitwhy,
                                        apr_wait_how_e waithow)
{
    pid_t pstatus;
    int waitpid_options = WUNTRACED;
    int exit_int;
    int ignore;
    apr_exit_why_e ignorewhy;

    if (exitcode == NULL) {
        exitcode = &ignore;
    }
    if (exitwhy == NULL) {
        exitwhy = &ignorewhy;
    }

    if (waithow != APR_WAIT) {
        waitpid_options |= WNOHANG;
    }

    if ((pstatus = waitpid(proc->pid, &exit_int, waitpid_options)) > 0) {
        proc->pid = pstatus;
        if (WIFEXITED(exit_int)) {
            *exitwhy = APR_PROC_EXIT;
            *exitcode = WEXITSTATUS(exit_int);
        }
        else if (WIFSIGNALED(exit_int)) {
            *exitwhy = APR_PROC_SIGNAL;
            *exitcode = WTERMSIG(exit_int);
        }
        else {

            /* unexpected condition */
            return APR_EGENERAL;
        }
        return APR_CHILD_DONE;
    }
    else if (pstatus == 0) {
        return APR_CHILD_NOTDONE;
    }

    return errno;
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

    if (parent_in != NULL && rv == APR_SUCCESS)
        rv = apr_file_dup(&attr->parent_in, parent_in, attr->pool);

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
  
    if (parent_out != NULL && rv == APR_SUCCESS)
        rv = apr_file_dup(&attr->parent_out, parent_out, attr->pool);

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
  
    if (parent_err != NULL && rv == APR_SUCCESS)
        rv = apr_file_dup(&attr->parent_err, parent_err, attr->pool);

    return rv;
}

APR_DECLARE(apr_status_t) apr_procattr_limit_set(apr_procattr_t *attr, apr_int32_t what, 
                                                  void *limit)
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
