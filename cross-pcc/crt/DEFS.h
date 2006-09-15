/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)DEFS.h	1.2 (2.11BSD GTE) 12/24/92
 */

#ifndef _DEFS_
#define	_DEFS_
/*
 * Machine language assist.  Uses the C preprocessor to produce suitable code
 * for various 11's.
 */
#define	ENTRY(x)	.globl _##x; _##x:;
#define	ASENTRY(x)	.globl x; x:;

#endif /* _DEFS_ */
