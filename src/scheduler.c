#include "scheduler.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PCB* head = NULL;
PCB* tail = NULL;

int scheduler()
{
    int errCode = 0;
    int maxInstructionsRR = 2; // for RR
    int maxInstructionsAging = 1; // for AGING

    while (head != NULL) 
    {
        PCB* current = head; /// pop head of queue
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        //printf("Running process with PID: %d\n", current->pid); // DEBUG

        if (strcmp(policy, "RR") == 0)
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

            if (current->pc < current->length) // if there are still commands in the script that have not been ran
            { 
                for (PCB* temp = head; temp != NULL; temp = temp->next) // update scores of processes in ready queue
                {
                    if (temp->score > 0) 
                    {
                        temp->score--;// decrement score of all processes in ready queue to implement aging
                    }
                }
                addToReadyQueue(current);
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
    return errCode;
}

void addToReadyQueue(PCB* process)
{
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