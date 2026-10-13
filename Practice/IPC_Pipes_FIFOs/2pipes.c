#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#decine BUFF_SIZE 4096

int main(int argc, char *argv[]){
    int ptoc_fd[2];
    int ctop_fd[2];

    if(pipe(ptoc_fd)==-1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if(pipe(ctop_fd)==-1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if(pid==-1){
        perror("fork failure");
        exit(EXIT_FAILURE);
    }

    if(pid == 0){
        close(ptoc[1]);
        close(ctop[0]);

        int fd2 = open(argv[2],O_WRONLY);
        ssize_t bytes = read(ptoc_fd,buf, sizeof(buf)-1);


    }else{
        close(ptoc[0]);
        close(ctop[1]);
        int fd1 = open(argv[1],O_RDONLY);

        char buf[BUFF_SIZE];
        ssize_t bytes;
        while((bytes = read(fd1,buf, BUFF_SIZE))>0){
            write(ptoc_fd[1],buf,bytes);
        }

        close(ptoc[1]);
        close(ctop[0]);
        close(fd1);
    }

}