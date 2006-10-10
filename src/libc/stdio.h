/*
 * Standard input/output library functions.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _STDIO_H_
#define _STDIO_H_ 1

#include <ansidecl.h>

int printf PARAMS(());
int vprintf PARAMS((char*, char*));

#endif /* _STDIO_H_ */
