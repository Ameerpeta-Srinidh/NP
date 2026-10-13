#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<signal.h>
#include<unistd.h>
#define MAX 256
#define MAX_SIZE 4096
typedef struct mesg{
    unsigned int seq_no;
    unsigned short reserved;
    unsigned char data[MAX];
}Message;

int main(int argc, char* argv[]){
    if(argc!=4){
        printf("Usage: executable, Server IP Address, Server Port Number, Message\n");
        exit(EXIT_FAILURE);
    }
    int socketfd;
     struct sockaddr_in server_addr;
    char* serv_addr = argv[1];
    printf("%s\n",serv_addr);

    socketfd = socket(AF_INET,SOCK_STREAM,0);
    if(socketfd == -1){
        printf("Error creating a socket\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET,serv_addr,&server_addr.sin_addr);
    int serv_port = atoi(argv[2]);
    server_addr.sin_port = htons(serv_port);

    if(connect(socketfd,(struct sockaddr*)&server_addr,sizeof(server_addr))==-1){
        printf("Error: connect\n");
        exit(EXIT_FAILURE);
    }
    char* string = argv[3];
    Message msg;
    msg.seq_no = 10;
    msg.reserved = 0;
    strncpy((char *)msg.data,string,sizeof(msg.data)-1);
    msg.data[sizeof(msg.data)-1] ='\0';
    int counter = 0;
    
    while(counter<100){
        Message buf;
        ssize_t bytesWritten,bytesRead;
        bytesWritten = write(socketfd,&msg,sizeof(Message));
        if(bytesWritten<=0){
            printf("error writing to socket\n");
            exit(EXIT_FAILURE);
        }
        bytesRead = read(socketfd,&buf,sizeof(buf));
         
        if(bytesRead<=0){
            printf("Error reading from server\n");
            exit(EXIT_FAILURE);
        }
        printf("Sent to Server: %d %s\n",msg.seq_no,msg.data);
        printf("Recieved from Server: %d %s\n",buf.seq_no,buf.data);
       
        msg.seq_no = buf.seq_no;
        counter++;

    }
    close(socketfd);
}