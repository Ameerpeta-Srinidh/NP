#ifndef STUDENT_H
#define STUDENT_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef enum{
    NONE,
    INSERT_STUDENT,
    INSERT_COURSE,
    UPDATE_STUDENT,
    UPDATE_COURSE,
    REMOVE_STUDENT,
    REMOVE_COURSE
}ParseMode;

typedef struct Course{
    int courseCode;
    int marks;
    struct Course *next;
    struct Course *prev;
}Course;

typedef struct Student{
    int rollNo;
    char name[50];
    float cgpa;
    int numCourses;
    Course *courseHead;
    struct Student *prev;
    struct Student *next;
}Student;

Student *searchStudent(int rollNo);
Course *searchCourse(Course *courseHead, int courseCode);

int addStudent(int rollNo, char name[], float cgpa);
int deleteStudent(int rollNo);
int modifyStudent(int rollNo, float updatedCgpa);
int addCourse(int rollNo, int courseCode, int marks);
int deleteCourse(int rollNo, int courseCode);
int modifyCourse(int rollNo, int courseCode, int updatedMarks);

Student *getHead(void);
void reverseList(void);
void freeList(void);



#endif
