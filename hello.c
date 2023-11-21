#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>


int main(int argc, char **argv){
    printf("Hello! these are the arguments i have recieved:\n");
    for (int i = 1; i < argc; i++){
        printf("%s\n", argv[i]);
    }
    
    return 1;
}