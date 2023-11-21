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

void execProg(char *filename, char **arguments){

    pid_t pid = fork();

    if (pid == 0){
        execv(filename, arguments);
        printf("whoooops! error");
        exit(1);
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
            execProg(arguments[0], arguments);
            return;
        }
    }
    //otherwise check in usr/local/bin , /usr/bin, /bin in that order 
    char *check1 = malloc(strlen("/usr/local/bin/") + strlen(arguments[0]) + 1);
    memcpy(check1, "/usr/local/bin/", strlen("/usr/local/bin/"));
    memcpy(check1 + strlen("/usr/local/bin/"), arguments[0], strlen(arguments[0]));
    check1[strlen("/usr/local/bin/") + strlen(arguments[0])] = '\0';
    if (access(check1, F_OK) == 0){
        execProg(check1, arguments);
        free(check1);
        return;
    } else {
        free(check1);
    }
    
    
    char *check2 = malloc(strlen("/usr/bin/") + strlen(arguments[0])+ 1 );
    memcpy(check2, "/usr/bin/", strlen("/usr/bin/"));
    memcpy(check2 + strlen("/usr/bin/"), arguments[0], strlen(arguments[0]));
    check2[strlen("/usr/bin/") + strlen(arguments[0])] = '\0';
    if (access(check2, F_OK) == 0){
        execProg(check2, arguments);
        free(check2);
        return;
    } else {
        free(check2);
    }
    
    
    char *check3 = malloc(strlen("/bin/") + strlen(arguments[0]) + 1);
    memcpy(check3, "/bin/", strlen("/bin/"));
    memcpy(check3 + strlen("/bin/"), arguments[0], strlen(arguments[0]));
    check2[strlen("/bin/") + strlen(arguments[0])] = '\0';
    if (access(check3, F_OK) == 0){
        execProg(check3, arguments);
        free(check3);
        return;
    } else{
        free(check3);
    }
    
    return;
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