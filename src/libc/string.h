/*
 * String handling functions.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _STRING_H_
#define _STRING_H_ 1

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

#endif /* _STRING_H_ */
