#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



int main(int argc, char **argv){
    
    write(1, argv[1], strlen(argv[1]));
}