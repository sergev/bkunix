/*
 * Read kernel table of mounted filesystems.
 */
#include <ansidecl.h>

int openmtab PARAMS((void));
int readmtab PARAMS((int, int*, int*, int*, int*));
void resetmtab PARAMS((int));
