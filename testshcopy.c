#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "wildcard.h"
#include "testsh.h"
#include "arraylist.h"


#define BUFSIZE 1000
#define BYTES 1

void execPipe(char **pipeArgs, char **pipeOut, int pipeArgsSize, int argsize){
    int fd[2]; //declare read and write end of pipe
    pipe(fd); //init pipe

    if (fork() == 0){
        //child
        dup2(fd[1], STDOUT_FILENO); //set stdout to the write end of pipe
        processArgs(pipeArgs, pipeArgsSize); //execute the left half of the pipe command
        exit(1);
    }
    close(fd[1]); //close write end of fd
   

    if (fork() == 0){
        dup2(fd[0], STDIN_FILENO); //set stdin to read end of pipe
        processArgs(pipeOut, argsize - pipeArgsSize - 1); //execute the right half of the pipe command
    }

    for (int i = 0; i < pipeArgsSize; i++){
        free(pipeArgs[i]); //free the individual objects of the pipe argument list (left half)
    } 

    free(pipeArgs); //free the actual pipe

    return;

}

//execProg() will execute a file and it given a filename/pathname and a list of arguments
void execProg(char *filename, char **arguments){

    //initialize process id and fork()
    pid_t pid = fork();

    if (pid == 0){
        //run the requested file and pass in the argument list
        execv(filename, arguments);

        //child should not reach here , if it does, print error and exit
        printf("whoooops! error"); 
        exit(1);
    }

    int child_status; 
    wait(&child_status);
    return;
}


//processArgs() determines what the argument list consists of 
//this is where we will process and handle:

//  -wildcards 
//  -barenames
//  -builtins
//  -conditionals 
//  -pipes 
//  -redirections 
//  -execution of local programs 
//  -exiting the shell 

void checkBareNames(char **arguments, int argsize){

    //If the first argument is not a shell built in or a local program, it is a barename 
    char *filename = arguments[0];
    
    //we are building this string: check1 = "usr/local/bin/<filename>"
    //allocate enough space for the full pathfile and a null terminator 
    char *check1 = malloc(strlen("/usr/local/bin/") + strlen(filename) + 1);
    //copy over the path 
    memcpy(check1, "/usr/local/bin/", strlen("/usr/local/bin/"));
    //copy over the filename 
    memcpy(check1 + strlen("/usr/local/bin/"), filename, strlen(filename));
    //add null terminator to check1
    check1[strlen("/usr/local/bin/") + strlen(filename)] = '\0';
    
    //if the current program is in "/usr/local/bin/" then execute it
    if (access(check1, F_OK) == 0){
        execProg(check1, arguments);
        free(check1); //free check1 string and return 
        return;
    } else {
        // free check1 
        free(check1);
    }
    
    //we are building this string: check2 = "/usr/bin/<filename>"
    //allocate enough space for the full pathfile and a null terminator 
    char *check2 = malloc(strlen("/usr/bin/") + strlen(filename)+ 1 );
    //copy over the path 
    memcpy(check2, "/usr/bin/", strlen("/usr/bin/"));
    //copy over the filename
    memcpy(check2 + strlen("/usr/bin/"), filename, strlen(filename));
    //add null term
    check2[strlen("/usr/bin/") + strlen(filename)] = '\0';

    //check if program is in "/usr/bin"
    if (access(check2, F_OK) == 0){
        execProg(check2, arguments); //execute
        free(check2);
        return;
    } else {
        free(check2);
    }
    
    
    //we are building this string: check3 = "/bin/<filename>"
    //allocate enough space for the full pathfile and a null terminator 
    char *check3 = malloc(strlen("/bin/") + strlen(filename) + 1);
    //copy over the path 
    memcpy(check3, "/bin/", strlen("/bin/"));
    //copy over the filename
    memcpy(check3 + strlen("/bin/"), filename, strlen(filename));
    //add null term
    check3[strlen("/bin/") + strlen(filename)] = '\0';

    //check if curr prog is in "/bin"
    if (access(check3, F_OK) == 0){
        execProg(check3, arguments); //execute
        free(check3);
        return;
    } else{
        free(check3);
    }

}
int processArgs(char **arguments, int argsize){

    char *filename = arguments[0];
    
    if (!(filename)){
        return argsize;
    } 

    
    /* WILD CARD EXPANSION HERE*/
    //once the argument list has been expanded, we can handle all other cases 
    //printf("ArgSize Before: %d\n", argsize);
    //printf("Argument 0: %s\n", arguments[0]);
    for (int i=1; i < argsize;i++)
    {
        //printf("Arg at %d: %s\n", i, arguments[i]);
        //if argument has a wildcard, start expansion.
        if (arguments[i] == NULL) {continue;}
        if (strchr(arguments[i], '*')!=NULL)
        {
            //printf("Found argument with wildcard\n");
            argsize = startExpansion(arguments[i], arguments, i, argsize);
        }
    }

    //printf("ArgSize After: %d\n", argsize);
    //printf("Argument 0: %s\n", arguments[0]);

    
        
    //Add Null to end of argument list to denote the end of the list (this is needed for execv)
    arguments[argsize] = NULL; 
    

    //ensure that the file is not null
    

    //if the user types "exit", exit out of the shell
    if (strcmp(filename, "exit") == 0){
        printf("Exiting...\n");
        exit(1);
    }

    
    char **pipeArgList = malloc(100);
    
    
    int pipeFlag = 0;
    for(int i = 0; i < argsize ; i++){
        //pipeArgList[i] = arguments[i];
        if (arguments[i] == NULL) {continue;}
        for(int j = 0; j < strlen(arguments[i]); j++){
            if(arguments[i][j] == 124){
                pipeFlag = 1;
                pipeArgList[i] = NULL;
                char **pipeOutput = arguments + i + 1;
                execPipe(pipeArgList, pipeOutput, i, argsize);
                return argsize;
                
            }
        }
        pipeArgList[i] = malloc(strlen(arguments[i]) + 1);
        strcpy(pipeArgList[i], arguments[i]);
        
    }

    if (pipeFlag == 0){
        for(int i = 0; i < argsize; i++){
            if (pipeArgList[i] == NULL){continue;}
            free(pipeArgList[i]);
        } 
    }
    free(pipeArgList);

    
    
    
    //check if first argument contains a '/' this indicates that this will be local program
    for (int i = 0; i < strlen(filename); i++){
        if(filename[i] == '/'){
            //printf("Filename: %s|\n", filename);
            execProg(filename, arguments); //execute program 
            return argsize;
        }
    }

    /*SHELL BUILT INS HERE*/
    

    /*BARENAME CHECK HERE*/
    
    checkBareNames(arguments, argsize);
    
    return argsize;

    //print error if command is not recognized
    //printf("Error: invalid command :(\n");

}



