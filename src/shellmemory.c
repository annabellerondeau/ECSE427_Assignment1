#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <scheduler.h>


#include "shellmemory.h"
#include "shell.h"

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[MEM_SIZE];

char * code_memory[MEM_SIZE]; // holds lines of code from file for source command
int memoryIndex = 0; // where we are in memeory when loading lines for source

// Helper functions
int match(char *model, char *var) {
    int i, len = strlen(var), matchCount = 0;
    for (i = 0; i < len; i++) {
        if (model[i] == var[i]) matchCount++;
    }
    if (matchCount == len) {
        return 1;
    } else return 0;
}

// Shell memory functions
// to respect MT we add lock and unlock to all previously written shell memory functions

void mem_init(){
    pthread_mutex_lock(&lock); // locking and unlocking does not disrupt single threaded function
    int i;
    for (i = 0; i < MEM_SIZE; i++){		
        shellmemory[i].var   = "none"; // FEATURE add later strdup( allows us to avoid crash if malloc is called on none node
        shellmemory[i].value = "none";
    }
    pthread_mutex_unlock(&lock);
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    pthread_mutex_lock(&lock);
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            pthread_mutex_unlock(&lock);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, "none") == 0){
            shellmemory[i].var   = strdup(var_in);
            shellmemory[i].value = strdup(value_in);
            pthread_mutex_unlock(&lock);
            return;
        } 
    }
    pthread_mutex_unlock(&lock);
    return;
}

//get value based on input key
char *mem_get_value(char *var_in) {
    pthread_mutex_lock(&lock); // ensures only 1 thread getting AND setting value simultaneously
    int i;

    for (i = 0; i < MEM_SIZE; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            pthread_mutex_unlock(&lock);
            return strdup(shellmemory[i].value);
        } 
    }
    pthread_mutex_unlock(&lock);
    return "Variable does not exist";
}

// load file into memory and return starting index and length of file in memory
int loadFileMemory(FILE *p, int *fileIndex, int *length) 
{
    pthread_mutex_lock(&lock);
    char line[MAX_USER_INPUT];
    *fileIndex = memoryIndex; // start loading file at current memory index
    int counter = 0;

    while (fgets(line, MAX_USER_INPUT - 1, p) != NULL) 
    {
        if (memoryIndex >= 1000) 
        {
            pthread_mutex_unlock(&lock);
            return -1; // full memory error
        }

        line[strcspn(line, "\r\n")] = 0;
        code_memory[memoryIndex] = strdup(line);

        memoryIndex++; // increment next free slot in memory
        counter++; // increment counter for length of file in memory
    }

    *length = counter; // length of file is number of lines read
    pthread_mutex_unlock(&lock);
    return 0; // success
}

char* mem_get_code_line(int index) // getter for code memory
{
    pthread_mutex_lock(&lock);
    char * temp = code_memory[index];
    pthread_mutex_unlock(&lock);
    return temp;
}

// if "none" no need to clear
void clearMemory() // clear code memory
{
    pthread_mutex_lock(&lock);
    int i;
    for (i = 0; i < MEM_SIZE; i++)
    {		
        if (shellmemory[i].var != NULL && strcmp(shellmemory[i].var, "none") != 0)
        {
            free(shellmemory[i].var);
            shellmemory[i].var= "none";
            free(shellmemory[i].value);
            shellmemory[i].value= "none";
        }
        if(code_memory[i] != NULL){
            free(code_memory[i]);
            code_memory[i] = NULL;
        }
    }
    memoryIndex = 0;
    pthread_mutex_unlock(&lock);
}