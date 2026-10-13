#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "tcp_header.h"

int main(int argc, char **argv){
    if(argc != 3){
        printf("Usage: %s <IP_ADDRESS> <PORT_NUMBER>\n", argv[0]);
        return -1;
    }
    struct sockaddr_in server_addr;
    char* servaddr = argv[1];
    char* port_no = argv[2];

    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    inet_pton(AF_INET,servaddr,&server_addr.sin_addr);
    int sockfd = socket(AF_INET,SOCK_DGRAM,0);
    if(sockfd<0){
        perror("socket:\n");
        exit(0);
    }

    TCPHeader tcp_h;
    TCPHeader tcph_reply;
    int n = 0;
    tcp_h.source_port = 14820;
    tcp_h.destination_port = 80;
    tcp_h.seq_no = 1;
    tcp_h.ack = 1;
    tcp_h.HLEN = 5;
    tcp_h.Reserved = 0;
    tcp_h.URG = 0;
    tcp_h.ACK = 0;
    tcp_h.PSH = 0;
    tcp_h.RST = 0;
    tcp_h.SYN = 1;
    tcp_h.FIN = 0;
    tcp_h.window = 8192;
    tcp_h.checksum = 0;
    tcp_h.urgentpointer = 0;

    sendto(sockfd,tcp_h,sizeof(tcp_h),0,(struct sockaddr*)&server_addr,sizeof(server_addr));
    n = recvfrom(sockfd, tcph_reply, sizeof(tcph_reply), 0, NULL, NULL);

    close(sockfd);
    exit(0);
}
