/*
 * This file includes the definitions for open()
 * described by POSIX for <fcntl.h>.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _FCNTL_H_
#define	_FCNTL_H_

/*
 * File status flags: these are used by open(2).
 */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#endif /* !_FCNTL_H_ */
