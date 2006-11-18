/*
 * Return file offset.
 * Coordinates with buffering.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <unistd.h>

long
ftell(iop)
	FILE *iop;
{
	long tres;
	register adjust;

	if (iop->_cnt < 0)
		iop->_cnt = 0;
	if (iop->_flag&_IOREAD)
		adjust = - iop->_cnt;
	else if(iop->_flag&(_IOWRT|_IORW)) {
		adjust = 0;
		if (iop->_flag&_IOWRT && iop->_base && (iop->_flag&_IONBF)==0)
			adjust = iop->_ptr - iop->_base;
	} else
		return -1;

	tres = lseek(fileno(iop), 0L, 1);
	if (tres<0)
		return(tres);

	tres += adjust;
	return tres;
}
