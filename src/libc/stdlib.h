/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#ifndef _STDLIB_H_
#define _STDLIB_H_ 1

#include <ansidecl.h>

void exit PARAMS((int));
void qsort PARAMS((char*, int, int, int (*)()));
int atoi PARAMS((char*));

char *malloc PARAMS((unsigned));
void free PARAMS((char*));
char *realloc PARAMS((char*, unsigned));
char *calloc PARAMS((unsigned, unsigned));

#endif /* _STDLIB_H_ */
