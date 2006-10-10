/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
# include "pass1.h"

/*	communication between lexical routines	*/

char	ftitle[100];   		/* title of the file */
#ifndef	LINT
char	ititle[100];   		/* title of initial file */
#endif
int	lineno;		/* line number of the input file */

CONSZ lastcon;  /* the last constant read by the lexical analyzer */
float fcon;   /* the last float read by the lexical analyzer */
double dcon;   /* the last double read by the lexical analyzer */


/*	symbol table maintainence */

struct symtab stab[SYMTSZ+1];  /* one extra slot for scratch */

int	curftn;  /* "current" function */
int	ftnno;  /* "current" function number */

int	curclass,	  /* current storage class */
	instruct,	/* "in structure" flag */
	stwart,		/* for accessing names which are structure members or names */
	blevel,		/* block level: 0 for extern, 1 for ftn args, >=2 inside function */
	curdim;		/* current offset into the dimension table */

OFFSZ	dimtab[ DIMTABSZ ];	/* same comments as below.  bit addressing
				 * everything forces this to be large.
				*/

OFFSZ	paramstk[ PARAMSZ ];  /* used in definition of function parameters */
			      /* ordinarily 'int' would be enough, but the
			       * "bit address" in 'strucoff' (for structures
			       * over 4kb) needs a 'long' *sigh*
			      */
int	paramno;	  /* the number of parameters */
OFFSZ	autooff,	/* the next unused automatic offset */
	argoff,		/* the next unused argument offset */
	strucoff;	/*  the next structure offset position */
int	regvar;		/* the next free register for register variables */
int	minrvar;	/* the smallest that regvar gets witing a function */
OFFSZ	inoff;		/* offset of external element being initialized */
int	brkflag = 0;	/* complain about break statements not reached */

struct sw swtab[SWITSZ];  /* table for cases within a switch */
struct sw *swp;  /* pointer to next free entry in swtab */
int swx;  /* index of beginning of cases for current switch */

/* debugging flag */
int xdebug = 0;

int strflg;  /* if on, strings are to be treated as lists */

int reached;	/* true if statement can be reached... */

int idname;	/* tunnel to buildtree for name id's */


NODE node[TREESZ];

int cflag = 0;  /* do we check for funny casts */
int hflag = 0;  /* do we check for various heuristics which may indicate errors */
int pflag = 0;  /* do we check for portable constructions */

int brklab;
int contlab;
int flostat;
int retlab = NOLAB;
int retstat;

/* save array for break, continue labels, and flostat */

int asavbc[BCSZ];
int *psavbc = asavbc ;

# ifndef BUG1
static char *
ccnames[] = { /* names of storage classes */
	"SNULL",
	"AUTO",
	"EXTERN",
	"STATIC",
	"REGISTER",
	"EXTDEF",
	"LABEL",
	"ULABEL",
	"MOS",
	"PARAM",
	"STNAME",
	"MOU",
	"UNAME",
	"TYPEDEF",
	"FORTRAN",
	"ENAME",
	"MOE",
	"UFORTRAN",
	"USTATIC",
	};

char * scnames( c ) register c; {
	/* return the name for storage class c */
	static char buf[12];
	if( c&FIELD ){
		sprintf( buf, "FIELD[%d]", c&FLDSIZ );
		return( buf );
		}
	return( ccnames[c] );
	}
# endif
