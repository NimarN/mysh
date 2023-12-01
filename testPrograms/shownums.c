#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h> 
#include <string.h> 

#define BUFSIZE 200

int main(int argc, char ** argv){
    char buf[BUFSIZE];
    int fd = STDIN_FILENO;
    int bytes;
    bytes = read(fd, buf, BUFSIZE);
    buf[bytes] = '\0';
    printf("what I have received from the first Program : %s\n", buf);
}