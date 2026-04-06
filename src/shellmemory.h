#include "pcb.h"

#define MEM_SIZE 1000

void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char* mem_get_code_line(int index);
int loadFileMemory(FILE *p, PCB *pcb);
int findFreeFrame();
void clearMemory();
int didThePageFault;
int getFrameTimestamp(int frameIndex);
int setFrameTimestamp(int frameIndex);