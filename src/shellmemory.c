#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "scheduler.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"

int loadPageToMemory(PCB *pcb, int pageNumber);
int global_timer = 0; // global timer to keep track of time instead of system clock

struct memory_struct {
    char *var;
    char *value;
};

struct memory_struct shellmemory[varmemsize]; // variable store

struct frame_to_page_table_mapping
{
    PCB *pcb; // pointer to the PCB of the process that owns this page
    int pageNumber; // page number of the process
    int timeStamp;
};

struct frame_to_page_table_mapping frame_to_page_table[framesize/3]; // frame to page table mapping


char * code_memory[framesize]; // frame store, is an array of frames of 3 lines each (multiply by 3 to get the total num of lines)
//int memoryIndex = 0; // where we are in memeory when loading lines for source NO LONGER NEEDED for A3

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

int getFrameTimestamp(int frameIndex) {
    return frame_to_page_table[frameIndex].timeStamp;
}

int setFrameTimestamp(int frameIndex) {
    return frame_to_page_table[frameIndex].timeStamp = global_timer++;
}

// Shell memory functions
// to respect MT we add lock and unlock to all previously written shell memory functions

void mem_init(){
    pthread_mutex_lock(&lock); // locking and unlocking does not disrupt single threaded function
    int i;
    for (i = 0; i < varmemsize; i++){		
        shellmemory[i].var   = "none"; // FEATURE add later strdup( allows us to avoid crash if malloc is called on none node
        shellmemory[i].value = "none";
    }
    for (i=0; i < framesize; i++)
    {
        code_memory[i] = NULL;
    }
    pthread_mutex_unlock(&lock);
}

// Set key value pair
void mem_set_value(char *var_in, char *value_in) {
    pthread_mutex_lock(&lock);
    int i;

    for (i = 0; i < varmemsize; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            shellmemory[i].value = strdup(value_in);
            pthread_mutex_unlock(&lock);
            return;
        } 
    }

    //Value does not exist, need to find a free spot.
    for (i = 0; i < varmemsize; i++){
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

    for (i = 0; i < varmemsize; i++){
        if (strcmp(shellmemory[i].var, var_in) == 0){
            pthread_mutex_unlock(&lock);
            return strdup(shellmemory[i].value);
        } 
    }
    pthread_mutex_unlock(&lock);
    return "Variable does not exist";
}

// load file into memory and return starting index and length of file in memory
int loadFileMemory(FILE *p, PCB *pcb) 
{
    pthread_mutex_lock(&lock);
    char line[MAX_USER_INPUT];

    int counter = 0;

    int pageLine = 0; // track line in the script file
    int pageNumber = 0; // track page we are filling for this specific process (the page in the page table attribute of the pcb struct)
    int frameIndex = -1; // physical frame number in code_memory

    while (fgets(line, MAX_USER_INPUT - 1, p) != NULL) 
    {
        if (pageLine % 3 == 0) // if the frame is full 
        {
            frameIndex = findFreeFrame();
            if (frameIndex == -1) // no free frame
            {            
                pthread_mutex_unlock(&lock);
                return -1; // full memory error
            }
            pcb->pageTable[pageNumber] = frameIndex; // sets the frame number of the pcb table for this page
            pageNumber++;
        }

        line[strcspn(line, "\r\n")] = 0;
        int physicalLine = (frameIndex * 3) + (pageLine % 3); //computethe actual physical line
        code_memory[physicalLine] = strdup(line);

        pageLine++;
    }

    //printf("[DEBUG] Finished loading file into memory for PID %d with total lines %d and total pages %d\n", pcb->pid, pageLine, pageNumber);
    pcb->totalPages = pageNumber;
    pcb->score = pageLine; // for aging, initialize score to length of script
    pthread_mutex_unlock(&lock);
    return 0; // success
}

