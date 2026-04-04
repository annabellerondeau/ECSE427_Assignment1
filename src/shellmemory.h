#include "pcb.h"

#define MEM_SIZE 1000
#define FRAME_SIZE 3
#define FRAME_STORE_SIZE 300
#define VARIABLE_STORE_SIZE 10
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char* mem_get_code_line(int index);
int loadFileMemory(FILE *p, PCB *pcb);
int findFreeFrame();
void clearMemory();
