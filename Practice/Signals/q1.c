#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

int main(){
    pid_t children[3];
    for(int i=0;i<4;i++){
        pid_t pid = fork();
        if(pid==0){
            sigset_t set;
            sigemptyset(&set);
            sigaddset(&set,SIGUSR1);
            sigprocmask(SIG_BLOCK,&set,NULL);
            sleep((i+1)*2);
            sigprocmask(SIG_UNBLOCK,&set,NULL);
            pause();
            exit(0);
        }
        else{
            children[i] = pid;
        }
    }
    for(int i=3;i>=0;i--){
        kill(children[i],SIGUSR1);
    }

}