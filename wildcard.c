#include <stdlib.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <errno.h>
#include <linux/limits.h>

//struct dirent {
//    ino_t          d_ino;       /* inode number */
//    off_t          d_off;       /* offset to the next dirent */
//    unsigned short d_reclen;    /* length of this record */
//    unsigned char  d_type;      /* type of file; not supported. DT_DIR for directory. DT_REG for regular file.
//                                   by all file system types */
//    char           d_name[256]; /* filename */
//};

//struct DIR {
//    struct dirent ent;
//    struct _WDIR *wdirp;
//};

void printWildcards(DIR *dirPtr, char *pathname)
{
    struct dirent *direntPtr;

    //Loops through a directories entries.
    while ((direntPtr = readdir(dirPtr))!=NULL)
    {
        char *direntName = direntPtr->d_name;

        printf("%s\n", direntName);
    }
}

int main(int argc, char **argv){

    //Eventually will make main a function that takes an argument with wildcard as parameter.
    char *argWithWildcard = "foo*bar";
    
    //stores current working directory in char *cwd
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }

    DIR *dirPtr;
    char* path = cwd;
    strcat(path, "/");

    dirPtr = opendir(path);
    printWildcards(dirPtr, argWithWildcard);
    closedir(dirPtr);

    return 1;
}