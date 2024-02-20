#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main ()
{
	/* [0] is the read end, [1] is the write end */
	int parent_to_child[2], child_to_parent[2];
	pid_t pid;

	if (pipe(parent_to_child) == -1) {
		fprintf(stderr, "Pipe failed\n");
		exit(EXIT_FAILURE);
	}

	if (pipe(child_to_parent) == -1) {
		fprintf(stderr, "Pipe failed\n");
		exit(EXIT_FAILURE);
	}

	if ((pid = fork()) == -1) {
		fprintf(stderr, "Fork failed\n");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) { /* Child code */
		close(parent_to_child[1]); /* Closes unused write end of pipe */
		close(child_to_parent[0]); /* Closes unused read end of pipe */
		
		char *buf = (char *)malloc(8);	/* No need for additional space given ping/pong */
		read(parent_to_child[0], buf, 5);

		printf("PID: %d; received %s\n", getpid(), buf);
		free(buf);

		write(child_to_parent[1], "pong", 5);

		close(child_to_parent[1]);
		close(parent_to_child[0]);

		exit(0);

	} else { /* Parent code */
		char buffer[8];		/* On the runtime stack -- local collection */
		int status;		/* Capturing result of child exiting */
		
		close(parent_to_child[0]); /* Closes unused write end of pipe */
		close(child_to_parent[1]); /* Closes unused read end of pipe */

		write(parent_to_child[1], "ping", 5);
		read(child_to_parent[0], buffer, 5);
		printf("PID: %d; received %s\n", getpid(), buffer);

		close(parent_to_child[1]);
		close(child_to_parent[0]);

		wait(&status);
		exit(0);
	}
	
	return 0;		/* Unreachable code */
}