//acceptArgs will iterate through the buffer and collect a sequence of tokens (arguments)
void acceptArgs(char *buf, int bytes){
    char **arguments = malloc(100); //allocate space for our argument list
    int argsize = 0; //initial argument list size
    int start = 0; 
    for (int i = 0; i < bytes; i++){

        //if you encounter a space, this marks the end of a single argument
        if (i > 0 && isspace(buf[i]) && !(isspace(buf[i - 1]))){ 
            char *arg = malloc(i - start + 1); //allocate space for the current argument
            memcpy(arg, buf + start, i - start); //copy the argument from buf to arg
            
            //check that the current argument isn't a space
            
            if (*arg == 32 || *arg == 10){
                start = i + 1;
                continue;
            } 

            arg[i -  start] = '\0'; 
            arguments[argsize] = arg; //add the argument to argument list
            argsize+= 1; //update size
            start = i + 1; //update start 
            
        }
    }

    //pass the argument list and size to processArgs
    argsize = processArgs(arguments, argsize);

    //free each object in the argument list
    for(int i = 0; i < argsize; i++){
        free(arguments[i]);
    }
    //free the argument list itself
    free(arguments);
}

int main(int argc, char **argv){
    
    char *greeting = "Welcome to mysh! :) \n"; //print greeting
    int fd, command_length, bytes, start = 0;
    char *buf = malloc(BUFSIZE); //allocate buffer on the heap
    char *input = NULL;
    int bytesLeftover = 0;

    write(1, greeting, strlen(greeting)); //write the greeting to stdout
    
    //Check is user provides a file for batch mode, open if file provided
    if (argc > 1){
        fd = open(argv[1], O_RDONLY);
        if (fd < 0){
            perror(argv[1]);
            exit(EXIT_FAILURE);
        } 
    } else {
        //if no file provided take input from stdin
        fd = STDIN_FILENO;
    }

    while(1){
        //prompt message
        write(1, "mysh> ", 6);
        //read respective file
        bytes = read(fd, buf , BUFSIZE);
        if (bytes == 0){
            exit(EXIT_SUCCESS);
        }
        
        
        /* THIS IS FOR BATCH MODE !!*/
        if (fd != 0){ 
            for(int i = 0; i < bytes; i++){
                if(buf[i] == '\n'){ //we have reached the end of a command
                    command_length = i - start;  //record the length of the command
                    input = realloc(input, command_length + 1); //allocate space to hold command
                    memcpy(input, buf + start, command_length); //copy command into input
                    input[command_length] = '\0'; //null terminate the input 
                    
                    acceptArgs(input, command_length);
                    
                    start = i + 1; //update start
                    command_length = 0; //refresh command length
                } 
            }
            //free(input);
        } else{
            /* THIS IS FOR INTERACTIVE MODE !!*/
            char *input = malloc(bytes + 1); //allocate memory for input
            memcpy(input + bytesLeftover, buf, bytes+1); //copy the contents of the buffer into input
            //null terminate

            input[bytes] = '\0'; 
            //if user enters only a backspace just continue
            if (strcmp(input, "\n") == 0){
                free(input);
                continue;
            }
            
            acceptArgs(input, bytes); 
            free(input);
        }
    }
    return 1;
}