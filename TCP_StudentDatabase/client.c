#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include <arpa/inet.h>
#include "student.h"
#include "parser.h"
#include "api.h"

int sockfd = -1;

int stub(ApiRequest *req){
    write(sockfd, req, sizeof(ApiRequest));
    ApiResponse reply;
    read(sockfd, &reply, sizeof(ApiResponse));
    return reply.status;
}

int addStudent(int rollNo, char name[], float cgpa){
    ApiRequest req;
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_ADD_STUDENT;
    req.rollNo = rollNo;
    strncpy(req.name, name, sizeof(req.name)-1);
    req.name[sizeof(req.name) - 1] = '\0';
    req.cgpa = cgpa;
    return stub(&req);
}

int deleteStudent(int rollNo){
    ApiRequest req;
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_DELETE_STUDENT;
    req.rollNo = rollNo;
    return stub(&req);
}

int modifyStudent(int rollNo, float updatedCgpa){
    ApiRequest req;
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_MODIFY_STUDENT;
    req.rollNo = rollNo;
    req.updatedCgpa = updatedCgpa;
    return stub(&req);
}

int addCourse(int rollNo, int courseCode, int marks){
    ApiRequest req;
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_ADD_COURSE;
    req.rollNo = rollNo;
    req.courseCode = courseCode;
    req.marks = marks;
    return stub(&req);
}

int deleteCourse(int rollNo, int courseCode){
   ApiRequest req;
   memset(&req, 0, sizeof(ApiRequest));
   req.apiType = API_DELETE_COURSE;
   req.rollNo = rollNo;
   req.courseCode = courseCode;
   return stub(&req);
}

int modifyCourse(int rollNo, int courseCode, int updatedMarks){
    ApiRequest req;
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_MODIFY_COURSE;
    req.rollNo = rollNo;
    req.courseCode = courseCode;
    req.updatedMarks = updatedMarks;
    return stub(&req);
}

int main(int argc, char **argv){
    if(argc != 4){
        printf("Usage: ./client <Server IP Address> <Server Port No> <Input File>\n");
        exit(0);
    }
    int port_no = atoi(argv[2]);
    struct sockaddr_in server_addr;
    char* servaddr = argv[1];
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd == -1){
        perror("socket:");
        exit(0);
    }
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    int retval=inet_pton(AF_INET,servaddr,&server_addr.sin_addr);
    if(retval==0){
        fprintf(stderr,"Given Server IP Address is not a valid format\n");
    }else if(retval == -1){
        perror("inet_pton:");
        exit(0);
    }
    server_addr.sin_port = htons(port_no);
    socklen_t serviplen = sizeof(server_addr);
    int connect_retval = connect(sockfd,(struct sockaddr*)&server_addr,serviplen);
    if(connect_retval == -1 && errno == ETIMEDOUT){
        fprintf(stderr,"No response from server\n");
        close(sockfd);
        exit(0);
    }else if(connect_retval == -1 && errno == ECONNREFUSED){
        fprintf(stderr,"No process is listening on the specified port number\n");
        close(sockfd);
        exit(0);
    }else if(connect_retval == -1 && errno == EHOSTUNREACH){
        fprintf(stderr,"Destination Unreachable/Cannot find the Server IP Address in the routing table\n");
        close(sockfd);
        exit(0);
    }

    FILE* fp = fopen(argv[3], "r");
    if(!fp){
    perror("Error opening input file");
    exit(5);
   }
   parseFile(fp);
   fclose(fp);

   close(sockfd);

}
