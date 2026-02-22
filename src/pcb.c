#include <stdio.h>
#include <stdlib.h>

#include "pcb.h"

PCB* createPCB(int startIndex, int length)
{
    PCB* newPCB = (PCB*)malloc(sizeof(PCB));
    newPCB->pid = processIDCounter++; // assign unique pid
    newPCB->startIndex = startIndex;
    newPCB->length = length;
    newPCB->pc = 0; // initialize program counter to 0
    newPCB->next = NULL; // initialize next pointer to NULL
    return newPCB;
}
