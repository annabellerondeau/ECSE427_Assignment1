extern char *policy;

#include "pcb.h"

int interpreter(char *command_args[], int args_size);
int loadFileMemory(FILE *p, PCB *pcb);
int help();