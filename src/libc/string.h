#include <ansidecl.h>

int strlen PARAMS((char*));
int strcmp PARAMS((char*, char*));
int strncmp PARAMS((char*, char*, int));

char *strcat PARAMS((char*, char*));
char *strcpy PARAMS((char*, char*));
char *strncat PARAMS((char*, char*, int));
char *strncpy PARAMS((char*, char*, int));
char *strchr PARAMS((char*, int));
char *strrchr PARAMS((char*, int));

int memcmp PARAMS((char*, char*, int));
char *memccpy PARAMS((char*, char*, int, int));
char *memchr PARAMS((char*, int, int));
char *memcpy PARAMS((char*, char*, int));
char *memset PARAMS((char*, int, int));
