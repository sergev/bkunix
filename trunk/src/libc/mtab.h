/*
 * Read kernel table of mounted filesystems.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _MTAB_H_
#define	_MTAB_H_

#include <ansidecl.h>

int openmtab PARAMS((void));
int readmtab PARAMS((int, int*, int*, int*, int*));
void resetmtab PARAMS((int));

#endif /* !_MTAB_H_ */
