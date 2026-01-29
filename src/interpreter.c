#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shellmemory.h"
#include "shell.h"

int MAX_ARGS_SIZE = 3;

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
int echo(char *text);

// Interpret commands and their arguments
int interpreter(char *command_args[], int args_size) {
    int i;

    if (args_size < 1 || args_size > MAX_ARGS_SIZE) {
        return badcommand();
    }

    for (i = 0; i < args_size; i++) {   // terminate args at newlines
        command_args[i][strcspn(command_args[i], "\r\n")] = 0;
    }

    if (strcmp(command_args[0], "help") == 0) {
        //help
        if (args_size != 1)
            return badcommand();
        return help();

    } else if (strcmp(command_args[0], "quit") == 0) {
        //quit
        if (args_size != 1)
            return badcommand();
        return quit();

    } else if (strcmp(command_args[0], "set") == 0) {
        //set
        if (args_size != 3)
            return badcommand();
        return set(command_args[1], command_args[2]);

    } else if (strcmp(command_args[0], "print") == 0) {
        if (args_size != 2)
            return badcommand();
        return print(command_args[1]);

    } else if (strcmp(command_args[0], "source") == 0) {
        if (args_size != 2)
            return badcommand();
        return source(command_args[1]);

    } else if (strcmp(command_args[0], "echo") == 0) {
        if (args_size != 2)
            return badcommand();
        return echo(command_args[1]);

    }  else if (strcmp(command_args[0], "run") == 0) {
        if (args_size < 2)
            return badcommand();
        int pid = fork();

        if (pid == 0) // child process
        {
            char *execArgs[MAX_ARGS_SIZE + 1]; // leave space for NULL
            int i;
            for (i = 0; i < args_size - 1; i++) // parse commands
            {
                execArgs[i] = command_args[i + 1]; // skip run
            }
            execArgs[i] = NULL;
            execvp(execArgs[0], execArgs); // execute command

            if (execvp(execArgs[0], execArgs) == -1) { // failure in exec
                badcommand(); 
                exit(1); 
            }

            exit(0); // terminate child process
        }
        else if (pid > 0) // parent process
        {
            wait(NULL); // wait for child to finish
            return 0;
        }
    } else
        return badcommand();
}

int help() {

    // note the literal tab characters here for alignment
    char help_string[] = "COMMAND			DESCRIPTION\n \
help			Displays all the commands\n \
quit			Exits / terminates the shell with “Bye!”\n \
set VAR STRING		Assigns a value to shell memory\n \
print VAR		Displays the STRING assigned to VAR\n \
source SCRIPT.TXT	Executes the file SCRIPT.TXT\n ";
    printf("%s\n", help_string);
    return 0;
}

int quit() {
    printf("Bye!\n");
    exit(0);
}

int set(char *var, char *value) {
    // Challenge: allow setting VAR to the rest of the input line,
    // possibly including spaces.

    // Hint: Since "value" might contain multiple tokens, you'll need to loop
    // through them, concatenate each token to the buffer, and handle spacing
    // appropriately. Investigate how `strcat` works and how you can use it
    // effectively here.

    mem_set_value(var, value);
    return 0;
}


int print(char *var) {
    printf("%s\n", mem_get_value(var));
    return 0;
}

int source(char *script) {
    int errCode = 0;
    char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        return badcommandFileDoesNotExist();
    }

    //fgets(line, MAX_USER_INPUT - 1, p);
    while (fgets(line, MAX_USER_INPUT - 1, p) != NULL) {
        errCode = parseInput(line);     // which calls interpreter()
        memset(line, 0, sizeof(line));

        if (feof(p)) {
            break;
        }
    }

    fclose(p);

    return errCode;
}

int echo (char *text) 
{
    int errCode = 0;
    if (text[0] == '$') 
    {
        char *varName = text + 1; // skip $
        char *value = mem_get_value(varName); // get variable value
        printf("%s\n", value);
    } 
    else 
    {
        printf("%s\n", text); // print the text
    }
    return 0;
}
