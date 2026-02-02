/*
 * Header file for first part of as - PDP/11 Assember
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */

/*
	Symbol Table
*/
struct symtab {
	char name[8];
	struct value v;
};

/*
	Pass1 context - all former globals
*/
#define PASS1_CHARTAB_SIZE 128
#define PASS1_SCHAR_SIZE 16
#define PASS1_ESCTAB_SIZE 16

struct pass1 {
	struct symtab *hshtab[HSHSIZ];
	struct symtab symtab[SYMBOLS+USERSYMBOLS];
	struct symtab *usymtab;
	struct symtab *symend;
	char symbol[8];
	unsigned savdot[3];

	char curfbr[10];
	int curfb[10];
	struct fb_tab nxtfb;

	FILE *fin;
	int fbfil;
	int pof;
	char fileflg;

	int errflg;
	int ifflg;
	char globflag;
	int eos_flag;

	union token tok;
	int line;
	int savop;
	char ch;
	int numval;
	int num_rtn;

	int nargs;
	char **curarg;

	char chartab[PASS1_CHARTAB_SIZE];
	char schar[PASS1_SCHAR_SIZE];
	char esctab[PASS1_ESCTAB_SIZE];
};

/* Accessors for values derived from struct fields */
#define dotrel(p1)	((p1)->symtab[0].v.type.i)
#define dot(p1)		((p1)->symtab[0].v.val.u)
#define dotdot(p1)	((p1)->symtab[1].v.val.u)

void pass1_init(struct pass1 *p1);

int address(struct pass1 *p1);
void add_symbol(struct pass1 *p1, struct symtab*, char*);
void aerror(struct pass1 *p1, int);
void aexit(struct pass1 *p1);
void aputw(struct pass1 *p1);
void assem(struct pass1 *p1);
int checkeos(struct pass1 *p1);
void checkrp(struct pass1 *p1);
void checkreg(struct pass1 *p1, struct value);
unsigned fbcheck(struct pass1 *p1, unsigned);
int f_create(struct pass1 *p1, char*);
void filerr(struct pass1 *p1, char*, char*);
void hash_enter(struct pass1 *p1, struct symtab*);
void opline(struct pass1 *p1);
char number(struct pass1 *p1);
unsigned char rch(struct pass1 *p1);
void readop(struct pass1 *p1);
void rname(struct pass1 *p1, unsigned char);
void setup(struct pass1 *p1);
struct value express(struct pass1 *p1);
unsigned short hash(struct pass1 *p1, char*);
char rsch(struct pass1 *p1);
int combine(struct pass1 *p1, int, int, int);
void write_syms(struct pass1 *p1, int);
void write_fb(struct pass1 *p1, int, struct fb_tab*);
