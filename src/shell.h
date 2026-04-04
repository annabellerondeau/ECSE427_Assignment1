#define MAX_USER_INPUT 1000

#include "pcb.h"

int parseInput(char inp[]);
int loadFileMemory(FILE *p, PCB *pcb);

extern pthread_t mainThreadID;
extern int mainThreadInitialized ;