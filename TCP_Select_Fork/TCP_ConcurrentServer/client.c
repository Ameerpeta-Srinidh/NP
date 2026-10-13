#include "common.h"

int main(int argc, char* argv[]){
    if(argc != 4){
        fprintf(stderr, "Usage: ./client <Server IP Address> <Server Port Number> <Message>\n");
        exit(EXIT_FAILURE);
    }

    char *serv_addr = argv[1];
    int serv_port = parse_port(argv[2]);
    char *string = argv[3];

    if(strlen(string) >= MAX){
        fprintf(stderr, "String length is too long for the message buffer\n");
        exit(EXIT_FAILURE);
    }

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if(socketfd == -1){
        perror("socket:");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    set_socketaddr(&server_addr, serv_addr, serv_port);

    if(connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
        perror("connect:");
        close(socketfd);
        exit(EXIT_FAILURE);
    }

    Message msg;
    bzero(&msg, sizeof(msg));
    srand(time(NULL) ^ getpid());
    msg.seq_no = rand();
    msg.reserved = 0;
    strncpy((char *)msg.data, string, sizeof(msg.data) - 1);
    msg.data[sizeof(msg.data) - 1] = '\0';

    int counter = 0;
    while(counter < 5){
        Message buf;
        ssize_t bytesWritten, bytesRead;
        bytesWritten = write_message(socketfd, msg);
        if(bytesWritten < 0){
            perror("write:");
            close(socketfd);
            exit(EXIT_FAILURE);
        }
        if(bytesWritten != sizeof(Message)){
            fprintf(stderr, "Incomplete message sent to server\n");
            close(socketfd);
            exit(EXIT_FAILURE);
        }

        bytesRead = read_message(socketfd, &buf);
        if(bytesRead < 0){
            perror("read:");
            close(socketfd);
            exit(EXIT_FAILURE);
        }
        if(bytesRead == 0){
            fprintf(stderr, "Server closed the connection\n");
            close(socketfd);
            exit(EXIT_FAILURE);
        }
        if(bytesRead != sizeof(Message)){
            fprintf(stderr, "Incomplete message received from server\n");
            close(socketfd);
            exit(EXIT_FAILURE);
        }

        printf("Sent to Server: Seq No = %u, Message = %s\n", msg.seq_no, msg.data);
        printf("Received from Server: Seq No = %u, Message = %s\n", buf.seq_no, buf.data);

        msg.seq_no = buf.seq_no;
        sleep(5);
        counter++;
    }

    close(socketfd);
    exit(EXIT_SUCCESS);
}
