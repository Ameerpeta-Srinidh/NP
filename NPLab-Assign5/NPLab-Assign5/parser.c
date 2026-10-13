#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "student.h"
#include "parser.h"

#define MAX_LINE 256

static ParseMode getParseMode(const char* line){
    if(strstr(line, "add student")) return INSERT_STUDENT;
    if(strstr(line, "add course")) return INSERT_COURSE;
    if(strstr(line, "modify student"))return UPDATE_STUDENT;
    if(strstr(line, "modify course")) return UPDATE_COURSE;
    if(strstr(line, "delete student"))return REMOVE_STUDENT;
    if(strstr(line, "delete course"))return REMOVE_COURSE;
    return NONE;
}

static void printOperationError(ParseMode pm, int result, int rollNo, int courseCode){
    if(pm == INSERT_STUDENT){
        if(result == 1)
            fprintf(stderr, "Error: student with roll number %d already exists.\n", rollNo);
        else
            fprintf(stderr, "Error: unable to add student with roll number %d.\n", rollNo);
    }else if(pm == INSERT_COURSE){
        if(result == 1)
            fprintf(stderr, "Error: course %d already exists for student %d.\n", courseCode, rollNo);
        else if(result == 2)
            fprintf(stderr, "Error: student with roll number %d does not exist.\n", rollNo);
        else
            fprintf(stderr, "Error: unable to add course %d for student %d.\n", courseCode, rollNo);
    }else if(pm == UPDATE_STUDENT || pm == REMOVE_STUDENT){
        fprintf(stderr, "Error: student with roll number %d does not exist.\n", rollNo);
    }else if(pm == UPDATE_COURSE || pm == REMOVE_COURSE){
        if(result == 2)
            fprintf(stderr, "Error: student with roll number %d does not exist.\n", rollNo);
        else
            fprintf(stderr, "Error: course %d does not exist for student %d.\n", courseCode, rollNo);
    }
}

int parseFile(FILE *fp){
    char line[MAX_LINE];
    ParseMode pm = NONE;
    while(fgets(line, sizeof(line), fp)){
        if(line[0] == '#'){
            ParseMode newPm = getParseMode(line);
            if(newPm != NONE) pm = newPm;
            else if(line[1] != '\n' && line[1] != '\0'){
                fprintf(stderr, "Invalid mode: %s", line);
                pm = NONE;
            }
            continue;
        }
        if(isspace((unsigned char)line[0]))
            continue;
        int rollNo, courseCode, marks, numCourses;
        char name[50];
        float cgpa;
        int result;
        switch(pm){
            case INSERT_STUDENT:
                if(sscanf(line, "%d, %49[^,], %f, %d", &rollNo, name, &cgpa, &numCourses) == 4){
                    result = addStudent(rollNo, name, cgpa);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, 0);
                }
                else
                    fprintf(stderr, "Invalid add student line: %s\n", line);
                break;
            case INSERT_COURSE:
                if(sscanf(line, "%d, %d, %d", &rollNo, &courseCode, &marks) == 3){
                    result = addCourse(rollNo, courseCode, marks);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, courseCode);
                }
                else
                    fprintf(stderr, "Invalid add course line: %s\n", line);
                break;
            case UPDATE_STUDENT:
                if(sscanf(line, "%d, %f", &rollNo, &cgpa) == 2){
                    result = modifyStudent(rollNo, cgpa);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, 0);
                }
                else
                    fprintf(stderr, "Invalid modify student line: %s\n", line);
                break;
            case UPDATE_COURSE:
                if(sscanf(line, "%d, %d, %d", &rollNo, &courseCode, &marks) == 3){
                    result = modifyCourse(rollNo, courseCode, marks);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, courseCode);
                }
                else
                    fprintf(stderr, "Invalid modify course line: %s\n", line);
                break;
            case REMOVE_STUDENT:
                if(sscanf(line, "%d", &rollNo) == 1){
                    result = deleteStudent(rollNo);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, 0);
                }
                else
                    fprintf(stderr, "Invalid delete student line: %s\n", line);
                break;
            case REMOVE_COURSE:
                if(sscanf(line, "%d, %d", &rollNo, &courseCode) == 2){
                    result = deleteCourse(rollNo, courseCode);
                    if(result != 0)
                        printOperationError(pm, result, rollNo, courseCode);
                }
                else
                    fprintf(stderr, "Invalid delete course line: %s\n", line);
                break;

            case NONE:    
            default:
                fprintf(stderr, "Error: unknown operation mode.\n");
                break;
        }
    }
    return 0;
}
