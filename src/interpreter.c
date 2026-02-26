#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include "interpreter.h"
#include "shellmemory.h"
#include "shell.h"
#include "pcb.h"
#include "scheduler.h"


int MAX_ARGS_SIZE = 10; // arbitrary (to be removed)

int help();
int quit();
int set(char *var, char *value);
int print(char *var);
int source(char *script);
int badcommandFileDoesNotExist();
int echo(char *text);
int my_ls();
int my_mkdir(char filename[]);
int my_cd(char *dirname);
int my_touch(char *filename);
int isAlphaNumeric(char word[]);
int tokenEnding(char c);
int prioritization(const void *c1, const void *c2);
int exec(char **scriptsAndPolicy, int numOfArgs);

int badcommand() {
    printf("Unknown Command\n");
    return 1;
}

// For source command only
int badcommandFileDoesNotExist() {
    printf("Bad command: File not found\n");
    return 3;
}

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
        if (args_size != 1)
            return badcommand();
        return my_ls();

    } else if (strcmp(command_args[0], "my_mkdir") == 0) {
        if (args_size != 2)
            return badcommand();
        return my_mkdir(command_args[1]);

    } else if (strcmp(command_args[0], "my_touch") == 0) {
      if (args_size != 2)
            return badcommand();
      return my_touch(command_args[1]);

    }else if (strcmp(command_args[0], "my_cd") == 0) {
       if (args_size != 2)
            return badcommand();
       return my_cd(command_args[1]);

    }else if (strcmp(command_args[0], "exec") == 0) {
        if (args_size > 7 || args_size < 3)
            return badcommand();
        return exec(command_args+1, args_size-1);
    }
    else{ // if none of the known commands are input
    //}
        return badcommand();
    }
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
    if (mtFlag){ // if process was multithreaded
        pthread_join(t1,NULL);
        pthread_join(t2,NULL);
    }
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
    //char line[MAX_USER_INPUT];
    FILE *p = fopen(script, "rt");      // the program is in a file

    if (p == NULL) {
        printf("DEBUG p == NULL \n");
        return badcommandFileDoesNotExist();
    }

    int fileIndex;
    int length; 

    int load = loadFileMemory(p, &fileIndex, &length); // load file into memory
    fclose(p);

    if (load != 0) { // fix: error message ?
        printf("DEBUG load failed \n");
        return badcommandFileDoesNotExist();
    }

    PCB* process = createPCB(fileIndex, length); // create process for file in memory
    addToReadyQueue(process); // add process to ready queue

    errCode = scheduler(); // run processes in ready queue, returns error code if any 
    return errCode;

    // //fgets(line, MAX_USER_INPUT - 1, p);
    // while (fgets(line, MAX_USER_INPUT - 1, p) != NULL) {
    //     errCode = parseInput(line);     // which calls interpreter()
    //     memset(line, 0, sizeof(line));

    //     if (feof(p)) {
    //         break;
    //     }
    // }

    // fclose(p);

    // return errCode;
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
    char *tmp[1000]; // an array of pointers to each file
    int w = 0;
    int p = 0;
    DIR *currentDirectory= opendir("."); // open current directory
    struct dirent *output = readdir(currentDirectory); // read one file from current directory
    while (output!= NULL){ // "It returns a null pointer upon reaching the end of the directory stream."
        tmp[w++] = strdup(output -> d_name);
        output = readdir(currentDirectory);
    }
    closedir(currentDirectory); // to avoid handling errors

    qsort(tmp, w, sizeof(tmp[0]), prioritization); // sort tmp

    while(w!=p){
        printf("%s\n", tmp[p++]);
    }
    return 0;
}

// character comparison function for qsort of my_ls
int prioritization(const void *c1, const void *c2){
    return strcmp(*(const char **) c1,*(const char **)c2); // cast characters to strcmp input type
}                                                          // and return strcmp comparison of both chars

