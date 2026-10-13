#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include "parser.h"
#include "student.h"
#include "api.h"

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int reqFd = -1;
int resFd = -1;

void sigpipe_handler(int signo) {
    if (signo == SIGPIPE) {
        printf("Server is not running or has closed the pipe\n");
        exit(1);
    }
}

int stub(ApiRequest *req){
    write(reqFd, req, sizeof(ApiRequest));
    ApiResponse reply;
    read(resFd, &reply, sizeof(ApiResponse));
    return reply.status;
}

int addStudent(int rollNo, char name[], float cgpa){
    ApiRequest req;
    // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
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
    // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_DELETE_STUDENT;
    req.rollNo = rollNo;
    return stub(&req);
}

int modifyStudent(int rollNo, float updatedCgpa){
    ApiRequest req;
    // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_MODIFY_STUDENT;
    req.rollNo = rollNo;
    req.updatedCgpa = updatedCgpa;
    return stub(&req);
}

int addCourse(int rollNo, int courseCode, int marks){
    ApiRequest req;
    // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_ADD_COURSE;
    req.rollNo = rollNo;
    req.courseCode = courseCode;
    req.marks = marks;
    return stub(&req);
}

int deleteCourse(int rollNo, int courseCode){
   ApiRequest req;
   // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
   memset(&req, 0, sizeof(ApiRequest));
   req.apiType = API_DELETE_COURSE;
   req.rollNo = rollNo;
   req.courseCode = courseCode;
   return stub(&req);
}

int modifyCourse(int rollNo, int courseCode, int updatedMarks){
    ApiRequest req;
    // Zero out the struct to prevent sending uninitialized garbage data over the FIFO
    memset(&req, 0, sizeof(ApiRequest));
    req.apiType = API_MODIFY_COURSE;
    req.rollNo = rollNo;
    req.courseCode = courseCode;
    req.updatedMarks = updatedMarks;
    return stub(&req);
}

int main(int argc, char *argv[]){
    signal(SIGPIPE, sigpipe_handler);
    
    if(argc != 2){
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }
    if ((mkfifo (FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) {
       fprintf(stderr, "can't create %s\n",FIFO1);
       exit(1);
    }

    if ((mkfifo (FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
    fprintf(stderr, "Can't create %s \n",FIFO2);
    exit(2);
  }
   reqFd = open(FIFO1, O_WRONLY, 0);
   if (reqFd < 0) {
       perror("Client failed to open FIFO1 for writing");
       exit(3);
   }

   resFd = open(FIFO2, O_RDONLY, 0);
   if (resFd < 0) {
       perror("Client failed to open FIFO2 for reading");
       exit(4);
   }

   FILE* fp = fopen(argv[1], "r");
   if(!fp){
    perror("Error opening input file");
    exit(5);
   }
   parseFile(fp);
   fclose(fp);

   close(reqFd);
   close(resFd);

}
