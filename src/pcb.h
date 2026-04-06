#ifndef PCB_H
#define PCB_H

#include <stdio.h>

static int processIDCounter = 1; // global counter to assign unique PIDs

typedef struct PCB_struct 
{
    int pid; // process id
    //int startIndex;
    char* filename;
    int pageTable[100]; // value of element i is the frame number in page i (should be -1)
    int totalPages;
    //int length;
    int pc;  // program counter
    int score; // aging -> starts with length 
    struct PCB_struct *next; // pointer to next PCB in the queue
    FILE *fp; // file pointer for page fault
} PCB;

PCB* createPCB(); // to implement

#endif