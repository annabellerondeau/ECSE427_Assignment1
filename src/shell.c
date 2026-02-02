#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h>
#include "shell.h"
#include "interpreter.h"
#include "shellmemory.h"

int parseInput(char ui[]);

// Start of everything
int main(int argc, char *argv[]) {
    printf("Shell version 1.5 created Dec 2025\n");

    char prompt = '$';  				// Shell prompt
    char userInput[MAX_USER_INPUT];		// user's input stored here
    int errorCode = 0;					// zero means no error, default

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
            break;
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

// There will be at most 10 chained instructions.
// Semicolon is the only accepted separator
int parseInput(char inp[]) {
    char tmp[200], *words[100];
    int ix = 0, w = 0;
    int wordlen;
    int errorCode;
    int numberOfCommands = 0;

    while (inp[ix] != '\0' && numberOfCommands < 10){ // Check if string of commands is done
        w=0;
        for (; inp[ix] == ' ' && ix < 1000; ix++); // skip white spaces
        while (inp[ix] != '\n' && inp[ix] != '\0' && inp[ix]!= ';' && numberOfCommands< 10 && ix < 1000) {
            // extract a word
            for (wordlen = 0; !wordEnding(inp[ix]) && inp[ix]!= ';' && numberOfCommands< 10 && ix < 1000; ix++, wordlen++) {
                tmp[wordlen] = inp[ix];
            }
            tmp[wordlen] = '\0';
            words[w] = strdup(tmp);
            w++;
            if (inp[ix] == '\0') break;
            ix++;
        }
        errorCode = interpreter(words, w);
        numberOfCommands++;
        if (inp[ix] == ';') ix++;
    }
    return errorCode; // how to detect error ?
}