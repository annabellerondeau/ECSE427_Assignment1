#ifndef PCB_H
#define PCB_H

typedef struct PCB_struct 
{
    int pid; // process id
    int startIndex;
    int length;
    int pc;  // program counter
    struct PCB *next; // pointer to next PCB in the queue
} PCB;

PCB* createPCB(int startIndex, int length); // to implement

#endif