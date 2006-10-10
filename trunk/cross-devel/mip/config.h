/*
 * Compiler configuration definitions.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#ifndef _CONFIG_
#define	_CONFIG_

/*
 * These flags control global compiler operation.
 */
#define	BUFSTDERR	1		/* buffer output to stderr */
#define STDPRTREE	1		/* means include prtree */
#define NESTCALLS	1		/* disallow two concurrent store()'s */
#define	FLEXNAMES	1		/* arbitrary length identifiers */
#ifdef FORT
#define	NOMAIN		1		/* use f1 main routine */
#endif

/*
 * Table sizes.
 */
#ifndef	FORT
#define	TREESZ		270
#else
#define TREESZ		1000		/* parse tree table size */
#endif
#define BCSZ		100		/* break/continue table size */
#define SYMTSZ		915		/* symbol table size */
#define DIMTABSZ 	1170		/* dimension/size table size */
#define PARAMSZ		130		/* parameter stack size */
#define SWITSZ		210		/* switch table size */
#define	DELAYS		20		/* delayed evaluation table size */
#define NRECUR		(10*TREESZ)	/* maximum eval recursion depth */
#define	MAXSCOPES	(SYMTSZ/30)	/* maximum active scopes */

/* in case anyone still uses fixed length names */
#ifndef FLEXNAMES
#define	NCHNAM		32		/* significant chars of identifier */
#endif

#endif /* _CONFIG_ */
