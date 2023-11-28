#include <dirent.h>
#include "arraylist.h"


int matchesPrefix (char *dirent, char *prefix);
int matchesSuffix (char *dirent, char *suffix);
int printWildcards(DIR *dirPtr, char *pathname, char *prefix, char *suffix, arraylist_t *argArray, int position, int argsize);
char *stringBeforeWildCard(char *argWithWilcard, int wildcardIndex);
int startExpansion(char *argWithWildcard, arraylist_t argArray, char **argumentList, int position, int argsize);
