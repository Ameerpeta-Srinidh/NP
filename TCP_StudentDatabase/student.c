#include "student.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
static Student* head = NULL;
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
    newNode->next = *head;
    if ((*head)!=NULL){
        (*head)->prev = newNode;
    }
    *head = newNode;
}

static void deleteStudentNode(Student** head, Student* student){
    Course *course = student->courseHead;
    while(course != NULL){
        Course *nextCourse = course->next;
        free(course);
        course = nextCourse;
    }
    if(student->prev != NULL){
        student->prev->next = student->next;
    }else{
        *head = student->next;
    }
    if(student->next != NULL){
        student->next->prev = student->prev;
    }
    free(student);
}

static void insertCourseNode(Course** courseHead, Course *newNode){
    newNode->next = *courseHead;
    newNode->prev = NULL;
    if(*courseHead != NULL){
        (*courseHead)->prev = newNode;
    }
    *courseHead = newNode;
}

static void deleteCourseNode(Course **courseHead, Course *course){
    if(course->prev != NULL){
        course->prev->next = course->next;
    }else{
        *courseHead = course->next;
    }
    if(course->next != NULL){
        course->next->prev = course->prev;
    }
    free(course);
}

Student* searchStudent(int rollNo){
    Student* temp = head;
    while(temp != NULL){
        if(temp->rollNo == rollNo){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

Course* searchCourse(Course *courseHead, int courseCode){
    Course* temp = courseHead;
    while(temp != NULL){
        if(temp->courseCode == courseCode){
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

static void reverseStudentList(Student** head){
    Student *current = *head;
    Student *prevNode = NULL;
    while(current!=NULL){
        Student* nextNode = current->next;
        current->next = prevNode;
        current->prev = nextNode;
        prevNode = current;
        current = nextNode;
    }
    *head = prevNode;
}

static void reverseCourseList(Course **courseHead){
    Course *current = *courseHead;
    Course *prevNode = NULL;
    while(current!=NULL){
        Course* nextNode = current->next;
        current->next = prevNode;
        current->prev = nextNode;
        prevNode = current;
        current = nextNode;
    }
    *courseHead = prevNode;
}

void reverseList(void){
    reverseStudentList(&head);
    Student *student = head;
    while(student!=NULL){
        reverseCourseList(&student->courseHead);
        student = student->next;
    }
}

Student *getHead(void){
    return head;
}

void freeList(void){
    Student *student = head;
    while(student != NULL){
        Student *nextStudent = student->next;
        Course *course = student->courseHead;
        while(course != NULL){
            Course *nextCourse = course->next;
            free(course);
            course = nextCourse;
        }
        free(student);
        student = nextStudent;
    }
    head = NULL;
}

int addStudent(int rollNo, char name[], float cgpa){
    Student* student = searchStudent(rollNo);
    if(student != NULL){
        return 1;
    }
    Student *newNode = createStudentNode(rollNo,name,cgpa);
    if(newNode == NULL){
        return 2;
    }
    insertStudentNode(&head, newNode);
    return 0;
}

int deleteStudent(int rollNo){
    Student *student = searchStudent(rollNo);
    if(student==NULL){
        return 1;
    }
    deleteStudentNode(&head,student);
    return 0;
}

int modifyStudent(int rollNo, float updatedCgpa){
    Student *student = searchStudent(rollNo);
    if(student==NULL){
        return 1;
    }
    student->cgpa = updatedCgpa;
    return 0;
}

int addCourse(int rollNo, int courseCode, int marks){
    Student *student = searchStudent(rollNo);
    if(student == NULL){
        return 2;
    }
    Course *course = searchCourse(student->courseHead,courseCode);
    if(course != NULL){
        return 1;
    }
    Course *newCourse = createCourseNode(courseCode,marks);
    if(newCourse == NULL){
        return 3;
    }
    insertCourseNode(&student->courseHead, newCourse);
    student->numCourses = student->numCourses + 1;
    return 0;
}

int deleteCourse(int rollNo, int courseCode){
    Student *student = searchStudent(rollNo);
    if(student == NULL){
        return 2;
    }
    Course *course = searchCourse(student->courseHead, courseCode);
    if(course == NULL){
        return 1;
    }
    deleteCourseNode(&student->courseHead,course);
    student->numCourses = student->numCourses - 1;
    return 0;
}

int modifyCourse(int rollNo, int courseCode, int updatedMarks){
    Student *student = searchStudent(rollNo);
    if(student == NULL){
        return 2;
    }
    Course *course = searchCourse(student->courseHead, courseCode);
    if(course == NULL){
        return 1;
    }
    course->marks = updatedMarks;
    return 0;
}
