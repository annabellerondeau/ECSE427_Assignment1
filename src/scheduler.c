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
char *policy = "FCFS"; // default policy is FCFS, can be changed by exec command
int maxInstructionsRR = 2; // for RR

pthread_t t1;
pthread_t t2;
int threadsInitialized = 0;
int active_jobs = 0;

//global MT controls
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;

void* manageThread(void *args);

int scheduler()
{
    int errCode = 0;
    if (strcmp(policy, "RR30") == 0) maxInstructionsRR = 30;
    int maxInstructionsAging = 1; // for AGING
    if (!mtFlag){
        while (head != NULL)
        {
            PCB* current = head; /// pop head of queue
            head = head->next;
            if (head == NULL)
            {
                tail = NULL;
            }
            //printf("Running process with PID: %d\n", current->pid); // DEBUG

            if (strcmp(policy, "RR") == 0 || strcmp(policy, "RR30") == 0)
            {
                int instructionsCompleted = 0;

                while (instructionsCompleted < maxInstructionsRR && current->pc < current->length) // while there are still commands and max of 2 has not been reached
                {
                    char *line = mem_get_code_line(current->startIndex + current->pc);
                    if (line != NULL)
                    {
                        errCode = parseInput(line);
                    }
                    current->pc++;
                    instructionsCompleted++;
                }

                if (current->pc < current->length) // if there are still commands in the script that have not been ran
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
                while (instructionsCompleted < maxInstructionsAging && current->pc < current->length)
                {
                    char *line = mem_get_code_line(current->startIndex + current->pc);
                    if (line != NULL)
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
                        temp->score--;// decrement score of all processes in ready queue
                    }
                    temp = temp->next;
                }

                if (current->pc < current->length) // if there are still commands in the script that have not been ran
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
                while (current->pc < current->length) // while there are still commands to execute in the script
                {
                    int mem_index = current->startIndex + current->pc; // compute index in code memory
                    char* command = mem_get_code_line(mem_index); // get command from mem
                    if (command != NULL)
                    {
                        errCode = parseInput(command);
                    }
                    else
                    {
                        printf("Error: Command not found at memory index %d\n", mem_index);
                        break;
                    }
                    current->pc++;
                }

                free(current);
            }
        }
    clearMemory(); // only when ready queue is empty
    }
    else{
        // Create threads
        pthread_mutex_lock(&lock);
        if (!threadsInitialized){
            threadsInitialized++;
            pthread_create(&t1, NULL, manageThread, NULL); // thread ID variable, attributes , the function to run, and its argument
            pthread_create(&t2, NULL, manageThread, NULL);
        }


       // if (backgroundFlag == 0) { // test from gemini
//            pthread_mutex_lock(&lock);
//            while (active_jobs > 0) {
//                // Wait for worker threads to signal that active_jobs reached 0
//                pthread_cond_wait(&queue_not_empty, &lock);
//            }
//            pthread_mutex_unlock(&lock);
//
//            // In foreground mode, we can clear memory once jobs are done
//            clearMemory();
      //  }
          pthread_cond_broadcast(&queue_not_empty);
          pthread_mutex_unlock(&lock);


        // Process is RR or RR30
    }
    return errCode;
}

void addToReadyQueue(PCB* process)
{
    pthread_mutex_lock(&lock);
    //active_jobs++;
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
    if (head == NULL || process->length < head->length) // if length is shorter than head length, process becomes new head
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
        while (current->next != NULL && current->next->length <= process->length) // order from shortest to longest length
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
        // FOUND ONLINE
        while (head == NULL && active_jobs > 0) {
            pthread_cond_wait(&queue_not_empty, &lock);
        } // FOUND ONLINE

        if (active_jobs == 0) {
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

        while (instructionsCompleted < maxInstructionsRR && current->pc < current->length) // while there are still commands and max of 2 has not been reached
        {
            char *line = mem_get_code_line(current->startIndex + current->pc);
            if (line != NULL)
            {
                errCode = parseInput(line);
            }
            current->pc++;
            instructionsCompleted++;
        }

        if (current->pc < current->length) // if there are still commands in the script that have not been ran
        {
            addToReadyQueue(current);
        }
        else // free memory
        {
            pthread_mutex_lock(&lock);
            active_jobs--;
            pthread_cond_broadcast(&queue_not_empty);
            pthread_mutex_unlock(&lock);
            free(current);
        }
    }
}
