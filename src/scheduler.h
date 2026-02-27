#include "pcb.h"
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#ifndef SCHEDULER_H
#define SCHEDULER_H

int scheduler();
void addToReadyQueue(PCB* process);
bool isReadyQueueEmpty();
void insertSJF(PCB* process);
void insertAGING(PCB* process);

// global variables
extern char *policy;
extern int mtFlag;
extern int backgroundFlag;
extern pthread_t t1;
extern pthread_t t2;
extern pthread_mutex_t lock;
extern int active_jobs ;
extern pthread_cond_t queue_not_empty;

#endif
