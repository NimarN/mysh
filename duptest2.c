#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    int fd[2];
    pipe(fd);

    if (fork() == 0){
        //child 
        
        dup2(fd[1], STDOUT_FILENO);
        execl("duptest1", "empty", NULL);
    }
    close(fd[1]);
    int bytes;
    char *buf = malloc(100);
    while((bytes = read(fd[0], buf, 100)) > 0){
        //do whatever
        char *whatever = malloc(bytes + 1);
        memmove(whatever, buf, bytes);
        whatever[bytes] = '\0';
        printf("what i found: %s\n", whatever);
        free(whatever);
    }
    
    free(buf);
    return 1;

}