int findFreeFrame()
{
    for (int i = 0; i < framesize/3; i++)
    {
        if (code_memory[i*3] == NULL) // if the frame if free
        {
            // printf("[DEBUG] Found free frame at index %d\n", i);
            return i;
        }
    }
    didThePageFault = 1; // set flag to indicate a page fault occurred
    
    // no free frames will need to evict

    printf("Page fault! Victim page contents:\n\n");

    // find frame with oldest time stamp
    //int frameToEvict = rand() % (framesize/3); // randomly select a frame to evict
    
    int frameToEvict = 0; // randomly select a frame to evict
    int oldestTimeStamp = frame_to_page_table[0].timeStamp;

    for (int i = 1; i < framesize/3; i++)
    {
        if (frame_to_page_table[i].timeStamp < oldestTimeStamp)
        {
            oldestTimeStamp = frame_to_page_table[i].timeStamp;
            frameToEvict = i;
        }
    }

    frame_to_page_table[frameToEvict].pcb->pageTable[frame_to_page_table[frameToEvict].pageNumber] = -1; // update page table of the process that owned the evicted page to indicate page is no longer in memory
    // printf("[DEBUG] Evicting frame %d which belongs to PID %d, file name %s and page number %d\n", frameToEvict, frame_to_page_table[frameToEvict].pcb->pid, frame_to_page_table[frameToEvict].pcb->filename, frame_to_page_table[frameToEvict].pageNumber);
    for (int j = 0; j < 3; j++)
    {
        char *line = code_memory[j + (frameToEvict * 3)];
        if (line != NULL)
        {
            printf("%s\n", line);
            free(line); // free the line in memory
            code_memory[j + (frameToEvict * 3)] = NULL; // set to null to indicate frame is now free
        }
    }
    printf("\nEnd of victim page contents.\n");

    return frameToEvict; // no free frames left
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
    //memoryIndex = 0;
    pthread_mutex_unlock(&lock);
}

int initialDemandLoading(PCB *pcb){ 
    
    pthread_mutex_lock(&lock);
    char line[MAX_USER_INPUT];
    int pageLine = 0; // track line in the script file

    while (fgets(line, sizeof(line), pcb->fp) != NULL) {
        pageLine++;
    }
    pcb->totalPages = (pageLine / 3) + (pageLine % 3 != 0);
    pcb->score = pageLine; // for aging, initialize score to length of script
    pcb->pc = 0; // initialize program counter to 0

    // Reset file pointer to the beginning of the file for future page faults
    rewind(pcb->fp);

    for (int i = 0; i < 100; i++) {
        pcb->pageTable[i] = -1;
    }
    
    // Load the first page of the process into memory
    loadPageToMemory(pcb, 0); 
    if (pcb->totalPages > 1) {
        loadPageToMemory(pcb, 1);
    }
    
    pthread_mutex_unlock(&lock);

    return 0; // success
}

// code_memory is our frame store, shellmemory is our variable store. We can have a maximum of 1000 lines in the code memory, and 100 variables in the variable store. Each process can take up to 100 lines of code (or 33 pages) and we have a maximum of 100 frames (or 300 lines) in the code memory.
int loadPageToMemory(PCB *pcb, int pageNumber){
    
    rewind(pcb->fp); // reset file pointer to the beginning of the file
    char temp[MAX_USER_INPUT];
    int frameIndex = findFreeFrame(); // WILL NEED TO HANDLE ERRORS
    frame_to_page_table[frameIndex].pcb = pcb; // update frame to page table mapping
    frame_to_page_table[frameIndex].pageNumber = pageNumber;
    setFrameTimestamp(frameIndex); // set timestamp for the loaded frame
    pcb->pageTable[pageNumber] = frameIndex;

    // Move file pointer to the start of the page we want to load
    int pageLine = pageNumber*3;
    for (int i = 0; i < pageLine; i++) {
        if (fgets(temp, sizeof(temp), pcb->fp) == NULL) return -1; // SAFE GUARD: if we reach end of file before reaching the start of the page, return error
    }

    for (int j = 0; j < 3; j++) {
        if (fgets(temp, sizeof(temp), pcb->fp) != NULL) { // if there exists a line
            
            temp[strcspn(temp, "\r\n")] = 0; // remove trailing newline characters

            int physicalLine = (frameIndex * 3) + j; //Compute physical line
            code_memory[physicalLine] = strdup(temp);
        } else {
            break; // done reading lines
        }
    }

    // Load the page into the frame

    return 0;// success 
}

int isPageInMemory(PCB *pcb, int pageNumber)
{
    return pcb->pageTable[pageNumber] != -1;
}