#include "common.h"
#include <sys/select.h>

int main(int argc, char **argv){
    if(argc != 3){
        fprintf(stderr, "Usage: ./server_select <Server IP Addr> <Server Port No>\n");
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    
    char *servaddr = argv[1];
    int port_no = parse_port(argv[2]);
    int sockfd = create_listening_socket(servaddr, port_no);

    int i, maxi, maxfd, connfd, new_sockfd;
    int client[MAX_CONNECTIONS];
    struct sockaddr_in client_addr_list[MAX_CONNECTIONS];
    ssize_t n;
    fd_set rset, allset;
    socklen_t clientlen;
    struct sockaddr_in client_addr;
    Message buf;

    maxfd = sockfd;
    maxi = -1;
    for(i = 0; i < MAX_CONNECTIONS; i++){
        client[i] = -1;
    }
    FD_ZERO(&allset);
    FD_SET(sockfd, &allset);

    for(;;){
        rset = allset;
        int nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if(nready < 0){
            if(errno == EINTR)
                continue;
            perror("select:");
            exit(EXIT_FAILURE);
        }

        if(FD_ISSET(sockfd, &rset)){
            clientlen = sizeof(client_addr);
            connfd = accept(sockfd, (struct sockaddr *)&client_addr, &clientlen);
            if(connfd < 0){
                perror("accept:");
                continue;
            }

            for(i = 0; i < MAX_CONNECTIONS; i++){
                if(client[i] < 0){
                    client[i] = connfd;
                    break;
                }
            }

            if(i == MAX_CONNECTIONS){
                fprintf(stderr, "Client limit exceeded\n");
                close(connfd);
            }else{
                client_addr_list[i] = client_addr;
                FD_SET(connfd, &allset);
                if(connfd > maxfd) maxfd = connfd;
                if(i > maxi) maxi = i;
            }

            if(--nready <= 0) continue;
        }

        for(i = 0; i <= maxi; i++){
            new_sockfd = client[i];
            if(new_sockfd < 0)
                continue;

            if(FD_ISSET(new_sockfd, &rset)){
                char client_address[INET_ADDRSTRLEN + 6];
                /*my format_addr() function adds a colon and port number so i'm using 16+6 total bytes*/
                format_addr(&client_addr_list[i], client_address, sizeof(client_address));
                n = read_message(new_sockfd, &buf);
                if(n == sizeof(Message)){
                    printf("Read from Client %s: Seq No = %u, Message = %s\n",client_address, buf.seq_no, buf.data);
                    unsigned int seqno = buf.seq_no;
                    seqno++;
                    buf.seq_no = seqno;

                    printf("Writing to Client %s: Seq No = %u, Message = %s\n", client_address, seqno, buf.data);
                    ssize_t bytesWritten = write_message(new_sockfd, buf);
                    if(bytesWritten != sizeof(Message)){
                        fprintf(stderr, "Error writing to Client %s\n", client_address);
                        if(bytesWritten < 0)
                            perror("write:");
                        else
                            fprintf(stderr, "Incomplete message sent to Client %s\n", client_address);
                        fprintf(stderr,"Client %s exiting\n", client_address);
                        close(new_sockfd);
                        FD_CLR(new_sockfd, &allset);
                        client[i] = -1;
                        if(--nready <= 0)
                            break;
                        continue;
                    }
                }else if(n == 0){
                    fprintf(stderr,"Client %s exiting\n", client_address);
                    close(new_sockfd);
                    FD_CLR(new_sockfd, &allset);
                    client[i] = -1;
                }else{
                    if(n>0)
                        fprintf(stderr, "Incomplete message received from Client %s\n", client_address);
                    else
                        fprintf(stderr, "Error reading from Client %s\n", client_address);
                    perror("read:");
                    fprintf(stderr,"Client %s exiting\n", client_address);
                    close(new_sockfd);
                    FD_CLR(new_sockfd, &allset);
                    client[i] = -1;
                }
                if(--nready <= 0)
                    break;
            }
        }
    }

    return 0;
}
