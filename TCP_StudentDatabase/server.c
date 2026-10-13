#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "api.h"
#include "student.h"

static int writeOutput(FILE *fp)
{
    if (fp == NULL)
        return -1;

    Student *student = getHead();
    while (student != NULL) {
        fprintf(fp, "%d, %s, %g, %d\n", student->rollNo, student->name,
                student->cgpa, student->numCourses);

        Course *course = student->courseHead;
        while (course != NULL) {
            fprintf(fp, "%d, %d\n", course->courseCode, course->marks);
            course = course->next;
        }

        student = student->next;
    }

    return 0;
}

static int readFull(int fd, void *buffer, size_t size)
{
    char *cursor = buffer;
    size_t remaining = size;

    while (remaining > 0) {
        ssize_t bytes = read(fd, cursor, remaining);
        if (bytes == 0)
            return 0;
        if (bytes < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        cursor += bytes;
        remaining -= bytes;
    }

    return 1;
}

static int writeFull(int fd, const void *buffer, size_t size)
{
    const char *cursor = buffer;
    size_t remaining = size;

    while (remaining > 0) {
        ssize_t bytes = write(fd, cursor, remaining);
        if (bytes < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }

        cursor += bytes;
        remaining -= bytes;
    }

    return 0;
}

static int handleClient(int connfd, const char *outputFilename)
{
    ApiRequest req;
    ApiResponse res;

    for (;;) {
        int readStatus = readFull(connfd, &req, sizeof(req));
        if (readStatus == 0)
            break;
        if (readStatus < 0) {
            perror("read");
            break;
        }

        switch (req.apiType) {
        case API_ADD_STUDENT:
            res.status = addStudent(req.rollNo, req.name, req.cgpa);
            printf("Add Student: %d\n", res.status);
            break;
        case API_DELETE_STUDENT:
            res.status = deleteStudent(req.rollNo);
            printf("Delete Student: %d\n", res.status);
            break;
        case API_MODIFY_STUDENT:
            res.status = modifyStudent(req.rollNo, req.updatedCgpa);
            printf("Modify Student: %d\n", res.status);
            break;
        case API_ADD_COURSE:
            res.status = addCourse(req.rollNo, req.courseCode, req.marks);
            printf("Add Course: %d\n", res.status);
            break;
        case API_DELETE_COURSE:
            res.status = deleteCourse(req.rollNo, req.courseCode);
            printf("Delete Course: %d\n", res.status);
            break;
        case API_MODIFY_COURSE:
            res.status = modifyCourse(req.rollNo, req.courseCode,
                                      req.updatedMarks);
            printf("Modify Course: %d\n", res.status);
            break;
        default:
            res.status = -1;
            break;
        }

        if (writeFull(connfd, &res, sizeof(res)) < 0) {
            perror("write");
            break;
        }
    }

    reverseList();

    FILE *outputFile = fopen(outputFilename, "w");
    if (outputFile != NULL) {
        writeOutput(outputFile);
        fclose(outputFile);
    } else {
        perror("Failed to open output file");
    }

    freeList();
    close(connfd);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        printf("Usage: ./server <Server IP Address> <Server Port Number> <Output File>\n");
        exit(EXIT_FAILURE);
    }

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
        fprintf(stderr, "Given Server IP Address is not a valid format\n");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (bind(listenfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int connfd = accept(listenfd, (struct sockaddr *)&client_addr, &len);
        if (connfd < 0) {
            if (errno == EINTR)
                continue;
            perror("accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(listenfd);
            handleClient(connfd, argv[3]);
            exit(EXIT_SUCCESS);
        }

        if (pid < 0) {
            perror("fork");
            close(connfd);
            continue;
        }

        close(connfd);
    }
}
