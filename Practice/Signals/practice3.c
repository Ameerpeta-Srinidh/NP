#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>
int counter = 0;
int reaped = 0;
void sigchld_handler(int signo){
    int status;
    pid_t pid;

    while((pid=waitpid(-1,&status,WNOHANG))>0){
        reaped++;
        printf("Child %d reaped\n",pid);
    }
}
void sigusr1_handler(int signo){
    counter++;
    printf("child event received: %d\n",signo);
}
int main(){
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD,&sa,NULL);
    struct sigaction sa2;
    sa2.sa_handler = sigusr1_handler;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_flags = SA_RESTART;
    sigaction(SIGUSR1,&sa2,NULL);
    for(int i=0;i<3;i++){
        pid_t pid = fork();
        if(pid==0){
            printf("child : %d, pid = %d\n",i+1,getpid());
            sleep(i+1);
            kill(getppid(),SIGUSR1);
            exit(0);
        }
    }
    printf("parent pid = %d\n",getpid());
    while(counter<3 && reaped<3){
        pause();
    }
}