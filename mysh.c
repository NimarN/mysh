#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/limits.h>

#include "wildcard.h"
#include "mysh.h"

#define BUFSIZE 1000


//This will record the status of any executed program
//1 if it exits normally, 0 if signaled/exits with error
int statusFlag;

//----------------------------------------------------------------------------------------//


//execPipe will execute a pipe
void execPipe(arraylist_t *pipeArgs, arraylist_t *pipeOut, int pipeArgsSize, int argsize){
    
    int fd[2]; //declare read and write end of pipe
    pipe(fd); //init pipe
    
    /*EXECUTE LEFT HALF OF PIPE COMMAND*/
    if (fork() == 0){
        //this is the child
        dup2(fd[1], STDOUT_FILENO); //set stdout to the write end of pipe
        processArgs(pipeArgs); //execute the left half of the pipe command
        exit(1);
    }
    close(fd[1]); //close write end of fd
   
   /*EXECUTE RIGHT HALF OF PIPE COMMAND*/
    if (fork() == 0){
        //this is the child
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


//******************************************************************************************//


//execProg() will execute a file and it given a filename/pathname and a list of arguments
void execProg(char *filename, char **arguments, char *outputFile, char *inputFile, char *cwd, int argsize){
    

    /*int stdinfd = dup(0);
    int stdoutfd = dup(1); */
    //initialize process id and fork()
    pid_t pid = fork();
    //uncommenting below makes the output redirect work
    //printf(":");

    if (pid == 0){
        // Child process
        // check that output and input files are not null
        //uncommenting below makes the output redirect work
        //printf("1");
        if (outputFile!=NULL)
        {
            //set fd
            int fd = open(outputFile,O_CREAT | O_TRUNC | O_WRONLY, 0640);
            
            //Sets standardout to the fd of outputfile.
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        if (inputFile != NULL)
        {   
            //set fd
            int fd = open(inputFile, O_RDONLY, S_IWUSR | S_IRUSR);
            
            //Sets standardout to the fd of outputfile.
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        //**PWD AND WHICH ARE HERE**///

        if (strcmp(arguments[0], "pwd") == 0) {
            // pwd command
            write(1, "Built in PWD: ", strlen("Built in PWD: "));
            write(1, cwd, strlen(cwd));
            write(1, "\n", strlen("\n\0"));
          
            //success
            exit(1);
        }

        if (strcmp(arguments[0], "which") == 0) {
            // which command
            if (argsize >= 2)
            {
                char *filename2 = arguments[1];

                char *check1 = malloc(strlen("/usr/local/bin/") + strlen(filename2) + 1);
                memcpy(check1, "/usr/local/bin/", strlen("/usr/local/bin/"));
                memcpy(check1 + strlen("/usr/local/bin/"), filename2, strlen(filename2));
                check1[strlen("/usr/local/bin/") + strlen(filename2)] = '\0';
                
                
                if (access(check1, F_OK) == 0){
                    write(1, "BuiltIn Which: ", strlen("BuiltIn Which:"));
                    write(1, check1, strlen(check1));
                    write(1, "\n", strlen("\n\0"));
                    free(check1); //free check1 string and return 
                    exit(1);
                } else {
                    // free check1 
                    free(check1);
                }

                char *check2 = malloc(strlen("/usr/bin/") + strlen(filename2)+ 1 );
                memcpy(check2, "/usr/bin/", strlen("/usr/bin/"));
                memcpy(check2 + strlen("/usr/bin/"), filename2, strlen(filename2));
                check2[strlen("/usr/bin/") + strlen(filename2)] = '\0';

                //check if program is in "/usr/bin"
                if (access(check2, F_OK) == 0){
                    write(1, "BuiltIn Which: ", strlen("BuiltIn Which:"));
                    write(1, check2, strlen(check2));
                    write(1, "\n", strlen("\n\0"));
                    free(check2);
                    
                    exit(1);
                } else {
                    free(check2);
                }
                
                char *check3 = malloc(strlen("/bin/") + strlen(filename2) + 1);
                memcpy(check3, "/bin/", strlen("/bin/"));
                memcpy(check3 + strlen("/bin/"), filename2, strlen(filename2));
                check3[strlen("/bin/") + strlen(filename2)] = '\0';

                //check if curr prog is in "/bin"
                if (access(check3, F_OK) == 0){
                    write(1, "BuiltIn Which: ", strlen("BuiltIn Which:"));
                    write(1, check3, strlen(check3));
                    write(1, "\n", strlen("\n\0"));
                    
                    exit(1);
                } else{
                    free(check3);
                }
                //printf("BuiltIn Which: %s\n", filename);
                //fail
                
                exit(0);
            }
            else
            {

                exit(0);
            }
        }

        //execute the program
        execv(filename, arguments);
      
        //child should not reach here , if it does, print error and exit
        printf("whoooops! error"); 
        exit(1);
    }

    else if (pid > 0){
        int child_status; //declare child status
        wait(&child_status); //wait for the chi;d
        
        //if you exit the child normally
        if (WIFEXITED(child_status)){ 
            //check if exit was succesful
            if (WEXITSTATUS(child_status) == 0) { 
                statusFlag = 1;  //if exit succesfully, set status flag to 1
            } else {
                // Child process exited with an error
                statusFlag = 0; //set status flag to 0 if exiting with an error
            }
            //set the status flag to 1
        }
        else if (WIFSIGNALED(child_status)){
            statusFlag = 0; //if exiting with a signal set status flag to 0;
        }
        return;
    } 
    else {
        //otherwise you failed to fork 
        perror("fork failed");
        return;
    }
}

//********************************************************************************************//

void checkBareNames(arraylist_t *argList, int argsize, char *outputFile, char *inputFile, char *cwd){

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
        execProg(check1, arguments, outputFile, inputFile, cwd, argsize);
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
        execProg(check2, arguments, outputFile, inputFile, cwd, argsize); //execute
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
        execProg(check3, arguments, outputFile, inputFile, cwd, argsize); //execute
        free(check3);
        return;
    } else{
        free(check3);
    }

}

//********************************************************************************************//


/*Process args will process the actual commands*/
int processArgs(arraylist_t *arguments){

    //set the first argument to filename
    char *filename = arguments->data[0];
    
    //set argument size
    int argsize = arguments->length;

    //check that the provided file is not null
    if (!(filename )){
        return argsize;
    } 


    /**** CONDITIONALS HERE ****/
    if (strcmp(filename, "then") == 0){
        
        //if the status flag is not 1 (previous command failed) return immedeatly
        if (statusFlag != 1) return argsize;

        //otherwise set conditionalArgs argument list
        arraylist_t *conditionalArgs = al_create(800);

        //iterate through arguments starting from after "then"
        for (int i = 1; i < arguments->length; i++){
            
            //creating argument
            char *arg = malloc(strlen(arguments->data[i]) + 1);
            
            //copy contents and push intp argument list
            strcpy(arg, arguments->data[i]);
            al_push(conditionalArgs, arg);
        }

        //call process args with new arglist
        processArgs(conditionalArgs);

        //free what was in conditionalArgs array
        for (int i = 0; i < conditionalArgs->length; i++){
            free(conditionalArgs->data[i]);
        }
        al_destroy(conditionalArgs);
        free(conditionalArgs);

        return argsize;
    
    } else if (strcmp(filename, "else") == 0){ //if you find an "else"
        
        //check status flag of previous command
        //if exited normally , return immeadeatly 
        if (statusFlag != 0) return argsize;

        //create new argument list
        arraylist_t *conditionalArgs = al_create(800);

        //copy whatever was in arguments after "else" and add to the argument list
        for (int i = 1; i < arguments->length; i++){
            char *arg = malloc(strlen(arguments->data[i]) + 1);
            
            strcpy(arg, arguments->data[i]);
            al_push(conditionalArgs, arg);
        }

        //process the new argument list
        processArgs(conditionalArgs);
        
        //free what was in conditionalArgs array
        for (int i = 0; i < conditionalArgs->length; i++){
            free(conditionalArgs->data[i]);
        }
        al_destroy(conditionalArgs);
        free(conditionalArgs);

        return argsize;
        
    }

    /**** EXITING HERE ****/
    //If first argument is exit then exit immeadeatly
    if (strcmp(filename, "exit") == 0){
        printf("Exiting...\n");
        exit(1);
    }

    /**** WILD CARD EXPANSION HERE ****/
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

    /**** CD BUILT IN HERE ****/
    char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));

        //cd here
        //printf("%d\n", argsize);
        //printf("%s\n", arguments->data[0]);
        if (argsize==2 && strcmp(arguments->data[0],"cd")==0)
        {
            if(chdir(arguments->data[1])==0)
            {
                getcwd(cwd, sizeof(cwd));
                printf("success!\nCurrent Dir is: %s\n", cwd);
                statusFlag = 1;
                return argsize;
            }
            else{
                getcwd(cwd, sizeof(cwd));
                printf("fail!\nCurrent Dir is: %s\n", cwd);
                statusFlag = 0;
                return argsize;
            }
        }
        else if (argsize!=2 && strcmp(arguments->data[0],"cd")==0)
        {
            printf("Wrong Number of Arguments for cd\n");
            statusFlag = 0;
            return argsize;
        }

    /**** PIPING OVER HERE ****/
    //create the pipeArgList, this will hold the right side of the command '|'
    // ex:    <pipeArgList> | <pipeOutputData>
    arraylist_t *pipeArgList = al_create(800);
    
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

    /**** REDIRECTING HERE ****/
    
    //redirect flag will determine if there was a redirect char, needed to determine when to free memory
    int redirectFlag = 0;

    //output and input files for the redirect
    char *outputFile = NULL;
    char *inputFile = NULL;

    arraylist_t *list = al_create(100);

    for (int i = 0; i < argsize; i++){
        if (arguments->data[i] == NULL){continue;}
        if (strcmp(arguments->data[i], ">")==0 && arguments->data[i+1] != NULL)
        {   
            //when we encounter a ">"
            redirectFlag = 1; //set redirect flag
            outputFile = malloc(strlen(arguments->data[i+1]) + 1); //allocate size for outputFile
            strcpy(outputFile, arguments->data[i+1]); //copy name of output file over
            i = i+1;
        }
        else if (strcmp(arguments->data[i], "<")==0 && arguments->data[i+1] != NULL)
        {
            //when we encounter a "<"
            redirectFlag = 1; //set redirect flag
            inputFile = malloc(strlen(arguments->data[i+1]) + 1); //allocate size for inputFile
            strcpy(inputFile, arguments->data[i+1]); //copy name of input file over
            i = i+1;
        }
        else{
            al_push(list, arguments->data[i]);
        } 
    }

    //n hold the buffer that will hold the return value of al_pop
    char *n;
    //get length of curr list
    int newLength = al_length(list);
    int k = newLength-1;
    
    //create truncArgs array which will hold what was before the redirect
    arraylist_t *truncArgs = al_create(sizeof(arguments->data[0])*(newLength+2));
    while ((n = al_pop(list)) != NULL) {
        truncArgs->data[k] = malloc(strlen(n) + 1);
        strcpy(truncArgs->data[k], n);
        truncArgs->length = truncArgs->length + 1;
        k = k-1;
    }
   
    //Null terminate truncArgs to denote end of list (need for exec)
    al_push(truncArgs, NULL); 
    
    //if we have redirecton
    if (redirectFlag == 1){
        //free the contents of arguments, we will build new argument list from truncArgs
        for(int i = 0; i < arguments->length; i++){
            free(arguments->data[i]);
        }
        al_destroy(arguments);
      
        al_destroy(list);
        free(list);

        arguments->data = truncArgs->data; //set data of arguments to truncArgs data
        arguments->length = truncArgs->length - 1; //set length
        arguments->size = truncArgs->size; //set size
        free(truncArgs);
        
    } else{
        
        //If no redirection present, free and destroy truncArgs
        al_destroy(list);
        free(list);

        for (int i = 0; i < truncArgs->length; i++){
            free(truncArgs->data[i]);
        }
        al_destroy(truncArgs);
        free(truncArgs);
    }

    



    /**** EXECTUION OF LOCAL PROGRAMS ****/
    //check if first argument contains a '/' this indicates that this will be local program
    filename = arguments->data[0];
    for (int i = 0; i < strlen(filename); i++){
        if(filename[i] == '/'){
            execProg(filename, arguments->data, outputFile, inputFile, cwd, argsize); //execute program 
            return argsize;
        }
    }

    /**** SHELL BUILT INS HERE ****/
    

    /**** BARENAME CHECK HERE ****/
    checkBareNames(arguments, argsize, outputFile, inputFile, cwd);

    /**** free whatever is still not freed ****/
    if (inputFile != NULL) free(inputFile);
    if (outputFile != NULL) free(outputFile);
    for (int i = 0; i < pipeArgList->length; i++){
        free(pipeArgList->data[i]);
    }

    al_destroy(pipeArgList);
    free(pipeArgList);

    return argsize;
}

//********************************************************************************************//


//acceptArgs will iterate through the buffer and collect a sequence of tokens (arguments)
void acceptArgs(char *buf, int bytes){
    //create and initialize argument arraylist
    arraylist_t *argList = al_create(800);
    if (buf[0] == 0 ) return;
    
    //ensure the array list was created properly
    if (argList == NULL){
        exit(1);
    }
    
    int start = 0;  
    for (int i = 0; i < bytes; i++){
        //if you encounter a space, this marks the end of a single argument
        if (i > 0 && isspace(buf[i]) && !(isspace(buf[i - 1]))){ 
            
            char *arg = malloc(i - start + 1); //allocate space for the current argument
           
            if (*arg == 32 ){
                start = i + 1;
                continue;
            } 
            
            memcpy(arg, buf + start, i - start);
            
            arg[i -  start] = '\0';

            //token flag will check if we encounter a redirect or pipe without spaces around it
            //this is to ensure that we dont double push into arg array
            int tokenFlag = 0; 
           
            for (int i = 0; i < strlen(arg); i++){
                //if we get any of > < or |
                if (arg[i] == '>' || arg[i] == '<' || arg[i] =='|'){
                    //just checking that we are in bounds
                    if (i - 1 >= 0 && i + 1 <= strlen(arg)){
                        tokenFlag = 1;
                        
                        // command would be: <prev> <comm> <after> where <comm> is any of < > or |
                        
                        //prev holds whatever is before a redirect or pipe
                        char *prev = malloc(i + 1); //space allocation
                        memcpy(prev, arg, i); //copy content
                        prev[i] = '\0'; //null terminate
                        
                        //after would hold whatever is after a redirect or pipe
                        char *after = malloc(strlen(arg) - i + 1); //space allocation
                        memcpy(after, arg + i + 1, strlen(arg) - i); //copy content
                        after[strlen(arg) - i ] = '\0'; //null terminate
                        
                        //comm will hold either a ">" or "<" or "|"
                        char *comm = malloc(2); //space allocation (2 because one byte for char, one for null term)
                        memcpy(comm, arg + i, 1); //copy over
                        comm[1] = '\0'; //null terminate
                        
                        //push into argList in this order 
                        al_push(argList, prev);
                        al_push(argList, comm);
                        al_push(argList, after);
                        
                        break; //break 
                    
                    } else {
                        break; //break 
                    }          
                }
            }
            
            //if token flag is 0 then no redirection or pipes present, thus push arg normally
            if (tokenFlag == 0){
                al_push(argList, arg);
            } else{
                //otherwise if there was a redirect or pipe then free arg (we didnt need it since we broke it up)
                free(arg);
            }
            start = i + 1; //update start 
        }
    }

    //pass the arraylist to processArgs
    processArgs(argList);
    
    //free the argument list 
    for (int i = 0; i < argList->length; i++){
        free(argList->data[i]);
    }
    al_destroy(argList);
    free(argList); 
    
    return;
}

//********************************************************************************************//


int main(int argc, char **argv){
    
    //initialize the previous process exit flag
    statusFlag = 1;
    
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
                if(buf[i] == '\n' && i > 0){ //we have reached the end of a command
                    command_length = i - start;  //record the length of the command
                    input = malloc(command_length + 1); //allocate space to hold command
                    memcpy(input, buf + start, command_length); //copy command into input
                    input[command_length] = '\0'; //null terminate the input 
                   
                    acceptArgs(input, command_length);
                    free(input);
                    
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