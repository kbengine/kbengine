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

#include <kernel/OS.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

struct pipefd {
	int in;
	int out;
	int err;
};

int main(int argc, char *argv[]) {
/* we expect the following...
 * 
 * argv[0] = this stub
 * argv[1] = directory to run in...
 * argv[2] = progname to execute
 * rest of arguments to be passed to program
 */
	char *progname = argv[2];
	char *directory = argv[1];
	struct pipefd *pfd;
	thread_id sender;
	void *buffer;
	char ** newargs;
	int i = 0;
	
	newargs = (char**)malloc(sizeof(char*) * (argc - 1));
  
	buffer = (void*)malloc(sizeof(struct pipefd));
	/* this will block until we get the data */
	receive_data(&sender, buffer, sizeof(struct pipefd));
	pfd = (struct pipefd*)buffer;
	
	if (pfd->in > STDERR_FILENO) {
		if (dup2(pfd->in, STDIN_FILENO) != STDIN_FILENO) return (-1);
	    close (pfd->in);
	}
	if (pfd->out > STDERR_FILENO) {
		if (dup2(pfd->out, STDOUT_FILENO) != STDOUT_FILENO) return (-1);
	    close (pfd->out);
	}
	if (pfd->err > STDERR_FILENO) {
		if (dup2(pfd->err, STDERR_FILENO) != STDERR_FILENO) return (-1);
	    close (pfd->err);
	}

	for	(i=3;i<=argc;i++){
	    newargs[i-3] = argv[i];
	}
	
	/* tell the caller we're OK to start */
	send_data(sender,1,NULL,0);

	if (directory != NULL)
		chdir(directory);
	execve (progname, newargs, environ);

	return (-1);
}
