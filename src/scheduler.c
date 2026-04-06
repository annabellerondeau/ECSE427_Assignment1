#include "scheduler.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "interpreter.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PCB* head = NULL;
PCB* tail = NULL;
char *policy = "RR"; // default policy is FCFS, can be changed by exec command
int maxInstructionsRR = 2; // for RR

pthread_t t1;
pthread_t t2;
int threadsInitialized = 0;
int active_jobs = 0;
int shutting_down = 0; // flag to signal threads to exit when quit is called
int didThePageFault = 0;  // flag to indicate whether page fault resulted in victim page being evicted


//global MT controls
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

void* manageThread(void *args);

int scheduler()
{
    int errCode = 0;
    if (strcmp(policy, "RR30") == 0) maxInstructionsRR = 30;
    int maxInstructionsAging = 1; // for AGING
    if (!mtFlag){ // not multithreading
        while (head != NULL)
        {
            PCB* current = head; /// pop head of queue
            head = head->next;
            if (head == NULL)
            {
                tail = NULL;
            }

            // Assignment 3 focuses on Round Robin STRICTLY
            int pageFaultTriggered = 0; // not triggered
            int frameIndex=0;

            if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0)
            {
                int instructionsCompleted = 0;

                while (instructionsCompleted < maxInstructionsRR && current->pc < (current->totalPages * 3)) // while there are still commands and max of 2 has not been reached
                {
                    // CHECK: PAGE FAULT

                    if (!isPageInMemory(current, current->pc / 3)){
                        pageFaultTriggered = 1;
                        loadPageToMemory(current, current->pc / 3); // load page to memory
                        if (!didThePageFault) {
                            printf("Page fault!\n");
                        }
                        break; // break out of loop to add process back to ready queue
                    }

                    //printf("[DEBUG] While loop entered for PID %d at Logical PC %d\n", current->pid, current->pc);
                    int index = computePhysicalIndex(current);
                    
                    char *line = mem_get_code_line(index); // get line from page frame

                    if (line == NULL|| strcmp(line, "none") == 0) // process is finished
                    {
                        current->pc = current->totalPages * 3;
                        break;
                    }
                    else // if line is executable
                    {
                        errCode = parseInput(line);
                        setFrameTimestamp(current->pageTable[current->pc / 3]); // update timestamp
                    }
                    current->pc++;
                    instructionsCompleted++;

                }
                //didThePageFault = 0; // reset flag for next time (not useful tho as it will always be a fault from now on)

                if (pageFaultTriggered || current->pc < (current->totalPages*3)) // if there are still commands in the script that have not been ran
                {
                    addToReadyQueue(current);
                }
                else // free memory
                {
                    free(current);
                }
            }

            else if (strcmp(policy, "AGING") == 0)
            {
                int instructionsCompleted = 0;
                while (instructionsCompleted < maxInstructionsAging && current->pc < (current->totalPages * 3))
                {
                    int index = computePhysicalIndex(current);
                    char *line = mem_get_code_line(index);
                    if (line == NULL|| strcmp(line, "none") == 0) // process is finished
                    {
                        current->pc = current->totalPages * 3;
                        break;
                    }
                    else                    
                    {
                        errCode = parseInput(line);
                    }
                    current->pc++;
                    instructionsCompleted++;
                }

                PCB* temp = head;
                while (temp!= NULL)
                {
                    if (temp->score > 0)
                    {
                        temp->score--; // decrement score of all processes in ready queue
                    }
                    temp = temp->next;
                }

                if (current->pc < (current->totalPages*3)) // if there are still commands in the script that have not been ran
                {
                    if (head != NULL && head->score < current->score)
                    {
                        addToReadyQueue(current);
                    }
                    else // if current process has highest score, it goes to the head of the queue
                    {
                        current->next = head;
                        head = current;
                        if (tail == NULL)
                        {
                            tail = current;
                        }
                    }
                }
                else // free memory
                {
                    free(current);
                }
            }

            else // FCFS or SJF
            {
                while (current->pc < (current->totalPages * 3)) // while there are still commands to execute in the script
                {

                    int mem_index = computePhysicalIndex(current); // compute index in code memory
                    char* command = mem_get_code_line(mem_index); // get command from memory
                    if (command == NULL|| strcmp(command, "none") == 0) // process is finished
                    {
                        current->pc = current-> totalPages * 3;
                        break;
                    }
                    else
                    {
                        errCode = parseInput(command);
                    }
                    current->pc++;

            
                }

                free(current);
            }
        }
        // removed for A3
        //clearMemory(); // only when ready queue is empty
    }
    else{
        // Create threads
        pthread_mutex_lock(&lock);
        if (!threadsInitialized){
            threadsInitialized++;
            pthread_create(&t1, NULL, manageThread, NULL); // thread ID variable, attributes , the function to run, and its argument
            pthread_create(&t2, NULL, manageThread, NULL);
        }
        pthread_mutex_unlock(&lock);

        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&queue_not_empty); 
        pthread_mutex_unlock(&lock);

        if (backgroundFlag == 0) { 
            int floor = pthread_equal(pthread_self(), mainThreadID) ? 0 : 1; // main thread waits for all jobs to finish, worker thread only waits for jobs added while it is running
            pthread_mutex_lock(&lock);
            while (active_jobs > floor) 
            {
                pthread_cond_wait(&queue_not_empty, &lock);
            }
            pthread_mutex_unlock(&lock);
        }

        pthread_mutex_lock(&lock);
        pthread_cond_broadcast(&queue_not_empty);
        pthread_mutex_unlock(&lock);
    }
    return errCode;
}

