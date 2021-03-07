/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "readcmd.h"
#include "csapp.h"
#include <signal.h>


int main(){
	int status;

	int old_fds[2], new_fds[2];

	signal(SIGCHLD, SIG_IGN); //killing zombies see POSIX.1-2001 about SIGCHLD SIG_IGN.

	while (1) {
		struct cmdline *l;
		int i, j;

		printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l || !strcmp(l->seq[0][0],"quit")) {
			//printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		/*if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);*/


		/* Display each command of the pipe */

		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}

			if(l->seq[i+1] != NULL){
				/*there is a next command */
				pipe(new_fds);
			}

			printf("\n");
			if(Fork()==0){

				/* Fils */

				if(l->bg){
					/*is bg */
					setpgid(0, 0);
				}

				if(i > 0){
					/*there is a previous command */
					dup2(old_fds[0],0);
					close(old_fds[0]);
					close(old_fds[1]);
				}

				else if(l->in){
					/*there is an input*/
					int fd_in = open(l->in, O_CREAT | O_RDONLY, 0744);
					if (fd_in == -1){
						fprintf(stderr,"%s: %s \n",l->in,"permission denied");
					} else {
						dup2(fd_in, 0);
						close(fd_in);
					}
				}

				if(l->seq[i+1] != NULL){
					/*there is a next command */
					close(new_fds[0]);
            		dup2(new_fds[1], 1);
            		close(new_fds[1]);
				}

				else if(l->out){
					/* there is a output */
					int fd_out = open(l->out, O_CREAT | O_WRONLY | O_TRUNC, 0744);
					if (fd_out == -1){
						fprintf(stderr,"%s: %s \n",l->out,"permission denied");
						exit(1);
					} else {
						dup2(fd_out, 1);
						close(fd_out);
					}
				}

				if (execvp(cmd[0],cmd)<0){
					fprintf(stderr,"%s: %s \n",cmd[0],"command not found");
					exit(1);
				}

			}else{

				/* Père */

				if(i > 0){
					/*there is a previous command */
					close(old_fds[0]);
            		close(old_fds[1]);
				}

				if(l->seq[i+1] != NULL){
					/*there is a next command */
					old_fds[0] = new_fds[0];
					old_fds[1] = new_fds[1];
				}

			}
		}

		if(l->bg){
			printf("is bg \n");
			//do nothing.
		}

		else{
			printf("fg start \n");
			waitpid(0,&status,0);
			printf("fg done \n");
		}

		if(i > 1){
			/* multiple cmds */
			close(old_fds[0]);
    		close(old_fds[1]);
		}
	}
}
