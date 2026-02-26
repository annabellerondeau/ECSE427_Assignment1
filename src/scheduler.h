#include "pcb.h"
#include <stdio.h>

#ifndef SCHEDULER_H
#define SCHEDULER_H

int scheduler();
void addToReadyQueue(PCB* process);
void insertSJF(PCB* process);
void insertAGING(PCB* process);

extern char *policy; // global variable

#endif
