/*
	as - PDP/11 assembler part 1 global data initialization
*/

#include "as.h"
#include "as1.h"

/*
	Symbol table
*/
struct symtab *hshtab[HSHSIZ];		/* Hash Table			  */
struct symtab symtab[SYMBOLS];		/* Permanent Symbol Table */
struct symtab usymtab[USERSYMBOLS];	/* Normal Symbol Table	  */

struct symtab *symend = symtab + SYMBOLS;	/* Points past last entry */
char symbol[8];						/* Symbol assembly line	  */
unsigned savdot[3] = {0,0,0};		/* Saved . for txt/dat/bss*/

/*
	Forward branch table
*/
char curfbr[10] = {0,0,0,0,0,0,0,0,0,0};  /* Relocation		  */
int curfb[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};  /* Value	  */
struct fb_tab nxtfb = {0,0,0};		/* Entry descriptor		  */

/*
	Files
*/
FILE * fin;							/* Input file			  */
int fbfil;							/* Forward branch output  */
int pof;							/* Token output file	  */
char fileflg;						/* (unused) file counter  */

/*
	Flags
*/
int errflg = 0;						/* Error counter		  */
int ifflg = 0;						/* Nested .if counter	  */
char globflag = 0;					/* true if pass 2 und->gbl*/
int eos_flag = 0;					/* true if at end of stmt */

/*
	Scanning/Parsing variables
*/
union token tok;					/* scanned token		  */
int line = 0;						/* Line in source file	  */
int savop = 0;						/* token lookahead		  */
char ch = 0;						/* character lookahead	  */
int numval = 0;						/* number / string length */
int num_rtn;						/* return val from number */


/*
	File name / arguments
*/
int nargs;							/* number of files to go  */
char **curarg;						/* current file name	  */

/*
	Tables
*/


/*
	Scanner character table, values match preprocessor vars CHAR*
	Note that values are in octal.  Signed values have the
	positive equivalents defined.
*/

char chartab[] = {
	-014,-014,-014,-014,-002,-014,-014,-014,
	-014,-022, -02,-014,-014,-022,-014,-014,
	-014,-014,-014,-014,-014,-014,-014,-014,
	-014,-014,-014,-014,-014,-014,-014,-014,
	-022,-020,-016,-014,-020,-020,-020,-012,
	-020,-020,-020,-020,-020,-020,0056,-006,
	0060,0061,0062,0063,0064,0065,0066,0067,
	0070,0071,-020,-002,-000,-020,-014,-014,
	-014,0101,0102,0103,0104,0105,0106,0107,
	0110,0111,0112,0113,0114,0115,0116,0117,
	0120,0121,0122,0123,0124,0125,0126,0127,
	0130,0131,0132,-020,-024,-020,-020,0137,
	-014,0141,0142,0143,0144,0145,0146,0147,
	0150,0151,0152,0153,0154,0155,0156,0157,
	0160,0161,0162,0163,0164,0165,0166,0167,
	0170,0171,0172,-014,-026,-014,0176,-014 };

/*
	Table of back-slash escape sequences in strings
*/

char schar[] = {'n',012,'t',011,'e',TOKEOF,'0',0,
			    'r',015,'a',006,'p',033,0,-1};


/*
	Table of special characters outside of strings
*/

char esctab[] = {'/','/','<',TOKLSH,'>',TOKRSH,'%',TOKVBAR,0,0};
