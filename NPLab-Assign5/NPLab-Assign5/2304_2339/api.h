#ifndef API_H
#define API_H

typedef enum {
    API_ADD_STUDENT,
    API_DELETE_STUDENT,
    API_MODIFY_STUDENT,
    API_ADD_COURSE,
    API_DELETE_COURSE,
    API_MODIFY_COURSE
} ApiType;

typedef struct {
    ApiType apiType;
    int rollNo;
    char name[50];
    float cgpa;
    int courseCode;
    int marks;
    int updatedMarks;
    float updatedCgpa;
} ApiRequest;

typedef struct {
    int status;
} ApiResponse;

#endif
