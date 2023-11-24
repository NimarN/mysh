#include <dirent.h>

int matchesPrefix (char *dirent, char *prefix);
int matchesSuffix (char *dirent, char *suffix);
char **printWildcards(DIR *dirPtr, char *pathname, char *prefix, char *suffix, char **argumentList, int position, int argsize);
char *stringBeforeWildCard(char *argWithWilcard, int wildcardIndex);
char **startExpansion(char *argWithWildcard, char **argumentList, int position, int argsize);
