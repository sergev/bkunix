/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

extern FILE *_endopen();

FILE *
freopen(file, mode, iop)
	char *file, *mode;
	register FILE *iop;
{
	fclose(iop);
	return _endopen(file, mode, iop);
}
