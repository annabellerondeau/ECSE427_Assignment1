#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"
#include "scheduler.h"

int parseInput(char ui[]);
pthread_t mainThreadID;
int mainThreadInitialized = 0;

// Start of everything
int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0); // prevent buffering of printf output
    printf("Shell version 1.5 created Dec 2025\n");

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

    mainThreadID = pthread_self(); // store main thread ID for later use in quit command
    mainThreadInitialized = 1; // set flag to indicate main thread ID has been initialized

    //init user input
    for (int i = 0; i < MAX_USER_INPUT; i++) {
        userInput[i] = '\0';
    }
    
    //init shell memory
    mem_init();
    while(1) {	
        if (isatty(fileno(stdin))) // print prompt in interactive mode
        {
            printf("%c ", prompt);
        }						
        // here you should check the unistd library 
        // so that you can find a way to not display $ in the batch mode
        if (fgets(userInput, MAX_USER_INPUT-1, stdin) == NULL) // break on EOF
        {
            if (!isReadyQueueEmpty()) // if there are still processes in the ready queue, run the scheduler before exiting
            {
                scheduler();
            }
            break;
        }

        // edge case: if user hits a blank line, contiue to next iteration
        if (strcmp(userInput, "\n") == 0 || userInput[0] == '\0') 
        { 
            if (!isReadyQueueEmpty()) 
            {
                scheduler();
            }
            continue; // Skip the rest of the loop
        }

        errorCode = parseInput(userInput);
        if (errorCode == -1) exit(99);	// ignore all other errors
        memset(userInput, 0, sizeof(userInput));
    }

    return 0;
}

int wordEnding(char c) {
    // You may want to add ';' to this at some point,
    // or you may want to find a different way to implement chains.
    return c == '\0' || c == '\n' || c == ' ' || c == ';';
}

// 1.2.4 One liners
int parseInput(char inp[]) {
    char tmp[200], *words[100];
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    int numberOfCommands = 0;

    while (inp[ix] != '\0' && inp[ix] != '\n' && numberOfCommands < 10){ // Check if string of commands is done
        w=0;
        for (; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces

        while (inp[ix] != '\n' && inp[ix] != '\0' && inp[ix]!= ';' && ix < 1000) {

            // extract a word
            for (wordlen = 0; !wordEnding(inp[ix]) && inp[ix]!= ';' && ix < 1000; ix++, wordlen++) {
                tmp[wordlen] = inp[ix];
            }

            tmp[wordlen] = '\0';
            words[w] = strdup(tmp);
            w++;
            if (inp[ix] == '\0' || inp[ix] == ';' || inp[ix] == '\n') break; // break from loop if command is completely parsed
            ix++;
        }
        if (w != 0) errorCode = interpreter(words, w); // in case there is 'ls;  ; mkdir $var'
        numberOfCommands++; // increment command count
        if (inp[ix] == ';') ix++; // skip ';' character
    }
    return errorCode;
}