// creates a new directory with the name dirname in the current directory
int my_mkdir(char filename[]){

    // filename is a string
    if (filename[0] != '$'){
        if (isAlphaNumeric(filename)){
            mkdir(filename, 0777); // file made with full permission 0777
            return 0;
        }else{
            printf("%s\n", "Bad command: my_mkdir");
            return 1;
        }
    }

    // filename is a $VAR
    else {
        filename++; // ignore '$'
        char *value = mem_get_value(filename);
        if (value == NULL){ // there is no associated value
           printf("%s\n", "Bad command: my_mkdir");
           return 1;
        }

        if (isAlphaNumeric(value)){ // it is a single alphanumeric token
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
    int x;
        for (x=0; !tokenEnding(word[x]); x++) { // while we haven't reached the end of the word
           if (!isalnum((unsigned char) word[x])) {  // isalnum returns whether a character is alphanumeric
                return 0; // false
           }
        }
    return 1; // true
}

int tokenEnding(char c) {
    return c == '\0' || c == '\n'; // returns whether the token has reached a terminating character
}

// creates a new empty file inside the current directory
int my_touch(char *filename)
{
    FILE *newFile ;
    newFile = fopen(filename, "w"); // open file in write mode: if the file does not exist it
                                    // is created, if it does it is truncated to zero length
    fclose(newFile);
    return 0; // operation is successful
}

// changes current directory to directory dirname, inside the current directory
int my_cd(char *dirname){
    if (chdir(dirname)!=0){ // function chdir returns 0 when the move to 'dirname' is successful
        printf("%s\n", "Bad command: my_cd");
        return 1; // operation is unsuccessful
    }
    return 0; // operation is successful
}

//3h
 int exec(char *scriptsAndPolicy[], int numOfArgs){

     // Identify flags
     int backgroundFlag = 0; // 0 means off
     int endIndex = numOfArgs-1;

     if ((strcmp(scriptsAndPolicy[endIndex], "MT") == 0)){
         mtFlag = 1; // remains enabled for duration of test case
         endIndex--;
     }
     if ((strcmp(scriptsAndPolicy[endIndex], "#") == 0)){
         backgroundFlag = 1;
         endIndex--;
     }

     // Identify policy
     policy = scriptsAndPolicy[endIndex];

     if ((strcmp(policy, "FCFS") != 0) && (strcmp(policy, "SJF") != 0) // Validate Policy
         && (strcmp(policy, "RR") != 0) && (strcmp(policy, "AGING") != 0)
         && (strcmp(policy, "RR30") != 0)) {
         printf("%s\n", "ERROR: Policy must be one of FCFS/SJF/RR(30)/AGING");
         return 1;
     }

     // Identify scripts
     int numOfProgs = endIndex;
     char *scripts[numOfProgs];
     for (int i=0; i<numOfProgs; i++){
          scripts[i] = scriptsAndPolicy[i];
          for(int j=0; j<i; j++)
             if (strcmp(scripts[i], scripts[j]) == 0){
                 printf("%s\n", "ERROR: No duplicate scripts allowed ):");
                 return 1;
             }
     }

     // initialize variables
     PCB* processArray[numOfProgs]; // create process for file in memory
     //int processSizeArray[numOfProgs];

     // First load everything
     for (int counter=0; numOfProgs > counter; counter++){
         FILE *p = fopen(scripts[counter], "rt");      // the program is in a file

         if (p == NULL) {
             return badcommandFileDoesNotExist();
         }

         int fileIndex;
         int length;

         int load = loadFileMemory(p, &fileIndex, &length); // load file into memory
         fclose(p);

         if (load != 0) {
             // FEATURE : CLEAR MEMORY
             // OPTIONAL: free the array of pcbs....
             clearMemory();
             return badcommandFileDoesNotExist();
         }
         processArray[counter] = createPCB(fileIndex, length);
         // processSizeArray[counter] = length; // For SJF
     }

     // Second add everything to queue
     for (int counter=0; numOfProgs > counter; counter++){
         addToReadyQueue(processArray[counter]); // add process to ready queue
     }

     if (backgroundFlag == 0)
     {
        return scheduler(); // Scheduler clears memory
     }

     return 0; // if background flag is on we don't run the scheduler so we return 0 for success
 }