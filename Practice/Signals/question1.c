#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void sigquit_handler(int signo){
    printf("received the signal - %d\n",signo);
    exit(0);
}
void sigusr1_handler(int signo){
    printf("received the signal - %d\n",signo);
}
void sigusr2_handler(int signo){
    printf("received the signal - %d\n",signo);
}

int main(void){
    printf("my pid is %d\n", getpid());
    signal(SIGQUIT, sigquit_handler);
    signal(SIGUSR1, sigusr1_handler);
    signal(SIGUSR2, sigusr2_handler);

    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_BLOCK,&set, NULL);
    printf("BLOCKED. sleeping for 30 seconds\n");
    int n = sleep(30);
    sigset_t pendingsignals;
    sigpending(&pendingsignals);
    printf("return value from sleep is %d\n",n);
    sigset_t unblockedset;
    sigemptyset(&unblockedset);
    sigaddset(&unblockedset,SIGUSR1);
    sigaddset(&unblockedset,SIGUSR2);
    sigaddset(&unblockedset,SIGQUIT);
    sigprocmask(SIG_UNBLOCK,&unblockedset,NULL);
    printf("UNBLOCKED. waiting for signals\n");
    while(1){
        pause();
    }

}