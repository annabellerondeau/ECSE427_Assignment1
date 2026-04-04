#ifndef PCB_H
#define PCB_H

static int processIDCounter = 1; // global counter to assign unique PIDs

typedef struct PCB_struct 
{
    int pid; // process id
    //int startIndex;
    char* filename;
    int pageTable[100]; // value of element i is the frame number in page i
    int totalPages;
    //int length;
    int pc;  // program counter
    int score; // aging -> starts with length 
    struct PCB_struct *next; // pointer to next PCB in the queue
} PCB;

PCB* createPCB(); // to implement

#endif