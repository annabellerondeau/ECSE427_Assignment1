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
    int maxInstructions = 2; // for RR

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
             
            while (instructionsCompleted < maxInstructions && current->pc < current->length) // while there are still commands and max of 2 has not been reached
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