#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int global = 10;

int main(int argc, char **argv)
{
    pid_t	pid;

    if ((pid = fork()) == 0) {
        printf("In Child Process: ");
        global += 5;
        //sleep(5);
    } else if (pid > 0) {
        printf("In Parent Process: ");
        global += 10;
    } else {
        fprintf(stderr, "Error in fork\n");
        exit(1);
	}

    printf("pid = %d, ppid = %d\n", getpid(), getppid());
    printf("Global = %d\n", global);
    exit(0);
}
