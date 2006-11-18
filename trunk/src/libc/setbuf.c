/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>

void
setbuf(iop, buf)
	register struct _iobuf *iop;
	char *buf;
{
	if (iop->_base != NULL && (iop->_flag & _IOMYBUF))
		free(iop->_base);
	iop->_flag &= ~(_IOMYBUF | _IONBF);
	if ((iop->_base = buf) == NULL) {
		iop->_flag |= _IONBF;
		iop->_cnt = 0;
	} else {
		iop->_ptr = iop->_base;
		iop->_cnt = BUFSIZ;
	}
}
