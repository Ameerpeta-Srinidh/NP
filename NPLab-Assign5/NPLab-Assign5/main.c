#include <stdio.h>
#include <stdlib.h>
#include "student.h"
#include "parser.h"
 
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
 
int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }
    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL){
        perror("Failed to open input file");
        return 1;
    }
    int result = parseFile(inputFile);
    fclose(inputFile);
    if(result != 0){
        freeList();
        return 1;
    }
 
    reverseList();
    FILE *outputFile = fopen("2304_2339.out", "w");
    if (outputFile == NULL){
        perror("Failed to open output file");
        freeList();
        return 1;
    }
    writeOutput(outputFile);
    fclose(outputFile);
    freeList();
    return 0;
}
 
