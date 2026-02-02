#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
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
// Adding a break for mine - Remove later
int my_ls();
int my_mkdir(char filename[]);
int my_cd(char *dirname);
int my_touch(char *filename);
int isAlphaNumeric(char word[]);
int tokenEnding(char c);
int prioritization(const void *c1, const void *c2);

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
    } else if (strcmp(command_args[0], "my_ls") == 0) {
        if (args_size != 1) return badcommand();
        return my_ls();
    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2) return badcommand();
        return my_mkdir(command_args[1]);
    } else if (strcmp(command_args[0], "my_touch") == 0) {
      if (args_size != 2) return badcommand();
      return my_touch(command_args[1]);
    }else if (strcmp(command_args[0], "my_cd") == 0) {
       if (args_size != 2) return badcommand();
       return my_cd(command_args[1]);
    }else{
        return badcommand();}
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

int my_ls(){
    char *tmp[1000];
    int w = 0;
    int p = 0;
    DIR *currentDirectory= opendir(".");
    struct dirent *output = readdir(currentDirectory);
    while (output!= NULL){ // "It returns a null pointer upon reaching the end of the directory stream."
        tmp[w++] = strdup(output->d_name);
        output = readdir(currentDirectory);
    }
    closedir(currentDirectory); // to avoid handling errors

    // step 2: sort tmp

    qsort(tmp, w, sizeof(tmp[0]), prioritization);

    while(w!=p){
        printf("%s\n", tmp[p++]);
        //free(tmp[p++]); //necessary ?
    }
    return 0;
}


// int (*compar)(const void *, const void *)
int prioritization(const void *c1, const void *c2){
    return strcmp(*(const char **) c1,*(const char **)c2); // we cast c1
}

int my_mkdir(char filename[]){ // parse for alphanumeric

    // Direct alphanumerical string
    if (filename[0] != '$'){
        if (isAlphaNumeric(filename)){
            mkdir(filename, 0777);
            return 0;}
            else{
                printf("%s\n", "Bad command: my_mkdir");
                return 1;
            }
    }

    // VAR
    else {
    filename++;
    char *value = mem_get_value(filename);
    if (value == NULL){ // is it found
       printf("%s\n", "Bad command: my_mkdir");
       return 1;
    }
    // is it a single token?
    if (isAlphaNumeric(value)){
    mkdir(value, 0777);
    return 0;
    }
    else{
        printf("%s\n", "Bad command: my_mkdir");
        return 1;
    }
}
}

int isAlphaNumeric(char word[]){
    int x =0;
        for (; !tokenEnding(word[x]); x++) {
           if (!isalnum((unsigned char) word[x])) {
                return 0;
           }
        }
    return 1;
}

int tokenEnding(char c) {
    return c == '\0' || c == '\n';
}

int my_touch(char *filename) // edge cases??
{
    FILE *newFile ;
    newFile = fopen(filename, "w");
    fclose(newFile);
    return 0; // return error code
}

int my_cd(char *dirname){
    if (chdir(dirname)!=0){
        printf("%s\n", "Bad command: my_cd");
    }
    return 0;
}