#define MEM_SIZE 1000
void mem_init();
char *mem_get_value(char *var);
void mem_set_value(char *var, char *value);
char* mem_get_code_line(int index);
int loadFileMemory(FILE *p, int *fileIndex, int *length);
void clearMemory();
