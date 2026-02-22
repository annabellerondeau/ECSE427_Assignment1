extern char *policy;

int interpreter(char *command_args[], int args_size);
int loadFileMemory(FILE *p, int *fileIndex, int *length);
int help();