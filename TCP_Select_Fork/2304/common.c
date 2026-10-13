#include "common.h"

int parse_port(char *port){
    int port_no = atoi(port);
    if(port_no < 1024 || port_no > 65535){
        fprintf(stderr, "Given port is illegal or privileged. Use a port between 1024 and 65535\n");
        exit(EXIT_FAILURE);
    }
    return port_no;
}

int set_socketaddr(struct sockaddr_in *addr, char *ip, int port){
    bzero(addr, sizeof(struct sockaddr_in));
    addr->sin_family = AF_INET;

    if(inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
        fprintf(stderr, "Given IP Address is not in a valid presentation format\n");
        exit(EXIT_FAILURE);
    }

    unsigned int port_no = (unsigned int)port;
    addr->sin_port = htons(port_no);
    return 0;
}

int create_listening_socket(char *ip, int port){
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        perror("socket:");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    set_socketaddr(&server_addr, ip, port);

    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind:");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if(listen(sockfd, MAX_CONNECTIONS) < 0){
        perror("listen:");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    return sockfd;
}

ssize_t read_message(int sockfd, Message *msg){
    ssize_t bytesRead = read(sockfd, msg, sizeof(Message));
    if(bytesRead == sizeof(Message)){
        msg->seq_no = ntohl(msg->seq_no);
        msg->reserved = ntohs(msg->reserved);
    }
    msg->data[MAX - 1] = '\0';
    return bytesRead;
}

ssize_t write_message(int sockfd, Message msg){
    msg.seq_no = htonl(msg.seq_no);
    msg.reserved = htons(msg.reserved);
    ssize_t bytesWritten = write(sockfd, &msg, sizeof(Message));
    return bytesWritten;
}

void format_addr(const struct sockaddr_in *addr, char *out, size_t len){
    char ip[INET_ADDRSTRLEN];
    unsigned int port_no = ntohs(addr->sin_port);

    if(inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)) == NULL){
        snprintf(ip, sizeof(ip), "unknown");
    }

    snprintf(out, len, "%s:%u", ip, port_no);
}