#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#define MAX 256
#define MAX_CONNECTIONS 5

typedef struct mesg{
    unsigned int seq_no;
    unsigned short reserved;
    unsigned char data[MAX];
} Message;

int parse_port(char *port);
int set_socketaddr(struct sockaddr_in *addr, char *ip, int port);
int create_listening_socket(char *ip, int port);
ssize_t read_message(int sockfd, Message *msg);
ssize_t write_message(int sockfd, Message msg);
void format_addr(const struct sockaddr_in *addr, char *out, size_t len);

#endif