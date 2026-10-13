#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "tcp_header.h"

int main(int argc, char **argv){
    if(argc! = 2){
        fprintf(stderr,"Usage: ./server <Port No>\n");
        exit(0);
    }
    struct sockaddr_in server_addr, client_addr;
    int sockfd;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd<0){
        perror("socket:\n");
        exit(0);
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = htonl(INADDR_ANY);
    bind(sockfd, (struct sockaddr*)&server_addr,sizeof(server_addr));


    TCPHeader tcp_h,tcph_reply;
    
    size_t n = recvfrom(sockfd,tcp_h,sizeof(tcp_h),0,(struct sockaddr*)&client_addr,sizeof(client_addr));
}