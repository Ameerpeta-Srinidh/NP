#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>

void handler(int signo){
    printf("i got a signal %d\n",signo);
}

int main(void){
    signal(SIGUSR1, handler);
     printf("pid: %d\n",getpid());

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGUSR1);
    printf("block SIGUSR1 for 5 seconds\n");
    sigprocmask(SIG_BLOCK,&set,NULL);
    sleep(15);
    sigprocmask(SIG_UNBLOCK,&set,NULL);

    printf("unblocked now, waiting\n");

   
    while(1){
        sleep(2);
    }
}