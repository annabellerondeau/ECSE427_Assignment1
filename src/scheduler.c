#include "scheduler.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include <stdio.h>
#include <stdlib.h>

PCB* head = NULL;
PCB* tail = NULL;

int scheduler()
{
    int errCode = 0;

    while (head != NULL) 
    {
        PCB* current = head; /// pop head of queue
        head = head->next;
        if (head == NULL)
        {
            tail = NULL;
        }
        //printf("Running process with PID: %d\n", current->pid); // DEBUG

        // FCFS 
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
    
    clearMemory(); 
    return errCode;
}

void addToReadyQueue(PCB* process)
{
    process->next = NULL; 

    if (head == NULL) 
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