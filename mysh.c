#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "wildcard.h"
#include "mysh.h"

#define BUFSIZE 1000
#define BYTES 1

void execPipe(arraylist_t *pipeArgs, arraylist_t *pipeOut, int pipeArgsSize, int argsize){
    
    int fd[2]; //declare read and write end of pipe
    pipe(fd); //init pipe
    
    /*EXECUTE LEFT HALF OF PIPE COMMAND*/
    if (fork() == 0){
        //child
        dup2(fd[1], STDOUT_FILENO); //set stdout to the write end of pipe
        processArgs(pipeArgs); //execute the left half of the pipe command
        exit(1);
    }
    close(fd[1]); //close write end of fd
   
   /*EXECUTE RIGHT HALF OF PIPE COMMAND*/
    if (fork() == 0){
        dup2(fd[0], STDIN_FILENO); //set stdin to read end of pipe
        processArgs(pipeOut); //execute the right half of the pipe command
    }

    //free what was in held in "left half" arglist
    for (int i = 0; i < pipeArgs->length; i++){
        free(pipeArgs->data[i]);
    }
    al_destroy(pipeArgs);
    free(pipeArgs);
    
    return;

}

//execProg() will execute a file and it given a filename/pathname and a list of arguments
void execProg(char *filename, char **arguments, char *outputFile, char *inputFile){

    //initialize process id and fork()
    pid_t pid = fork();

    if (pid == 0){
        //run the requested file and pass in the argument list

        // Child process
        if (outputFile!=NULL)
        {
            int fd = open(outputFile,O_CREAT | O_TRUNC | O_WRONLY, 0640);
            //Sets standardout to the fd of outputfile.
            //printf("fd: %d", fd);
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (inputFile != NULL)
        {
            //close(STDIN_FILENO);
            int fd = open(inputFile, O_RDONLY, S_IWUSR | S_IRUSR);
            //Sets standardout to the fd of outputfile.
            //printf("fd: %d", fd);
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        execv(filename, arguments);
      
        //child should not reach here , if it does, print error and exit
        printf("whoooops! error"); 
        exit(1);
    }
    else if (pid > 0){
        int child_status;
        wait(&child_status);
        return;
    } 
    else {
        perror("fork failed");
    }
}

void freeList(char **argList, int truncSize)
{
    //free each object in the argument list
    for(int i = 0; i < truncSize; i++){
        free(argList[i]);
    }
    //free the argument list itself
    free(argList);
    return;
}


//processArgs() determines what the argument list consists of 
//this is where we will process and handle:


void checkBareNames(arraylist_t *argList, int argsize, char *outputFile, char *inputFile){

    char **arguments = argList->data;
    
    //If the first argument is not a shell built in or a local program, it is a barename 
    char *filename = arguments[0];
    if (filename == NULL) {return;}
    
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
        execProg(check1, arguments, outputFile, inputFile);
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
        execProg(check2, arguments, outputFile, inputFile); //execute
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
        execProg(check3, arguments, outputFile, inputFile); //execute
        free(check3);
        return;
    } else{
        free(check3);
    }

}

/*Process args will process the actual commands*/
int processArgs(arraylist_t *arguments){

    //set the first argument to filename
    char *filename = arguments->data[0];
    
    //set argument size
    int argsize = arguments->length;
    
    //check that the provided file is not null
    if (!(filename)){
        return argsize;
    } 
    if (strcmp(filename, "exit") == 0){
        printf("Exiting...\n");
        exit(1);
    }

    
    /* WILD CARD EXPANSION HERE*/
    //once the argument list has been expanded, we can handle all other cases 
   
    for (int i=1; i < argsize ;i++)
    {
        //if argument has a wildcard, start expansion.
        if (arguments->data[i] == NULL) {continue;}
        if (strchr(arguments->data[i], '*')!=NULL)
        {
            argsize = startExpansion(arguments->data[i], *arguments, arguments->data, i, argsize);
        }
    } 
    
    //update array length to argsize
    arguments->length = argsize;


    //Add Null to end of argument list to denote the end of the list (this is needed for execv)
    //arguments[argsize] = NULL; 
    al_push(arguments, NULL);

    int redirectFlag = 0;
    char *outputFile = NULL;
    char *inputFile = NULL;
    arraylist_t *list = al_create(100);

    for (int i = 0; i < argsize; i++){
        if (arguments->data[i] == NULL){continue;}
        if (strcmp(arguments->data[i], ">")==0 && arguments->data[i+1] != NULL)
        {
            //printf("Found >\n");
            redirectFlag = 1;
            outputFile = arguments->data[i+1];
            //printf("Current Output File: %s\n", outputFile);
            //printf("Index: %d\n", i);
            i = i+1;
        }
        else if (strcmp(arguments->data[i], "<")==0 && arguments->data[i+1] != NULL)
        {
            //printf("Found >\n");
            redirectFlag = 1;
            inputFile = arguments->data[i+1];
            //printf("Current Input File: %s\n", inputFile);
            i = i+1;
        }
        else{
            al_push(list, arguments->data[i]);
            //printf("Pushed %s\n", arguments[i]);
        } 
    }

    char *n;
    int newLength = al_length(list);
    int k = newLength-1;
    //printf("AL length: %d\n", al_length(list));
    
    arraylist_t *truncArgs = al_create(sizeof(arguments->data[0])*(newLength+2));
    while ((n = al_pop(list)) != NULL) {
	    //printf("Popped %s\n", n);
        //truncArgs->data[k] = n;
        truncArgs->data[k] = malloc(strlen(n) + 1);
        strcpy(truncArgs->data[k], n);
        truncArgs->length = truncArgs->length + 1;
        k = k-1;
    }
   
    //al_destroy(list);
    //free(list);
    al_push(truncArgs, NULL);
    

   if (redirectFlag == 1){
      
        //free(arguments);
        
        arguments->data = truncArgs->data;
        arguments->length = truncArgs->length - 1;
        arguments->size = truncArgs->size;
        //free(truncArgs);
        
    } else{
        for (int i = 0; i < truncArgs->length; i++){
            free(truncArgs->data[i]);
        }
        al_destroy(truncArgs);
        free(truncArgs);
    }

    //if the user types "exit", exit out of the shell
    

    //create the "left half" pipe argument list
    arraylist_t *pipeArgList = al_create(100);
    
    for(int i = 0; i < arguments->length; i++){
        if (arguments->data[i] == NULL) {continue;} //handle case where curr element is NULL
        for(int j = 0; j < strlen(arguments->data[i]); j++){
            //if you encounter a '|' char
            if(arguments->data[i][j] == 124){
                //Terminate the left half pipe argument array list
                al_push(pipeArgList, NULL); 
                //save the right half of command (whatever is after '|')
                char **pipeOutputData = (arguments->data) + i + 1;
                //declare new arraylist
                arraylist_t pipeOutput;
                //populate the arraylist with the right half of pipe command
                pipeOutput.data = pipeOutputData;
                //update length of the "right half" array 
                pipeOutput.length = argsize - i;
                //execute the left and right side commands
                execPipe(pipeArgList, &pipeOutput, i, argsize);

                return argsize;
            }
        }
        // if havent encountered a '|' yet then continue building "left half" arraylist
        char *newArg = malloc(strlen(arguments->data[i]) + 1); //allocate space for argument
        strcpy(newArg, arguments->data[i]); //copy argument over to newArg
        al_push(pipeArgList, newArg); //add to arraylist
        
    }

 
    

    
    /*EXECTUION OF LOCAL PROGRAMS*/
    //check if first argument contains a '/' this indicates that this will be local program
    filename = arguments->data[0];
    for (int i = 0; i < strlen(filename); i++){
        if(filename[i] == '/'){
            execProg(filename, arguments->data, outputFile, inputFile); //execute program 
            return argsize;
        }
    }

    /*SHELL BUILT INS HERE*/
    

    /*BARENAME CHECK HERE*/
    checkBareNames(arguments, argsize, outputFile, inputFile);

    for (int i = 0; i < arguments->length; i++){
        free(arguments->data[i]);
    }
    for (int i = 0; i < pipeArgList->length; i++){
        free(pipeArgList->data[i]);
    }

    

    al_destroy(pipeArgList);
    free(pipeArgList);

    return argsize;
}



//acceptArgs will iterate through the buffer and collect a sequence of tokens (arguments)
void acceptArgs(char *buf, int bytes){
    //create and initialize argument arraylist
    arraylist_t *argList = al_create(100);
    
    //ensure the array list was created properly
    if (argList == NULL){
        exit(1);
    }

    int start = 0; 
    for (int i = 0; i < bytes; i++){
        //if you encounter a space, this marks the end of a single argument
        if (i > 0 && isspace(buf[i]) && !(isspace(buf[i - 1]))){ 
            char *arg = malloc(i - start + 1); //allocate space for the current argument
            memcpy(arg, buf + start, i - start); //copy the argument from buf to arg
            
            //check that the current argument isn't a space or newline
            if (*arg == 32 || *arg == 10){
                start = i + 1;
                continue;
            } 

            //null terminate the argument
            arg[i -  start] = '\0'; 

            //add argument to arraylist
            al_push(argList, arg);
            start = i + 1; //update start 
            
        }
    }

    //pass the arraylist to processArgs
    processArgs(argList);
    
    //free the argument list 
    
    /*
    for (int i = 0; i < argList->length; i++){
        free(argList->data[i]);
    }
    al_destroy(argList);
    free(argList); */
    
    
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