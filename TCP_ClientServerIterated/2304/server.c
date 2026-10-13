#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include<fcntl.h>
#include<signal.h>
#include <unistd.h>
#define MAX 256

typedef struct mesg{
    unsigned int seq_no;
    unsigned short reserved;
    unsigned char data[MAX];
}Message;

int main(int argc, char* argv[]){
    if(argc!=3){
        printf("Usage: executable, Server IP Address, Server Port Number\n");
        exit(EXIT_FAILURE);
    }
    int listenfd,connfd;
    struct sockaddr_in client_addr,server_addr;
    socklen_t len = sizeof(client_addr);
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd==-1){
        printf("Error creating socket\n");
        exit(EXIT_FAILURE);
    }
    bzero(&server_addr, sizeof(server_addr));
    char* servaddr = argv[1];
    char* serv_port = argv[2];
    server_addr.sin_family = AF_INET;
    inet_aton(servaddr,&server_addr.sin_addr);
    server_addr.sin_port = htons(atoi(serv_port));
    int retval = bind(listenfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
    if(retval<0){
        printf("error in bind\n");
        exit(EXIT_FAILURE);
    }
    int lisretvel = listen(listenfd,10);
    if(lisretvel<0){
        printf("error in listen\n");
        exit(EXIT_FAILURE);
    }
   
    for(;;){
       
        connfd = accept(listenfd,(struct sockaddr*)&client_addr,&len);
        if(connfd<0){
            printf("error in accept\n");
            exit(EXIT_FAILURE);
        }
        printf("Connection from client %s\n",inet_ntoa(client_addr.sin_addr));
        ssize_t bytesRead,bytesWritten;
        Message buf;
        while((bytesRead = read(connfd,&buf,sizeof(buf)))>0){
        
        printf("Read from Client: %d %s\n",buf.seq_no,buf.data);
        buf.seq_no += 1;
        bytesWritten = write(connfd,&buf,sizeof(buf));
        if(bytesWritten<=0){
            break;
        }
        printf("Written to Client: %d %s\n",buf.seq_no,buf.data);
    }
    if(bytesRead<0){
            printf("Error reading from client\n");
            exit(EXIT_FAILURE);
        }
        if(bytesRead==0){
            printf("Client disconnected\n");
        }
        close(connfd);
    }
    
}