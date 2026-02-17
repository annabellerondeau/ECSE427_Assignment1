typedef struct PCB_struct 
{
    int pid; // process id
    int startIndex;
    int length;
    int pc;  // program counter
    struct PCB *next; // pointer to next PCB in the queue
}; PCB_struct;

PCB* createPCB(int startIndex, int length);