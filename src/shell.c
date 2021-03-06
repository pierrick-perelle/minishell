/*
 * Copyright (C) 2002, Simon Nieuviarts
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "readcmd.h"
#include "csapp.h"


int main()
{
	int status;

	while (1) {
		struct cmdline *l;
		int i, j;

		printf("shell> ");
		l = readcmd();

		/* If input stream closed, normal termination */
		if (!l || !strcmp(l->seq[0][0],"quit")) {
			printf("exit\n");
			exit(0);
		}

		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
			for (j=0; cmd[j]!=0; j++) {
				printf("%s ", cmd[j]);
			}
			printf("\n");
			

			if(Fork()==0){
				if(l->out){
					int fd_out = open(l->out, O_CREAT | O_WRONLY | O_TRUNC, 0744);
					if (fd_out == -1)
						perror("open");
					dup2(fd_out, 1);
					close(fd_out);
				}
				if(l->in){
					int fd_in = open(l->in, O_CREAT | O_RDONLY, 0744);
					if (fd_in == -1)
						perror("open");
					dup2(fd_in, 0);
					close(fd_in);
				}
				if (execvp(cmd[0],cmd)<0){
					fprintf(stderr,"%s : %s \n",cmd[0],"command not found");
					exit(1);
				}

			}else{
				Wait(&status);
			}

			


		}
	}
}