#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX 256
#define STDOUT 1

int main(int argc, char **argv)
{
	pid_t	        pid;
	int             fd[2];

	if (pipe(fd) < 0) {
		fprintf(stderr, "Error opening pipe\n");
		exit(2);
	}

	if ((pid = fork()) == 0) {
		printf("In Child Process...fd[0] = %d, fd[1] = %d\n", fd[0], fd[1]);
	}
	else if (pid > 0) {
		printf("In Parent Process(child pid = %d), fd[0] = %d, fd[1] = %d\n", 
				pid, fd[0], fd[1]);
		wait(NULL);
	}
	else {
		fprintf(stderr, "Error in fork\n");
		exit(1);
	}

	printf("Exiting from process %d\n", getpid());
	exit(0);
}
