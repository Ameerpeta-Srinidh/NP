#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	pid_t   pid;
	int     rv;

	if ((pid = fork()) < 0) {
		fprintf(stderr, "Error forking\n");
		exit(1);
	} else if (pid == 0) {
		/* Child */     
		rv = execl("hello", "hello", NULL);
		if (rv < 0) {
			fprintf(stderr, "Error execing\n");
			exit(2);
		}
	} else { /* Parent */
		wait(NULL);
		printf("Exiting Parent...\n");
	}

	exit(0);
}


