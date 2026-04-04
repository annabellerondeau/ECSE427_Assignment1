#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"

PCB* createPCB()
{
    PCB* newPCB = (PCB*)malloc(sizeof(PCB));
    newPCB->pid = processIDCounter++; // assign unique pid

    for (int i = 0; i < 100; i++)
    {
        newPCB->pageTable[i] = -1;
    }
    //newPCB->startIndex = startIndex;
    //newPCB->length = length;
    //newPCB->score = 0; // for aging, initialize score to length
    newPCB->totalPages = 0; // later set in loadFileMemory
    newPCB->pc = 0; // initialize program counter to 0
    newPCB->next = NULL; // initialize next pointer to NULL
    return newPCB;
}
