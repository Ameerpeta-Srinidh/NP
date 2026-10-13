#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>

int termCount = 0;
int hupCount = 0;
void handler(int signo){
    if(signo == SIGTERM){
        printf("Signal %d received\n",signo);
        termCount++;
    }else if(signo == SIGHUP){
        printf("Signal %d received\n",signo);
        hupCount++;
    }
    
}

int main(){
    printf("my pid is %d\n",getpid());
    sigset_t blockedset;
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigemptyset(&blockedset);
    sigaddset(&blockedset,SIGTERM);
    sigaddset(&blockedset,SIGHUP);
    sigprocmask(SIG_BLOCK,&blockedset,NULL);

    sigaction(SIGTERM,&sa,NULL);
    sigaction(SIGHUP,&sa,NULL);

    unsigned int n = sleep(15);
    printf("return value from sleep is %d\n",n);

    sigset_t pendingset;
    sigpending(&pendingset);
    if(sigismember(&pendingset,SIGTERM)){
        printf("pending signal : SIGTERM\n");
    }
    if(sigismember(&pendingset,SIGHUP)){
        printf("pending signal : SIGHUP\n");
    }

    sigset_t unblockedset;
    sigemptyset(&unblockedset);
    sigaddset(&unblockedset,SIGHUP);
    sigprocmask(SIG_UNBLOCK,&unblockedset,NULL);
   

    while(hupCount<2){
        pause();
    }

    printf("Final SIGTERM Count is %d\n",termCount);
    printf("Final SIGHUP Count is %d\n",hupCount);

    return 0;

}