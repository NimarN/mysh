#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "wildcard.h"

#define BUFSIZE 1000
#define BYTES 200

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
void processArgs(char **arguments, int argsize){

    
    /* WILD CARD EXPANSION HERE*/
    //once the argument list has been expanded, we can handle all other cases 
    //printf("ArgSize Before: %d\n", argsize);
    //printf("Argument 0: %s\n", arguments[0]);
    for (int i=1; i < argsize;i++)
    {
        //printf("Arg at %d: %s\n", i, arguments[i]);
        //if argument has a wildcard, start expansion.
        if (strchr(arguments[i], '*')!=NULL)
        {
            //printf("Found argument with wildcard\n");
            argsize = startExpansion(arguments[i], arguments, i, argsize);
        }
    }

    //printf("ArgSize After: %d\n", argsize);
    //printf("Argument 0: %s\n", arguments[0]);

    char *filename = arguments[0];
        
    //Add Null to end of argument list to denote the end of the list (this is needed for execv)
    arguments[argsize] = NULL; 
    

    //if the user types "exit", exit out of the shell
    if (strcmp(filename, "exit") == 0){
        printf("Exiting...\n");
        exit(1);
    }
    
    //check if first argument contains a '/' this indicates that this will be local program
    for (int i = 0; i < strlen(filename); i++){
        if(filename[i] == '/'){
            //printf("Filename: %s|\n", filename);
            execProg(filename, arguments); //execute program 
            return; 
        }
    }

    /*SHELL BUILT INS HERE*/
    

    //If the first argument is not a shell built in or a local program, it is a barename 
    
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
    
    return;
}



//acceptArgs will iterate through the buffer and collect a sequence of tokens (arguments)
void acceptArgs(char *buf, int bytes){
    char **arguments = malloc(100); //allocate space for our argument list
    int argsize = 0; //initial argument list size
    int start = 0; 
    for (int i = 0; i < bytes; i++){

        //if you encounter a space, this marks the end of a single argument
        if (isspace(buf[i])){ 
            char *arg = malloc(i - start + 1); //allocate space for the current argument
            memcpy(arg, buf + start, i - start); //copy the argument from buf to arg
            arg[i -  start] = '\0'; 
            arguments[argsize] = arg; //add the argument to argument list
            argsize+= 1; //update size
            start = i + 1; //update start 
            
        }
    }

    //pass the argument list and size to processArgs
    processArgs(arguments, argsize);

    //free each object in the argument list
    for(int i = 0; i < argsize; i++){
        free(arguments[i]);
    }
    //free the argument list itself
    free(arguments);
}

int myshell(){
    
    char *greeting = "Welcome to mysh! :) \n"; //print greeting
    write(1, greeting, strlen(greeting)); //write the greeting to stdout
    char *buf = malloc(BUFSIZE); //allocate buffer on the heap
    
    int bytes;
    
    //this loop will always run until "exit" is written as the first argument
    while(1){

        write(1, "mysh> ", 6);
        bytes = read(0, buf, BUFSIZE);
        char *input = malloc(bytes + 1);
        memcpy(input, buf, bytes+1);
        input[bytes] = '\0';
        acceptArgs(input, bytes); 

        free(input);
    }
    
    return 1;
}

int main(int argc, char **argv){
    myshell();
    return 1;
}