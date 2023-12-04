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
#include "arraylist.h"

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

//Only touching this method
int printWildcards(DIR *dirPtr, char *pathname, char *prefix, char *suffix, arraylist_t *argArray, int position, int argsize, int pathProvidedFlag, char *path)
{
    char **argumentList = argArray-> data;
    //printf("AL Before: ");
    /*for (int i = 0; i < argArray->length; i++){
        printf("%s ", argumentList[i]);
    }*/
    //printf("\n");
    struct dirent *direntPtr;
    int minLength = strlen(prefix) + strlen(suffix);
    int firstMatch = 0;
    //int argLocation = 0;
    //printf("Minlength: %d\n", minLength);
    //Loops through a directories entries.
    char *match;
    while ((direntPtr = readdir(dirPtr))!=NULL)
    {
        char *direntName = direntPtr->d_name;
        if (strlen(direntName) >= minLength)
        {
            matchesSuffix(direntName, suffix);
            if (matchesPrefix(direntName, prefix)==1 && matchesSuffix(direntName, suffix)==1)
            {
                if (firstMatch == 0)
                {
                    if (pathProvidedFlag == 1){

                        match = malloc(strlen(direntName) + strlen(path) + 1);
                        memcpy(match, path, strlen(path));
                        memcpy(match + strlen(path), direntName, strlen(direntName));
                        match[strlen(direntName) + strlen(path)] = '\0';
                        free(argumentList[position]);
                        argumentList[position] = match;
                        firstMatch = 1;
                        position = position+1;
                    }
                    else{
                        match = malloc(strlen(direntName)+1);
                        strcpy(match, direntName);
                        free(argumentList[position]);
                        argumentList[position] = match;
                        firstMatch = 1;
                        position = position+1;
                    }  
                }
                else
                {
                    argsize = argsize+1;
                    argArray->length = (argArray->length) + 1;
                    position = position+1;

                    //What ever is malloced from here leaks...
                    if (pathProvidedFlag){
                        match = malloc(strlen(direntName) + strlen(path) + 1);
                        memcpy(match, path, strlen(path));
                        memcpy(match + strlen(path), direntName, strlen(direntName));
                        match[strlen(direntName) + strlen(path)] = '\0';
                    }
                    else {
                        match = malloc(strlen(direntName)+1);
                        strcpy(match, direntName);
                    }
                    

                    for (int i = argsize - 1; i>=position; i--)
                    {
                        //free(argumentList[i]);
                        //printf("curr: %s \n", argumentList[i]);
                        
                        //printf("prev: %s \n", argumentList[i - 1]);
                        argumentList[i] = argumentList[i-1];
                    }
                    //free(argumentList[position]);
                    argumentList[position-1] = match;
                }
            }
            //uncomment to print all directory entries above the min length
            //printf("%s\n", direntName);            
        }
        //uncomment to print all directory entries
        //printf("%s\n", direntName);
    }
    

    return argsize;
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
int startExpansion(char *wildCardArg, arraylist_t argArray, char **argumentList, int position, int argsize)
{

    //Eventually will make main a function that takes an argument with wildcard as parameter.
    //Change line below to test with different wildcard locations.
    
    //stores current working directory in char *cwd
    char cwd[PATH_MAX];
    char *argWithWildcard;

    char *providedPath = NULL;
    int pathProvidedFlag = 0;
    int charsAfterSlash = 0;
    for (int i = 0; i < strlen(wildCardArg); i++){

        if(wildCardArg[i] == '/'){
            pathProvidedFlag = 1;
            charsAfterSlash = 0;
        } 
        else if (wildCardArg[i] == '*' && pathProvidedFlag == 1){
            providedPath = malloc(i - charsAfterSlash + 1);
            memcpy(providedPath, wildCardArg, i - charsAfterSlash);
            providedPath[i - charsAfterSlash] = '\0';
            argWithWildcard = wildCardArg + i - (charsAfterSlash);
            //printf("arg with wildcard: %s\n", argWithWildcard);
            //printf("Provided Path: %s\n", providedPath);
            break;
        }
        else {
            charsAfterSlash++;
        }
    }

    DIR *dirPtr;
    char* path;
    
    if (providedPath != NULL){
        path = providedPath;
        //printf("path provided !\n");
    }
    else if (getcwd(cwd, sizeof(cwd)) != NULL) {
        path = cwd;
        strcat(path, "/");
        argWithWildcard = wildCardArg;
        //printf("Current working dir: %s\n", cwd);
    } else {
        perror("getcwd() error");
        //return;
    } 

    

    //testing 
    char *wildcardLocation = strchr(argWithWildcard, '*');
    //printf("Wildcard Location: %s\n", wildcardLocation);

    //wildcard index for foo*bar returns 3, meaning string is 0 indexed.
    int wildcardIndex = wildcardLocation - argWithWildcard;
    //printf("Wildcard Index: %d\n", wildcardIndex);

    char *prefix = stringBeforeWildCard(argWithWildcard, wildcardIndex);
    //printf("Prefix: %s\n", prefix);

    char *suffix = stringAfterWildCard(argWithWildcard, wildcardIndex);
    //printf("Sufix: %s\n", suffix);

    dirPtr = opendir(path);
    argsize = printWildcards(dirPtr, argWithWildcard, prefix, suffix, &argArray, position, argsize, pathProvidedFlag, path);
    closedir(dirPtr);
    free(providedPath);
    free(prefix);
    free(suffix);

    return argsize;
} 