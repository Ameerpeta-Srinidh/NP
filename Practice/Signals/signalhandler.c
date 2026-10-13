#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void sigusr1_handler(int signo){
    printf("caught the signal %d\n",signo);
}

void sigusr2_handler(int signo){
    printf("caught the signal %d\n",signo);
}

int main(void){
    printf("my pid is %d\n", getpid());
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGUSR1);
    sigaddset(&set,SIGUSR2);
    printf("BLOCKED. sleeping for 15 seconds\n");
    sigprocmask(SIG_BLOCK,&set, NULL);
    sleep(15);
    sigset_t pendingset;
    sigpending(&pendingset);
    if(sigismember(&pendingset,SIGUSR1)){
        printf("currently pending - sigusr1\n");
    }
    if(sigismember(&pendingset,SIGUSR2)){
        printf("currently pending - sigusr2\n");
    }
    sigset_t allowedset;
    sigemptyset(&allowedset);
    sigaddset(&allowedset, SIGUSR1);
    sigprocmask(SIG_UNBLOCK,&allowedset, NULL);

        pause();
        printf("program ending..\n");
        return 0;


}
