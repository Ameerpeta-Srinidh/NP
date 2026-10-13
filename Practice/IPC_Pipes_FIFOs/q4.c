#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/types.h>
#include<errno.h>

#define FIFO "/tmp/fifo.1"
#define BUFF_SIZE 4096

void sigpipe_handler(int signo){
    printf("SIGPIPE Error: Reader not connected\n");
    exit(EXIT_SUCCESS);
}

void producer(char *inputFilename){
    signal(SIGPIPE,sigpipe_handler);
    if((mkfifo(FIFO,0666))<0 && errno != EEXIST){
        perror("mkfifo failure");
        exit(1);
    }
    int readfd;
    if((readfd = open(inputFilename, O_RDONLY))<0){
        fprintf(stderr,"can't open %s",inputFilename);
        exit(1);
    }
    int writefd;
   if(( writefd = open(FIFO,O_WRONLY | O_NONBLOCK))<0){
    if(errno == ENXIO){
        printf("no reader connected");
        close(readfd);
        exit(1);
    }
   }
   char buf[BUFF_SIZE];
   int n;
   while((n=read(readfd,buf,BUFF_SIZE))>0){
      if(write(writefd,buf,n)==-1){
        perror("write error, or reader not connected");
        exit(1);
      }
   }
   close(readfd);
   close(writefd);

}

void consumer(char *outputFilename){
    if(mkfifo(FIFO,0666)<0 && errno != EEXIST){
        perror("mkfifo failure");
        exit(1);
    }
    int readfd;
    if((readfd = open(FIFO,O_RDONLY))<0){
        fprintf(stderr,"can't open %s for reading",FIFO);
        exit(1);
    }
    int writefd;
    if((writefd=open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC, 0666))<0){
        perror("can't open for writing");
        exit(1);
    }
    char buf[BUFF_SIZE];
    int n;
    while((n = read(readfd,buf,BUFF_SIZE)) > 0){
        for(int i=0;i<n;i++){
            if(buf[i]=='\n'){
                buf[i]=
            }
        }
        write(writefd,buf,n);
    }

    close(readfd);
    close(writefd);
}

int main(int argc, char* argv[]){
    if(strcmp(argv[1], "-r") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Usage for consumer: %s -r <output_file>\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        consumer(argv[2]);
    } else {
        producer(argv[1]);
    }
}