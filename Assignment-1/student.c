#include "student.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

static Student* createStudentNode(int rollNo, char name[], float cgpa){
    Student  *newNode;
    newNode = (Student*)malloc(sizeof(Student));
    if(newNode == NULL){
        return NULL;
    }
    newNode->rollNo = rollNo;
    strcpy(newNode->name, name);
    newNode->cgpa = cgpa;
    newNode->numCourses = 0;
    newNode->courseHead = NULL;
    newNode->prev = NULL;
    newNode->next = NULL;

    return newNode;
}

static Course* createCourseNode(int courseCode, int marks){
    Course *newNode;
    newNode = (Course*)malloc(sizeof(Course));
    if(newNode == NULL){
        return NULL;
    }

    newNode->courseCode = courseCode;
    newNode->marks = marks;

    return newNode;
}

static void insertStudentNode(Student** head,Student* newNode){
    if((*head) == NULL){
        (*head) = newNode;
        return;
    }
    if ((*head)!=NULL){
        (*head)->prev = newNode;
    }
    *head = newNode;
}