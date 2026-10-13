#include "common.h"

int main(int argc, char* argv[]){
    if(argc != 3){
        fprintf(stderr, "Usage: ./server_fork <Server IP Address> <Server Port Number>\n");
        exit(EXIT_FAILURE);
    }

    char *servaddr = argv[1];
    int serv_port = parse_port(argv[2]);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    int listenfd = create_listening_socket(servaddr, serv_port);
    int connfd;
    struct sockaddr_in client_addr;
    pid_t pid;
    socklen_t len = sizeof(client_addr);

    for(;;){
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len);
        if(connfd < 0){
            if(errno == EINTR)
                continue;
            perror("accept:");
            continue;
        }

        pid = fork();
        if(pid == 0){
            char client_address[INET_ADDRSTRLEN + 6];
            close(listenfd);

            format_addr(&client_addr, client_address, sizeof(client_address));

            ssize_t bytesRead, bytesWritten;
            Message buf;

            while((bytesRead = read_message(connfd, &buf)) > 0){
                if(bytesRead != sizeof(Message)){
                    fprintf(stderr, "Incomplete message received from Client %s\n", client_address);
                    break;
                }

                printf("Read from Client %s: Seq No = %u, Message = %s\n",client_address, buf.seq_no, buf.data);

                unsigned int seq_no = buf.seq_no;
                seq_no++;
                buf.seq_no = seq_no;

                printf("Writing to Client %s: Seq No = %u, Message = %s\n",client_address, seq_no, buf.data);
                bytesWritten = write_message(connfd, buf);
                if(bytesWritten < 0){
                    perror("write:");
                    fprintf(stderr, "Error writing to Client %s\n", client_address);
                    break;
                }
                if(bytesWritten != sizeof(Message)){
                    fprintf(stderr, "Incomplete message sent to Client %s\n", client_address);
                    break;
                }
            }

            if(bytesRead < 0){
                fprintf(stderr, "Error reading from Client %s\n", client_address);
            }
            
            if(bytesRead == 0){
                fprintf(stderr, "Client %s closed the connection\n", client_address);
            }else if(bytesRead > 0 && bytesRead != sizeof(Message)){
                fprintf(stderr, "Client %s sent an incomplete message\n", client_address);
            }
            close(connfd);
            exit(EXIT_SUCCESS);
        }

        if(pid < 0){
            perror("fork:");
            close(connfd);
            continue;
        }

        close(connfd);
    }
}
