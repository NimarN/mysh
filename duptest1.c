#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

int main(){
    char *message = "Hello World!\n";
    write(1, message, strlen(message));
    return 1;
}