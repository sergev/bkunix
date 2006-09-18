/*
	Header file for first part of as - PDP/11 Assember
*/

/*
	Symbol Table
*/
struct symtab {
	char name[8];
	struct value v;
};

extern struct symtab *hshtab[HSHSIZ];	/* Hash Table			  */
extern struct symtab symtab[SYMBOLS];	/* Permanent Symbol Table */
extern struct symtab usymtab[USERSYMBOLS];	/* Normal Symbol Table*/

#define dotrel	(symtab[0].v.type.i)	/* Relocation factor for .*/
#define dot		(symtab[0].v.val.u)		/* Assembler Program ctr  */
#define dotdot	(symtab[1].v.val.u)		/* Origin location counter*/

extern struct symtab *symend;			/* Points past last entry */
extern char symbol[8];					/* Symbol assembly line	  */
extern unsigned savdot[3];				/* Saved . for txt/dat/bss*/

/*
	Forward branch table
*/
extern char curfbr[10];					/* Relocation for $0-$9	  */
extern int  curfb[10];					/* . for $0 - $9		  */
extern struct fb_tab nxtfb;				/* next forward branch	  */

/*
	Files
*/
extern FILE * fin;							/* Input file			  */
extern int fbfil;						/* Forward branch file	  */
extern int pof;							/* Token output file	  */
extern char fileflg;					/* (unsued) file counter  */

/*
	Flags
*/
extern int errflg;						/* error counter		  */
extern int ifflg;						/* .if nesting counter	  */
extern char globflag;					/* true if 2 undef=>gbl   */
extern int eos_flag;					/* true if at end of stmt */

/*
	Scanning/Parsing variables
*/
extern union token tok;					/* Scanned token		  */
extern int line;						/* Line # in source file  */
extern int savop;						/* saved token			  */
extern char ch;							/* saved character		  */
extern int numval;						/* number / string length */
extern int num_rtn;						/* return value from number*/

/*
	File name / arguments
*/
extern int nargs;						/* Number of files left	  */
extern char **curarg;					/* Current input file nam */

/*
	Tables
*/
extern char chartab[];					/* Scanner character table*/
extern char schar[];					/* escapes in strings	  */
extern char esctab[];					/* escapes in free text	  */

int address();
void add_symbol();
void aerror();
void aexit();
void aputw();
void assem();
int checkeos();
void checkrp();
void checkreg();
int close();
int creat();
unsigned fbcheck();
int f_create();
void filerr();
void hash_enter();
int isdigit();
void opline();
char number();
unsigned char rch();
void readop();
void rname();
void setup();
int write();
