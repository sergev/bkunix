/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

void
rewind(iop)
	register struct _iobuf *iop;
{
	fflush(iop);
	lseek(fileno(iop), 0L, 0);
	iop->_cnt = 0;
	iop->_ptr = iop->_base;
	iop->_flag &= ~(_IOERR|_IOEOF);
	if (iop->_flag & _IORW)
		iop->_flag &= ~(_IOREAD|_IOWRT);
}
