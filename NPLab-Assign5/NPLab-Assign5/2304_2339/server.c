#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "student.h"
#include "api.h"

#define FIFO1 "/tmp/fifo.1"
#define FIFO2 "/tmp/fifo.2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int reqFd = -1;
int resFd = -1;

static int writeOutput(FILE *fp){
    if (fp ==NULL)
        return -1;
 
    Student *student = getHead();
    while(student != NULL){
        fprintf(fp, "%d, %s, %g, %d\n",student->rollNo, student->name, student->cgpa, student->numCourses);
        Course *course = student->courseHead;
        while(course != NULL){
            fprintf(fp, "%d, %d\n", course->courseCode, course->marks);
            course = course->next;
        }
        student = student->next;
    }
    return 0;
}

int main(){
    if((mkfifo (FIFO1, FILE_MODE) < 0) && (errno != EEXIST)) {
       fprintf(stderr, "can't create %s\n",FIFO1);
       exit(1);
    }

    if((mkfifo (FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) {
        fprintf(stderr, "Can't create %s \n",FIFO2);
        exit(2);
    }

    reqFd = open(FIFO1, O_RDONLY, 0);
    if (reqFd < 0) {
        perror("Server failed to open FIFO1 for reading");
        exit(3);
    }

    resFd = open(FIFO2, O_WRONLY, 0);
    if (resFd < 0) {
        perror("Server failed to open FIFO2 for writing");
        exit(4);
    }

    ApiRequest req;
    ApiResponse res;
    int bytes;

    while((bytes = read(reqFd, &req, sizeof(ApiRequest))) > 0){
        int result;
        switch(req.apiType){
        case API_ADD_STUDENT:
            result = addStudent(req.rollNo, req.name, req.cgpa);
            printf("Add Student: %d\n", result);
            break;
        case API_DELETE_STUDENT:
            result = deleteStudent(req.rollNo);
            printf("Delete Student: %d\n", result);
            break;
        case API_MODIFY_STUDENT:
            result = modifyStudent(req.rollNo, req.updatedCgpa);
            printf("Modify Student: %d\n", result);
            break;
        case API_ADD_COURSE:
            result = addCourse(req.rollNo, req.courseCode, req.marks);
            printf("Add Course: %d\n", result);
            break;
        case API_DELETE_COURSE:
            result = deleteCourse(req.rollNo, req.courseCode);
            printf("Delete Course: %d\n", result);
            break;
        case API_MODIFY_COURSE:
            result = modifyCourse(req.rollNo, req.courseCode, req.updatedMarks);
            printf("Modify Course: %d\n", result);
            break;
        default:
            result = -1;
            break;
        }
        res.status = result;
        write(resFd, &res, sizeof(ApiResponse));
    }

    close(reqFd);
    close(resFd);

    reverseList();
    FILE *outputFile = fopen("2304_2339.out", "w");
    if (outputFile != NULL){
        writeOutput(outputFile);
        fclose(outputFile);
    } else {
        perror("Failed to open output file");
    }

    freeList();
    return 0;
}