int computePhysicalIndex(PCB* process)
{
    int logicalLine = process->pc;
    int pageNumber = logicalLine / 3; // page
    int offset = logicalLine % 3; // line on the page
    int frameNumber = process->pageTable[pageNumber];

    int physicalIndex = (frameNumber * 3) + offset; // compute physical index
    //printf("[DEBUG] PID %d is accessing Logical PC %d at Physical Index %d\n", process->pid, process->pc, physicalIndex);
    return physicalIndex;
}


void addToReadyQueue(PCB* process)
{
    pthread_mutex_lock(&lock);
    process->next = NULL;

    if (strcmp(policy, "SJF") == 0)
    {
        insertSJF(process);
    }

    else if (strcmp(policy, "AGING") == 0)
    {
        insertAGING(process); // for simplicity, we will use SJF for AGING as well, since they both prioritize shorter jobs. In a real implementation, we would need to update the scores of the processes in the ready queue to implement aging.
    }

    else if (head == NULL) 
    {
        head = process;
        tail = process;
    } 
    else
    {
        tail->next = process;
        tail = process;
    }
    pthread_cond_signal(&queue_not_empty);
    pthread_mutex_unlock(&lock);
}

void insertSJF(PCB* process)
{
    if (head == NULL || process->totalPages < head->totalPages) // if length is shorter than head length, process becomes new head
    {
        process->next = head;
        head = process;
        if (tail == NULL) 
        {
            tail = process;
        }
    } 
    else 
    {
        PCB* current = head;
        while (current->next != NULL && current->next->totalPages <= process->totalPages) // order from shortest to longest length
        {
            current = current->next;
        }
        process->next = current->next;
        current->next = process;
        if (process->next == NULL) 
        {
            tail = process;
        }
    }
}

void insertAGING(PCB* process)
{
    if (head == NULL || process->score < head->score) // if score is shorter than head score, process becomes new head
    {
        process->next = head;
        head = process;
        if (tail == NULL) 
        {
            tail = process;
        }
    } 
    else 
    {
        PCB* current = head;
        // if they are equal, the newer process is inserted after the older process
        while (current->next != NULL && current->next->score <= process->score) // order from shortest to longest score
        {
            current = current->next;
        }
        process->next = current->next;
        current->next = process;
        if (process->next == NULL) 
        {
            tail = process;
        }
    }
}

bool isReadyQueueEmpty()
{
    return head == NULL;
}

void* manageThread(void *args){
    int errCode = 0;
    while (1){ // as RR keeps adding to queue and we have 2 threads we cannot terminate thread on HEAD!= NULL
        pthread_mutex_lock(&lock); //lock queeue before checking

        while (head == NULL && active_jobs > 0 && !shutting_down) { // if queue is empty, wait for signal that it's not empty or that we are shutting down
            pthread_cond_wait(&queue_not_empty, &lock);
        } 

        if (shutting_down || (active_jobs == 0 && head == NULL) )
        { 
            pthread_mutex_unlock(&lock);
            break;
        }

        PCB* current = head; /// pop head of queue

        //iterate through queue
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        pthread_mutex_unlock(&lock);

        // check policy (ONLY RR or RR30)
        int instructionsCompleted = 0;

        while (instructionsCompleted < maxInstructionsRR && current->pc < current->totalPages) // while there are still commands and max of 2 has not been reached
        {
            int index = computePhysicalIndex(current);
            char *line = mem_get_code_line(index);
            if (line == NULL|| strcmp(line, "none") == 0) // process is finished
            {
                current->pc = current->totalPages * 3;
                break;
            }
            else
            {
                errCode = parseInput(line);
            }
            current->pc++;
            instructionsCompleted++;
        }

        if (current->pc < current->totalPages) // if there are still commands in the script that have not been ran
        {
            addToReadyQueue(current);
        }

        else 
        { 
            // PCB is finished
            free(current); 
            
            pthread_mutex_lock(&lock);
            active_jobs--; 
            pthread_cond_broadcast(&queue_not_empty); 
            
            if (shutting_down) {
                pthread_mutex_unlock(&lock);
                return NULL; 
            }
            pthread_mutex_unlock(&lock);
        }
    }
    
    return NULL; // NEEDED TO COMPILE
}
