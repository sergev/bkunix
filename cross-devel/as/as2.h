/*
 * AS - PDP/11 Assembler part 2
 *
 * Header file
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#define DEBUG 0

/*
        br/jmp table length
*/
#define BRLEN 1024 /* max number of jbr/jcc  */

/*
        a.out header structure
*/
struct hdr {
    int txtmagic;        /* magic number (br)	  */
    unsigned atxtsiz[3]; /* segment sizes		  */
    unsigned symsiz;     /* size of symbol table	  */
    unsigned stksiz;     /* entry location (0)	  */
    unsigned exorig;     /* unused				  */
    unsigned unused;     /* relocation supressed	  */
};

/*
        output buffer structure
*/
struct out_buf {
    char *slot;    /* current word in buffer */
    char *max;     /* &(end of buffer)		  */
    unsigned seek; /* file seek location	  */
    char buf[512]; /* data buffer			  */
};

/*
        Pass2 context - all former globals
*/
struct pass2 {
    struct value symtab[SYMBOLS];
    struct value usymtab[USERSYMBOLS];

    struct fb_tab fbtab[1024];
    struct fb_tab *fbbufp;
    struct fb_tab *curfb[20];
    struct fb_tab **nxtfb;
    struct fb_tab *endtable;

    struct hdr hdr;

    unsigned aseek[3];
    unsigned relseek[3];
    unsigned symseek;
    unsigned *tseekp;
    unsigned *rseekp;

    char brtab[BRLEN / 8];
    int brtabp;
    int brdelt;

    struct out_buf txtp;
    struct out_buf relp;

    int savdot[3];
    unsigned adrbuf[6];
    unsigned *padrb;
    char argb[44];
    int defund;
    int txtfil;
    int symf;
    int fbfil;
    int fin;
    int fout;
    int passno;
    int outmod;
    int line;
    int savop;
    void *xsymbol;
    int swapf;
    int rlimit;
    int datbase;
    int bssbase;
    int numval;
    int maxtyp;
    union token tok;

    char *atmp1;
    char *atmp2;
    char *atmp3;
    char *outfile;
    int debug;
};

/* Accessors for values derived from struct fields */
#define dotrel(p2)   ((p2)->symtab[0].type.u)
#define dot(p2)      ((p2)->symtab[0].val.u)
#define dotdot(p2)   ((p2)->symtab[1].val.u)
#define txtsiz(p2)   ((p2)->hdr.atxtsiz[0])
#define datsiz(p2)   ((p2)->hdr.atxtsiz[1])
#define bsssiz(p2)   ((p2)->hdr.atxtsiz[2])
#define txtseek(p2)  ((p2)->aseek[0])
#define datseek(p2)  ((p2)->aseek[1])
#define trelseek(p2) ((p2)->relseek[0])
#define drelseek(p2) ((p2)->relseek[1])

void pass2_init(struct pass2 *p2);
int *pass2_reltp2(struct pass2 *p2);
int *pass2_reltm2(struct pass2 *p2);
int *pass2_relte2(struct pass2 *p2);

void p2_addrovf(struct pass2 *p2);
void p2_aerror(struct pass2 *p2, int);
void p2_aexit(struct pass2 *p2, int);
int p2_agetw(struct pass2 *p2);
void p2_aputw(struct pass2 *p2, struct out_buf *, int);
void p2_assem(struct pass2 *p2);
int p2_checkeos(struct pass2 *p2);
void p2_checkreg(struct pass2 *p2, struct value *);
void p2_checkrp(struct pass2 *p2);
int p2_combine(struct pass2 *p2, int, int, int *);
void p2_docolon(struct pass2 *p2, union token *);
void p2_doequal(struct pass2 *p2, union token *);
void p2_doreloc(struct pass2 *p2, struct value *);
void p2_dotmax(struct pass2 *p2);
void p2_fbadv(struct pass2 *p2);
void p2_filerr(struct pass2 *p2, char *);
void p2_flush(struct pass2 *p2, struct out_buf *);
int p2_getbr(struct pass2 *p2);
int p2_maprel(struct pass2 *p2, int);
int p2_ofile(struct pass2 *p2, char *);
void p2_op2a(struct pass2 *p2, unsigned);
void p2_op2b(struct pass2 *p2, unsigned, unsigned);
void p2_opline(struct pass2 *p2);
void p2_oset(struct pass2 *p2, struct out_buf *, int);
void p2_outb(struct pass2 *p2, int, int);
void p2_outw(struct pass2 *p2, int, int);
void p2_readop(struct pass2 *p2);
int p2_setbr(struct pass2 *p2, int);
void p2_setup(struct pass2 *p2);
struct value p2_express(struct pass2 *p2);
struct value p2_expres1(struct pass2 *p2);
unsigned p2_address(struct pass2 *p2);
