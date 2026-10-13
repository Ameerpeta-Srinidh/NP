#ifndef TCPHEADER_H
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{
    unsigned short source_port:16;
    unsigned short destination_port:16;
    unsigned int window:16;
    unsigned int urgentpointer:16;
    unsigned int checksum:16;
    unsigned int seq_no:32;
    unsigned int ack:32;
    unsigned short HLEN:4;
    unsigned short Reserved:6;
    unsigned short URG:1;
    unsigned short ACK:1;
    unsigned short PSH:1;
    unsigned short RST:1;
    unsigned short SYN:1;
    unsigned short FIN:1;

}TCPHeader;

#endif