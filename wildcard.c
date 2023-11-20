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


//returns 1 if directory entry prefix matches wildcard prefix
int matchesPrefix (char *dirent, char *prefix)
{
    //int suffixLength = strlen(suffix);
    //int lookAfterIndex = strlen(dirent)-strlen(prefix);

    char *start = &dirent[0];
    char *end = &dirent[strlen(prefix)];
    char *substring = (char *)calloc(1, end - start + 1);
    memcpy(substring, start, end - start);
    //printf("Suffix: %s\n", substring);

    if(strcmp(substring, prefix)==0)
    {
        free(substring);
        return 1;
    }
    free(substring);
    return 0;
}
int matchesSuffix (char *dirent, char *suffix)
{
    //int suffixLength = strlen(suffix);
    int lookAfterIndex = strlen(dirent)-strlen(suffix);

    char *start = &dirent[lookAfterIndex];
    char *end = &dirent[strlen(dirent)];
    char *substring = (char *)calloc(1, end - start + 1);
    memcpy(substring, start, end - start);
    //printf("Suffix: %s\n", substring);

    if(strcmp(substring, suffix)==0)
    {
        free(substring);
        return 1;
    }
    free(substring);
    return 0;
}

void printWildcards(DIR *dirPtr, char *pathname, char *prefix, char *suffix)
{
    struct dirent *direntPtr;
    int minLength = strlen(prefix) + strlen(suffix);
    printf("Minlength: %d\n", minLength);
    //Loops through a directories entries.
    while ((direntPtr = readdir(dirPtr))!=NULL)
    {
        char *direntName = direntPtr->d_name;
        if (strlen(direntName) >= minLength)
        {
            matchesSuffix(direntName, suffix);
            if (matchesPrefix(direntName, prefix)==1 && matchesSuffix(direntName, suffix)==1)
            {
                printf("%s\n", direntName);
            }
            //uncomment to print all directory entries above the min length
            //printf("%s\n", direntName);            
        }
        //uncomment to print all directory entries
        //printf("%s\n", direntName);
    }
}

char *stringBeforeWildCard(char *argWithWilcard, int wildcardIndex)
{
    char *start = &argWithWilcard[0];
    char *end = &argWithWilcard[wildcardIndex];
    char *substring = (char *)calloc(1, end - start + 1);
    memcpy(substring, start, end - start);


    return substring;
}
char *stringAfterWildCard(char *argWithWilcard, int wildcardIndex)
{
    char *start = &argWithWilcard[wildcardIndex+1];
    char *end = &argWithWilcard[strlen(argWithWilcard)];
    char *substring = (char *)calloc(1, end - start + 1);
    memcpy(substring, start, end - start);

    return substring;
}


//Only call this function once we confirm the parameter has exactly 1 wildcard.
int main(int argc, char **argv){

    //Eventually will make main a function that takes an argument with wildcard as parameter.
    //Change line below to test with different wildcard locations.
    char *argWithWildcard = "foo*bar.txt";
    
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

    //testing 
    char *wildcardLocation = strchr(argWithWildcard, '*');
    //printf("Wildcard Location: %s\n", wildcardLocation);

    //wildcard index for foo*bar returns 3, meaning string is 0 indexed.
    int wildcardIndex = wildcardLocation - argWithWildcard;
    //printf("Wildcard Index: %d\n", wildcardIndex);

    char *prefix = stringBeforeWildCard(argWithWildcard, wildcardIndex);
    printf("Prefix: %s\n", prefix);

    char *suffix = stringAfterWildCard(argWithWildcard, wildcardIndex);
    printf("Sufix: %s\n", suffix);

    dirPtr = opendir(path);
    printWildcards(dirPtr, argWithWildcard, prefix, suffix);
    closedir(dirPtr);
    free(prefix);
    free(suffix);

    return 1;
}