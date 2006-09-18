/*
	AS - PDP/11 Assembler part 2

	Header file
*/

#define DEBUG 0

/*
	Symbol Table (without the character part)
*/
extern struct value symtab[SYMBOLS];	/* Permanent symbol Table */
extern struct value usymtab[USERSYMBOLS];  /* User symbol table	*/
extern struct value *endtable;			/* End of symbol tables */

#define dotrel	(symtab[0].type.u)		/* Relocation factor for .*/
#define dot	(symtab[0].val.u)		/* Assembler PC			  */
#define dotdot	(symtab[1].val.u)		/* Origin location counter*/

/*
	Forward branch table
*/
extern struct fb_tab fbtab[1024];		/* forward branch table	  */
extern struct fb_tab *fbbufp;			/* current loc in fbtab	  */
extern struct fb_tab *curfb[10];		/* entries for back refs  */
extern struct fb_tab *nxtfb[10];		/* entries for forward ref*/

/*
	a.out header structure
*/
extern struct hdr {
	int txtmagic;						/* magic number (br)	  */
	unsigned atxtsiz[3];				/* segment sizes		  */
	unsigned symsiz;					/* size of symbol table	  */
	unsigned stksiz;					/* entry location (0)	  */
	unsigned exorig;					/* unused				  */
	unsigned unused;					/* relocation supressed	  */
} hdr;

#define txtsiz	(hdr.atxtsiz[0])
#define datsiz	(hdr.atxtsiz[1])
#define bsssiz	(hdr.atxtsiz[2])

/*
	File seek locations
*/
extern int aseek[3];					/* seek for "absolute"	  */
extern int relseek[3];					/* seek for reloc. data	  */
extern int symseek;						/* seek for symbol table  */

#define txtseek		(aseek[0])			/* seek for txt abs data  */
#define datseek		(aseek[1])			/* seek for data abs data */
#define trelseek	(relseek[0])		/* seek for txt reloc.	  */
#define drelseek	(relseek[1])		/* seek for data reloc.	  */

extern int *tseekp;						/* ptr to abs seek entry  */
extern int *rseekp;						/* ptr to reloc. seek ent.*/

/*
	br/jmp table
*/
#define BRLEN	1024					/* max number of jbr/jcc  */
extern char brtab[];					/* jbr table 1=can't	  */
extern int brtabp;						/* current entry number	  */
extern int brdelt;						/* current displacement	  */

/*
	output buffer structure
*/
struct out_buf {
	int *slot;							/* current word in buffer */
	int *max;							/* &(end of buffer)		  */
	int seek;							/* file seek location	  */
	int buf[256];						/* data buffer			  */
};

extern struct out_buf txtp;				/* abs data buffer		  */
extern struct out_buf relp;				/* relocation data buffer */

/*
	Other variables, etc.
*/

extern int savdot[3];					/* saved . for txt/dat/bss*/

extern unsigned adrbuf[6];				/* for constructint addrs */
extern unsigned *padrb;					/* pointer into adrbuf	  */

extern char argb[];						/* input source file name */
extern int defund;						/* true if undef auto def */
extern int txtfil;						/* input token file		  */
extern int symf;						/* symbol table file	  */
extern int fbfil;						/* forward branch file	  */
extern int fin;							/* generic input file	  */
extern int fout;						/* output file			  */
extern int passno;						/* pass of this phase	  */
extern int outmod;						/* 777 for ok 666 for err */
extern int line;						/* line in source file	  */
extern int savop;						/* token "stack"		  */
extern int xsymbol;						/* symbol offset for ext. */
extern int swapf;						/* true to swap operands  */
extern int rlimit;						/* displacement limit	  */
extern int datbase;						/* base addr for .data	  */
extern int bssbase;						/* base address for .bss  */
extern int numval;						/* value of numeric token */
extern int maxtyp;						/* max type in combination*/

extern int reltp2[];					/* combine r1 + r2		  */
extern int reltm2[];					/* combine r1 - r2		  */
extern int relte2[];					/* combine r1 other r2	  */

extern union token tok;					/* token from token file  */

void addrovf();
void aerror();
void aexit();
int agetw();
void aputw();
void assem();
int checkeos();
void checkreg();
void checkrp();
int combine();
int creat();
void docolon();
void doequal();
void doreloc();
void dotmax();
void fbadv();
void filerr();
void flush();
int getbr();
int maprel();
int ofile();
void op2a();
void op2b();
int open();
void opline();
void oset();
void outb();
void outw();
void readop();
int setbr();
void setup();
