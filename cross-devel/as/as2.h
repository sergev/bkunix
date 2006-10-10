/*
	AS - PDP/11 Assembler part 2

	Header file
*/

#define DEBUG 0

/*
	Symbol Table (without the character part)
*/
extern struct value symtab[SYMBOLS];		/* Permanent symbol Table */
extern struct value usymtab[USERSYMBOLS];	/* User symbol table	*/

#define dotrel	(symtab[0].type.u)		/* Relocation factor for .*/
#define dot	(symtab[0].val.u)		/* Assembler PC			  */
#define dotdot	(symtab[1].val.u)		/* Origin location counter*/

/*
	Forward branch table
*/
extern struct fb_tab fbtab[1024];		/* forward branch table	  */
extern struct fb_tab *fbbufp;			/* current loc in fbtab	  */
extern struct fb_tab *curfb[20];		/* entries for back refs  */
extern struct fb_tab **nxtfb;			/* entries for forward ref*/
extern struct fb_tab *endtable;			/* end of branch table */

/*
	a.out header structure
*/
extern struct hdr {
	int txtmagic;				/* magic number (br)	  */
	unsigned atxtsiz[3];			/* segment sizes		  */
	unsigned symsiz;			/* size of symbol table	  */
	unsigned stksiz;			/* entry location (0)	  */
	unsigned exorig;			/* unused				  */
	unsigned unused;			/* relocation supressed	  */
} hdr;

#define txtsiz	(hdr.atxtsiz[0])
#define datsiz	(hdr.atxtsiz[1])
#define bsssiz	(hdr.atxtsiz[2])

/*
	File seek locations
*/
extern unsigned aseek[3];			/* seek for "absolute"	  */
extern unsigned relseek[3];			/* seek for reloc. data	  */
extern unsigned symseek;			/* seek for symbol table  */

#define txtseek		(aseek[0])		/* seek for txt abs data  */
#define datseek		(aseek[1])		/* seek for data abs data */
#define trelseek	(relseek[0])		/* seek for txt reloc.	  */
#define drelseek	(relseek[1])		/* seek for data reloc.	  */

extern unsigned *tseekp;			/* ptr to abs seek entry  */
extern unsigned *rseekp;			/* ptr to reloc. seek ent.*/

/*
	br/jmp table
*/
#define BRLEN	1024				/* max number of jbr/jcc  */
extern char brtab[];				/* jbr table 1=can't	  */
extern int brtabp;				/* current entry number	  */
extern int brdelt;				/* current displacement	  */

/*
	output buffer structure
*/
struct out_buf {
	char *slot;				/* current word in buffer */
	char *max;				/* &(end of buffer)		  */
	unsigned seek;				/* file seek location	  */
	char buf[512];				/* data buffer			  */
};

extern struct out_buf txtp;			/* abs data buffer		  */
extern struct out_buf relp;			/* relocation data buffer */

/*
	Other variables, etc.
*/

extern int savdot[3];				/* saved . for txt/dat/bss*/

extern unsigned adrbuf[6];			/* for constructint addrs */
extern unsigned *padrb;				/* pointer into adrbuf	  */

extern char argb[];				/* input source file name */
extern int defund;				/* true if undef auto def */
extern int txtfil;				/* input token file		  */
extern int symf;				/* symbol table file	  */
extern int fbfil;				/* forward branch file	  */
extern int fin;					/* generic input file	  */
extern int fout;				/* output file			  */
extern int passno;				/* pass of this phase	  */
extern int outmod;				/* 777 for ok 666 for err */
extern int line;				/* line in source file	  */
extern int savop;				/* token "stack"		  */
extern int xsymbol;				/* symbol offset for ext. */
extern int swapf;				/* true to swap operands  */
extern int rlimit;				/* displacement limit	  */
extern int datbase;				/* base addr for .data	  */
extern int bssbase;				/* base address for .bss  */
extern int numval;				/* value of numeric token */
extern int maxtyp;				/* max type in combination*/

extern int reltp2[];				/* combine r1 + r2		  */
extern int reltm2[];				/* combine r1 - r2		  */
extern int relte2[];				/* combine r1 other r2	  */

extern union token tok;				/* token from token file  */

void addrovf(void);
void aerror(int);
void aexit(int);
int agetw(void);
void aputw(struct out_buf*, int);
void assem(void);
int checkeos(void);
void checkreg(struct value *);
void checkrp(void);
int combine(int, int, int*);
void docolon(union token*);
void doequal(union token*);
void doreloc(struct value*);
void dotmax(void);
void fbadv(void);
void filerr(char*);
void flush(struct out_buf*);
int getbr(void);
int maprel(int);
int ofile(char*);
void op2a(unsigned);
void op2b(unsigned, unsigned);
void opline(void);
void oset(struct out_buf*, int);
void outb(int, int);
void outw(int, int);
void readop(void);
int setbr(int);
void setup(void);
struct value express(void);
struct value expres1(void);
unsigned address(void);
