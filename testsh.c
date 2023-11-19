#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFSIZE 1000
#define BYTES 200

void execProg(char **arguments){
    char *filename = arguments[0];
    //char **restOfTheArgs = arguments;
    

    pid_t pid = fork();

    if (pid == 0){
        execv(filename, arguments);
        printf("error");
    }

    int child_status;
    wait(&child_status);
    
}
void processArgs(char **arguments, int argsize){
    
    if (strcmp(arguments[0], "exit") == 0){
        printf("Exiting...\n");
        exit(1);
    }

    //check if first argument contains a '/'
    for (int i = 0; i < strlen(arguments[0]); i++){
        if(arguments[0][i] == '/'){
            execProg(arguments);
        }
    }
}

void acceptArgs(char *buf, int bytes){
    char **arguments = malloc(100);
    int argsize = 0;
    int start = 0;
    for (int i = 0; i < bytes; i++){
        if (isspace(buf[i])){
            char *arg = malloc(i - start + 1);
            memcpy(arg, buf + start, i - start);
            arg[i -  start] = '\0';
            arguments[argsize] = arg;
            argsize+= 1;
            start = i + 1;
            
        }
    }

    arguments[argsize] = NULL;
    
    
    processArgs(arguments, argsize);
    for(int i = 0; i < argsize; i++){
        free(arguments[i]);
    }
    free(arguments);
}

int myshell(){
    
    char *greeting = "Welcome to mysh! :) \n";
    write(1, greeting, strlen(greeting));
    char *buf = malloc(BUFSIZE);
    
    int bytes;
    
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