/*
 * AS - PDP/11 Assembler part 2
 *
 * Header file
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include "as.h"
#include "as2.h"

/*
	Symbol Table (without the character part)
*/
struct value symtab[SYMBOLS];			/* Permanent symbol Table */
struct value usymtab[USERSYMBOLS];		/* User symbol table	*/


/*
	Forward branch table
*/
struct fb_tab fbtab[1024];			/* forward branch table	  */
struct fb_tab *fbbufp;				/* current loc in fbtab	  */
struct fb_tab *curfb[20];			/* entries for back refs  */
struct fb_tab **nxtfb = curfb + 10;		/* entries for forward ref*/
struct fb_tab *endtable;			/* end of branch table */

/*
	a.out header structure
*/
struct hdr hdr = {0407, { 0,0,0 }, 0,0,0,0};


/*
	File seek locations
*/
unsigned aseek[3];					/* seek for "absolute"	  */
unsigned relseek[3];					/* seek for reloc. data	  */
unsigned symseek;					/* seek for symbol table  */

unsigned *tseekp = &txtseek;				/* ptr to abs seek entry  */
unsigned *rseekp = &trelseek;			/* ptr to reloc. seek ent.*/

/*
	br/jmp table
*/
char brtab[BRLEN/8];				/* jbr table 1=can't	  */
int brtabp = 0;					/* current entry number	  */
int brdelt = 0;					/* current displacement	  */


struct out_buf txtp;				/* abs data buffer		  */
struct out_buf relp;				/* relocation data buffer */

/*
	Other variables, etc.
*/

int savdot[3] = {0,0,0};			/* saved . for txt/dat/bss*/

unsigned adrbuf[6];				/* for constructint addrs */
unsigned *padrb;				/* pointer into adrbuf	  */

char argb[44];					/* input source file name */
int defund = 0;					/* true if undef auto def */
int txtfil;					/* input token file		  */
int symf;					/* symbol table file	  */
int fbfil;					/* forward branch file	  */
int fin;					/* generic input file	  */
int fout;					/* output file			  */
int passno = 0;					/* pass of this phase	  */
int outmod = 0777;				/* 777 for ok 666 for err */
int line = 0;					/* line in source file	  */
int savop = 0;					/* token "stack"		  */
int xsymbol = 0;				/* symbol offset for ext. */
int swapf = 0;					/* true to swap operands  */
int rlimit = 0;					/* displacement limit	  */
int datbase = 0;				/* base addr for .data	  */
int bssbase = 0;				/* base address for .bss  */
int numval = 0;					/* value of numeric token */
int maxtyp = 0;					/* max type in combination*/

union token tok;				/* tokens read from pass 1*/

/*
	Relocation combination tables for various operators
*/

/*
	Table to combine r1 + r2
*/
int reltp2[] = {	0,  0,  0,  0,  0,  0,
			0, -1,  2,  3,  4,040,
			0,  2, -2, -2, -2, -2,
			0,  3, -2, -2, -2, -2,
			0,  4, -2, -2, -2, -2,
			0,040, -2, -2, -2, -2 };

/*
	Table to combine r1 - r2
*/
int reltm2[] = {	0,  0,  0,  0,  0,  0,
			0, -1,  2,  3,  4,040,
			0, -2,  1, -2, -2, -2,
			0, -2, -2,  1, -2, -2,
			0, -2, -2, -2,  1, -2,
			0, -2, -2, -2, -2, -2 };

/*
	Table to combine r1 OP r2  (OP not + or -)
*/
int relte2[] = {	0,  0,  0,  0,  0,  0,
			0, -1, -2, -2, -2, -2,
			0, -2, -2, -2, -2, -2,
			0, -2, -2, -2, -2, -2,
			0, -2, -2, -2, -2, -2,
			0, -2, -2, -2, -2, -2 };
