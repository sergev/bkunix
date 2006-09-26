#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "a.out.h"

/*
 * PDP-11 opcode types.
 */
#define OPCODE_NO_OPS		0
#define OPCODE_REG		1	/* register */
#define OPCODE_OP		2	/* generic operand */
#define OPCODE_REG_OP		3	/* register and generic operand */
#define OPCODE_REG_OP_REV	4	/* register and generic operand,
					   reversed syntax */
#define OPCODE_AC_FOP		5	/* fpu accumulator and generic float
					   operand */
#define OPCODE_OP_OP		6	/* two generic operands */
#define OPCODE_DISPL		7	/* pc-relative displacement */
#define OPCODE_REG_DISPL	8	/* redister and pc-relative
					   displacement */
#define OPCODE_IMM8		9	/* 8-bit immediate */
#define OPCODE_IMM6		10	/* 6-bit immediate */
#define OPCODE_IMM3		11	/* 3-bit immediate */
#define OPCODE_ILLEGAL		12	/* illegal instruction */
#define OPCODE_FOP_AC		13	/* generic float argument, then fpu
					   accumulator */
#define OPCODE_FOP		14	/* generic float operand */
#define OPCODE_AC_OP		15	/* fpu accumulator and generic int
					   operand */
#define OPCODE_OP_AC		16	/* generic int argument, then fpu
					   accumulator */

/*
 * PDP-11 instruction set extensions.
 */
#define NONE		0	/* not in instruction set */
#define BASIC		1	/* basic instruction set (11/20 etc) */
#define CSM		2	/* commercial instruction set */
#define CIS		3	/* commercial instruction set */
#define EIS		4	/* extended instruction set (11/45 etc) */
#define FIS		5	/* KEV11 floating-point instructions */
#define FPP		6	/* FP-11 floating-point instructions */
#define LEIS		7	/* limited extended instruction set (11/40 etc) */
#define MFPT		8	/* move from processor type */
#define MPROC		9	/* multiprocessor instructions: tstset, wrtlck */
#define MXPS		10	/* move from/to processor status */
#define SPL		11	/* set priority level */
#define UCODE		12	/* microcode instructions: ldub, med, xfc */
#define EXT_NUM		13	/* total number of extension types */

struct opcode {
	const char *name;
	int opcode;
	int mask;
	int type;
	int extension;
} op[] = {
  /* name,	pattern, mask,	opcode type,		insn type,    alias */
  { "halt",	0x0000,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "wait",	0x0001,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "rti",	0x0002,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "bpt",	0x0003,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "iot",	0x0004,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "reset",	0x0005,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "rtt",	0x0006,	0xffff, OPCODE_NO_OPS,		LEIS },
  { "mfpt",	0x0007,	0xffff, OPCODE_NO_OPS,		MFPT },
  { "jmp",	0x0040,	0xffc0, OPCODE_OP,		BASIC },
  { "rts",	0x0080,	0xfff8, OPCODE_REG,		BASIC },
  { "",		0x0088, 0xfff8, OPCODE_ILLEGAL,		NONE },
  { "",		0x0090, 0xfff8, OPCODE_ILLEGAL,		NONE },
  { "spl",	0x0098,	0xfff8, OPCODE_IMM3,		SPL },
  { "nop",	0x00a0,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "clc",	0x00a1,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "clv",	0x00a2,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_3",	0x00a3,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "clz",	0x00a4,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_5",	0x00a5,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_6",	0x00a6,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_7",	0x00a7,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cln",	0x00a8,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_9",	0x00a9,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_a",	0x00aa,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_b",	0x00ab,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_c",	0x00ac,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_d",	0x00ad,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "cl_e",	0x00ae,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "ccc",	0x00af,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_0",	0x00b0,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "sec",	0x00a1,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "sev",	0x00b2,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_3",	0x00b3,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "sez",	0x00b4,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_5",	0x00b5,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_6",	0x00b6,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_7",	0x00b7,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "sen",	0x00b8,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_9",	0x00b9,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_a",	0x00ba,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_b",	0x00bb,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_c",	0x00bc,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_d",	0x00bd,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "se_e",	0x00be,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "scc",	0x00bf,	0xffff, OPCODE_NO_OPS,		BASIC },
  { "swab",	0x00c0,	0xffc0, OPCODE_OP,		BASIC },
  { "br",	0x0100, 0xff00, OPCODE_DISPL,		BASIC },
  { "bne",	0x0200, 0xff00, OPCODE_DISPL,		BASIC },
  { "beq",	0x0300, 0xff00, OPCODE_DISPL,		BASIC },
  { "bge",	0x0400, 0xff00, OPCODE_DISPL,		BASIC },
  { "blt",	0x0500, 0xff00, OPCODE_DISPL,		BASIC },
  { "bgt",	0x0600, 0xff00, OPCODE_DISPL,		BASIC },
  { "ble",	0x0700, 0xff00, OPCODE_DISPL,		BASIC },
  { "jsr",	0x0800, 0xfe00, OPCODE_REG_OP,		BASIC },
  { "clr",	0x0a00, 0xffc0, OPCODE_OP,		BASIC },
  { "com",	0x0a40, 0xffc0, OPCODE_OP,		BASIC },
  { "inc",	0x0a80, 0xffc0, OPCODE_OP,		BASIC },
  { "dec",	0x0ac0, 0xffc0, OPCODE_OP,		BASIC },
  { "neg",	0x0b00, 0xffc0, OPCODE_OP,		BASIC },
  { "adc",	0x0b40, 0xffc0, OPCODE_OP,		BASIC },
  { "sbc",	0x0b80, 0xffc0, OPCODE_OP,		BASIC },
  { "tst",	0x0bc0, 0xffc0, OPCODE_OP,		BASIC },
  { "ror",	0x0c00, 0xffc0, OPCODE_OP,		BASIC },
  { "rol",	0x0c40, 0xffc0, OPCODE_OP,		BASIC },
  { "asr",	0x0c80, 0xffc0, OPCODE_OP,		BASIC },
  { "asl",	0x0cc0, 0xffc0, OPCODE_OP,		BASIC },
  { "mark",	0x0d00, 0xffc0, OPCODE_IMM6,		LEIS },
  { "mfpi",	0x0d40, 0xffc0, OPCODE_OP,		BASIC },
  { "mtpi",	0x0d80, 0xffc0, OPCODE_OP,		BASIC },
  { "sxt",	0x0dc0, 0xffc0, OPCODE_OP,		LEIS },
  { "csm",	0x0e00, 0xffc0, OPCODE_OP,		CSM },
  { "tstset",	0x0e40, 0xffc0, OPCODE_OP,		MPROC },
  { "wrtlck",	0x0e80, 0xffc0, OPCODE_OP,		MPROC },
/*{ "",		0x0ec0, 0xffe0, OPCODE_ILLEGAL,		NONE },*/
  { "mov",	0x1000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "cmp",	0x2000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bit",	0x3000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bic",	0x4000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bis",	0x5000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "add",	0x6000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "mul",	0x7000, 0xfe00, OPCODE_REG_OP_REV,	EIS },
  { "div",	0x7200, 0xfe00, OPCODE_REG_OP_REV,	EIS },
  { "ash",	0x7400, 0xfe00, OPCODE_REG_OP_REV,	EIS },
  { "ashc",	0x7600, 0xfe00, OPCODE_REG_OP_REV,	EIS },
  { "xor",	0x7800, 0xfe00, OPCODE_REG_OP,		LEIS },
  { "fadd",	0x7a00, 0xfff8, OPCODE_REG,		FIS },
  { "fsub",	0x7a08, 0xfff8, OPCODE_REG,		FIS },
  { "fmul",	0x7a10, 0xfff8, OPCODE_REG,		FIS },
  { "fdiv",	0x7a18, 0xfff8, OPCODE_REG,		FIS },
/*{ "",		0x7a20, 0xffe0, OPCODE_ILLEGAL,		NONE },*/
/*{ "",		0x7a40, 0xffc0, OPCODE_ILLEGAL,		NONE },*/
/*{ "",		0x7a80, 0xff80, OPCODE_ILLEGAL,		NONE },*/
/*{ "",		0x7b00, 0xffe0, OPCODE_ILLEGAL,		NONE },*/
  { "l2dr",	0x7c10, 0xfff8, OPCODE_REG,		CIS },	/*l2d*/
  { "movc",	0x7c18, 0xffff, OPCODE_NO_OPS,		CIS },
  { "movrc",	0x7c19, 0xffff, OPCODE_NO_OPS,		CIS },
  { "movtc",	0x7c1a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "locc",	0x7c20, 0xffff, OPCODE_NO_OPS,		CIS },
  { "skpc",	0x7c21, 0xffff, OPCODE_NO_OPS,		CIS },
  { "scanc",	0x7c22, 0xffff, OPCODE_NO_OPS,		CIS },
  { "spanc",	0x7c23, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmpc",	0x7c24, 0xffff, OPCODE_NO_OPS,		CIS },
  { "matc",	0x7c25, 0xffff, OPCODE_NO_OPS,		CIS },
  { "addn",	0x7c28, 0xffff, OPCODE_NO_OPS,		CIS },
  { "subn",	0x7c29, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmpn",	0x7c2a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtnl",	0x7c2b, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtpn",	0x7c2c, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtnp",	0x7c2d, 0xffff, OPCODE_NO_OPS,		CIS },
  { "ashn",	0x7c2e, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtln",	0x7c2f, 0xffff, OPCODE_NO_OPS,		CIS },
  { "l3dr",	0x7c30, 0xfff8, OPCODE_REG,		CIS },	/*l3d*/
  { "addp",	0x7c38, 0xffff, OPCODE_NO_OPS,		CIS },
  { "subp",	0x7c39, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmpp",	0x7c3a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtpl",	0x7c3b, 0xffff, OPCODE_NO_OPS,		CIS },
  { "mulp",	0x7c3c, 0xffff, OPCODE_NO_OPS,		CIS },
  { "divp",	0x7c3d, 0xffff, OPCODE_NO_OPS,		CIS },
  { "ashp",	0x7c3e, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtlp",	0x7c3f, 0xffff, OPCODE_NO_OPS,		CIS },
  { "movci",	0x7c58, 0xffff, OPCODE_NO_OPS,		CIS },
  { "movrci",	0x7c59, 0xffff, OPCODE_NO_OPS,		CIS },
  { "movtci",	0x7c5a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "locci",	0x7c60, 0xffff, OPCODE_NO_OPS,		CIS },
  { "skpci",	0x7c61, 0xffff, OPCODE_NO_OPS,		CIS },
  { "scanci",	0x7c62, 0xffff, OPCODE_NO_OPS,		CIS },
  { "spanci",	0x7c63, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmpci",	0x7c64, 0xffff, OPCODE_NO_OPS,		CIS },
  { "matci",	0x7c65, 0xffff, OPCODE_NO_OPS,		CIS },
  { "addni",	0x7c68, 0xffff, OPCODE_NO_OPS,		CIS },
  { "subni",	0x7c69, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmpni",	0x7c6a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtnli",	0x7c6b, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtpni",	0x7c6c, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtnpi",	0x7c6d, 0xffff, OPCODE_NO_OPS,		CIS },
  { "ashni",	0x7c6e, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtlni",	0x7c6f, 0xffff, OPCODE_NO_OPS,		CIS },
  { "addpi",	0x7c78, 0xffff, OPCODE_NO_OPS,		CIS },
  { "subpi",	0x7c79, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cmppi",	0x7c7a, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtpli",	0x7c7b, 0xffff, OPCODE_NO_OPS,		CIS },
  { "mulpi",	0x7c7c, 0xffff, OPCODE_NO_OPS,		CIS },
  { "divpi",	0x7c7d, 0xffff, OPCODE_NO_OPS,		CIS },
  { "ashpi",	0x7c7e, 0xffff, OPCODE_NO_OPS,		CIS },
  { "cvtlpi",	0x7c7f, 0xffff, OPCODE_NO_OPS,		CIS },
  { "med",	0x7d80, 0xffff, OPCODE_NO_OPS,		UCODE },
  { "xfc",	0x7dc0, 0xffc0, OPCODE_IMM6,		UCODE },
  { "sob",	0x7e00, 0xfe00, OPCODE_REG_DISPL,	LEIS },
  { "bpl",	0x8000, 0xff00, OPCODE_DISPL,		BASIC },
  { "bmi",	0x8100, 0xff00, OPCODE_DISPL,		BASIC },
  { "bhi",	0x8200, 0xff00, OPCODE_DISPL,		BASIC },
  { "blos",	0x8300, 0xff00, OPCODE_DISPL,		BASIC },
  { "bvc",	0x8400, 0xff00, OPCODE_DISPL,		BASIC },
  { "bvs",	0x8500, 0xff00, OPCODE_DISPL,		BASIC },
  { "bcc",	0x8600, 0xff00, OPCODE_DISPL,		BASIC },/*bhis*/
  { "bcs",	0x8700, 0xff00, OPCODE_DISPL,		BASIC },/*blo*/
  { "emt",	0x8800, 0xff00, OPCODE_IMM8,		BASIC },
  { "sys",	0x8900, 0xff00, OPCODE_IMM8,		BASIC },/*trap*/
  { "clrb",	0x8a00, 0xffc0, OPCODE_OP,		BASIC },
  { "comb",	0x8a40, 0xffc0, OPCODE_OP,		BASIC },
  { "incb",	0x8a80, 0xffc0, OPCODE_OP,		BASIC },
  { "decb",	0x8ac0, 0xffc0, OPCODE_OP,		BASIC },
  { "negb",	0x8b00, 0xffc0, OPCODE_OP,		BASIC },
  { "adcb",	0x8b40, 0xffc0, OPCODE_OP,		BASIC },
  { "sbcb",	0x8b80, 0xffc0, OPCODE_OP,		BASIC },
  { "tstb",	0x8bc0, 0xffc0, OPCODE_OP,		BASIC },
  { "rorb",	0x8c00, 0xffc0, OPCODE_OP,		BASIC },
  { "rolb",	0x8c40, 0xffc0, OPCODE_OP,		BASIC },
  { "asrb",	0x8c80, 0xffc0, OPCODE_OP,		BASIC },
  { "aslb",	0x8cc0, 0xffc0, OPCODE_OP,		BASIC },
  { "mtps",	0x8d00, 0xffc0, OPCODE_OP,		MXPS },
  { "mfpd",	0x8d40, 0xffc0, OPCODE_OP,		BASIC },
  { "mtpd",	0x8d80, 0xffc0, OPCODE_OP,		BASIC },
  { "mfps",	0x8dc0, 0xffc0, OPCODE_OP,		MXPS },
  { "movb",	0x9000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "cmpb",	0xa000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bitb",	0xb000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bicb",	0xc000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "bisb",	0xd000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "sub",	0xe000, 0xf000, OPCODE_OP_OP,		BASIC },
  { "cfcc",	0xf000, 0xffff, OPCODE_NO_OPS,		FPP },
  { "setf",	0xf001, 0xffff, OPCODE_NO_OPS,		FPP },
  { "seti",	0xf002, 0xffff, OPCODE_NO_OPS,		FPP },
  { "ldub",	0xf003, 0xffff, OPCODE_NO_OPS,		UCODE },
  /* fpp trap	0xf004..0xf008 */
  { "setd",	0xf009, 0xffff, OPCODE_NO_OPS,		FPP },
  { "setl",	0xf00a, 0xffff, OPCODE_NO_OPS,		FPP },
  /* fpp trap	0xf00b..0xf03f */
  { "ldfps",	0xf040, 0xffc0, OPCODE_OP,		FPP },
  { "stfps",	0xf080, 0xffc0, OPCODE_OP,		FPP },
  { "stst",	0xf0c0, 0xffc0, OPCODE_OP,		FPP },
  { "clrf",	0xf100, 0xffc0, OPCODE_FOP,		FPP },
  { "tstf",	0xf140, 0xffc0, OPCODE_FOP,		FPP },
  { "absf",	0xf180, 0xffc0, OPCODE_FOP,		FPP },
  { "negf",	0xf1c0, 0xffc0, OPCODE_FOP,		FPP },
  { "mulf",	0xf200, 0xff00, OPCODE_FOP_AC,		FPP },
  { "modf",	0xf300, 0xff00, OPCODE_FOP_AC,		FPP },
  { "addf",	0xf400, 0xff00, OPCODE_FOP_AC,		FPP },
  { "ldf",	0xf500, 0xff00, OPCODE_FOP_AC,		FPP },	/*movif*/
  { "subf",	0xf600, 0xff00, OPCODE_FOP_AC,		FPP },
  { "cmpf",	0xf700, 0xff00, OPCODE_FOP_AC,		FPP },
  { "stf",	0xf800, 0xff00, OPCODE_AC_FOP,		FPP },	/*movfi*/
  { "divf",	0xf900, 0xff00, OPCODE_FOP_AC,		FPP },
  { "stexp",	0xfa00, 0xff00, OPCODE_AC_OP,		FPP },
  { "stcfi",	0xfb00, 0xff00, OPCODE_AC_OP,		FPP },
  { "stcff",	0xfc00, 0xff00, OPCODE_AC_FOP,		FPP },	/* ? */
  { "ldexp",	0xfd00, 0xff00, OPCODE_OP_AC,		FPP },
  { "ldcif",	0xfe00, 0xff00, OPCODE_OP_AC,		FPP },
  { "ldcff",	0xff00, 0xff00, OPCODE_FOP_AC,		FPP },	/* ? */
/* This entry MUST be last; it is a "catch-all" entry that will match when no
 * other opcode entry matches during disassembly.
 */
  { "",		0x0000, 0x0000, OPCODE_ILLEGAL,		NONE },
};
#define AFTER_INSTRUCTION "\t"
#define OPERAND_SEPARATOR ", "

#define JUMP		0x1000	/* flag that this operand is used in a jump */

/* sign-extend a 16-bit number in an int */
#define SIGN_BITS	(8 * sizeof (int) - 16)
#define sign_extend(x)	(((int)(x) << SIGN_BITS) >> SIGN_BITS)

struct exec hdr;
FILE *textfd, *relfd;
int rflag;
unsigned int relcode, srccode, dstcode, srcrel, dstrel;
unsigned int baseaddr;

/* Symbol table, dynamically allocated. */
struct nlist *stab;
int stabindex, stablen;
struct nlist symtext = { ".text", N_TEXT, 0 };
struct nlist symdata = { ".data", N_DATA, 0 };
struct nlist symbss = { ".bss", N_BSS, 0 };

/*
 * Add a name to symbol table.
 */
void
addsym (name, type, val)
	char *name;
	int type;
	unsigned int val;
{
	if (stabindex >= stablen) {
		if (! stablen) {
			stablen = 100;
			stab = (struct nlist*) malloc (stablen *
				sizeof (struct nlist));
		} else {
			stablen += 100;
			stab = (struct nlist*) realloc (stab,
				stablen * sizeof (struct nlist));
		}
		if (! stab) {
			fprintf (stderr, "dis: out of memory on %.8s\n",
				name);
			exit(2);
		}
	}
	strncpy (stab[stabindex].n_name, name, sizeof(stab->n_name));
	stab[stabindex].n_type = type;
	stab[stabindex].n_value = val;
	++stabindex;
}

/*
 * Print all text or data symbols located at the address.
 */
void
prsym (addr)
	unsigned int addr;
{
	struct nlist *p;
	int printed;

	printed = 0;
	for (p=stab; p<stab+stabindex; ++p) {
		if (p->n_value == addr && ((p->n_type & N_TYPE) == N_TEXT ||
		    (p->n_type & N_TYPE) == N_DATA)) {
			printf ("%06o <%.8s>:\n", addr, p->n_name);
			++printed;
		}
	}
	if (printed == 0 && addr == 0)
		printf ("%06o <.text>:\n", 0);
	if (printed == 0 && addr == hdr.a_text)
		printf ("%06o <.data>:\n", hdr.a_text);
}

/*
 * Find a symbol nearest to the address.
 * If no symbol found, return .text or .data or .bss.
 */
struct nlist *
findsym (addr)
	unsigned int addr;
{
	struct nlist *p, *last;

	last = 0;
	for (p=stab; p<stab+stabindex; ++p) {
		if ((p->n_type & N_TYPE) != N_TEXT &&
		    (p->n_type & N_TYPE) != N_DATA)
			continue;
		if (p->n_value > addr || (last != 0 &&
		    p->n_value < last->n_value))
			continue;
		last = p;
	}
	if (last == 0)
		return addr < symdata.n_value ? &symtext :
			addr < symbss.n_value ? &symdata : &symbss;
	if (last->n_value > addr)
		return &symtext;
	if (symbss.n_value >= last->n_value && symbss.n_value <= addr)
		return &symbss;
	if (symdata.n_value >= last->n_value && symdata.n_value <= addr)
		return &symdata;
	return last;
}

/*
 * Read 16-bit word at current file position.
 */
unsigned int
freadw (fd)
	FILE *fd;
{
	unsigned int val;

	val = getc (fd);
	val |= getc (fd) << 8;
	return val;
}

/*
 * Print relocation information.
 */
void
prrel (r)
	register unsigned int r;
{
	if (r == A_RABS) {
		printf ("  ");
		return;
	}
	putchar ((r & A_RPCREL) ? '.' : '=');
	switch (r & A_RMASK) {
	default:      printf ("?");  break;
	case A_RABS:  printf ("a");  break;
	case A_RTEXT: printf ("t");  break;
	case A_RDATA: printf ("d");  break;
	case A_RBSS:  printf ("b");  break;
	case A_REXT:  printf ("%d", A_RINDEX (r));
	}
}

/*
 * Print integer register name.
 */
void
prreg (reg)
	int reg;
{
	reg &= 7;
	switch (reg) {
	case 0: case 1: case 2: case 3: case 4: case 5:
		printf ("r%d", reg);
		break;
	case 6:
		printf ("sp");
		break;
	case 7:
		printf ("pc");
		break;
	default:
		printf ("r?");
		break;
	}
}

void
praddr (address, rel)
	unsigned int address, rel;
{
	struct nlist *sym;
	char *name;
	int offset;

	if ((rel & A_RMASK) == A_REXT) {
		sym = stab + A_RINDEX (rel);
		name = "???";
		if (sym >= stab && sym < stab + stabindex)
			name = sym->n_name;

		if (address == 0)
			printf ("<%.8s>", name);
		else if (address < 8)
			printf ("<%.8s+%d>", name, address);
		else
			printf ("<%.8s+%#o>", name, address);
		return;
	}
	if (address < 20 && sign_extend(address) > -20)
		printf ("%d", address);
	else
		printf ("%#o", address);

	sym = findsym (address);
	printf (" <%.8s", sym->n_name);
	if (address == sym->n_value) {
		printf (">");
		return;
	}
	offset = address - sym->n_value;
	if (offset >= 0) {
		printf ("+");
	} else {
		printf ("-");
		offset = - offset;
	}
	if (offset < 8)
		printf ("%d>", offset);
	else
		printf ("%#o>", offset);
}

/*
 * Print instruction code and relocation info.
 * From one to three words are printed, depending on addressing mode.
 * Store src and dst additional words in srccode and dstcode.
 * Return 0 on error.
 */
void
prcode (memaddr, opcode)
	unsigned int *memaddr;
	unsigned int opcode;
{
	int i, src, dst, level;

	printf (" %06o", opcode);
	if (rflag)
		prrel (relcode);
	*memaddr += 2;
	level = 1;

	src = (opcode >> 6) & 0x3f;
	dst = opcode & 0x3f;
	for (i=0; op[i].mask; i++)
		if ((opcode & op[i].mask) == op[i].opcode)
			break;
	switch (op[i].type) {
	case OPCODE_OP_OP:
		switch ((src >> 3) & 7) {
		case 2:
		case 3:
			if ((src & 7) != 7)
				break;
			/* fall through */
		case 6:
		case 7:
			srccode = freadw (textfd);
			*memaddr += 2;
			printf (" %06o", srccode);
			if (relfd) {
				srcrel = freadw (relfd);
				if (rflag)
					prrel (srcrel);
			}
			++level;
			break;
		}
		/* fall through */
	case OPCODE_OP:
	case OPCODE_FOP:
	case OPCODE_REG_OP:
	case OPCODE_REG_OP_REV:
	case OPCODE_AC_FOP:
	case OPCODE_FOP_AC:
	case OPCODE_AC_OP:
	case OPCODE_OP_AC:
		switch ((dst >> 3) & 7) {
		case 2:
		case 3:
			if ((dst & 7) != 7)
				break;
			/* fall through */
		case 6:
		case 7:
			dstcode = freadw (textfd);
			*memaddr += 2;
			printf (" %06o", dstcode);
			if (relfd) {
				dstrel = freadw (relfd);
				if (rflag)
					prrel (dstrel);
			}
			++level;
			break;
		}
		break;
	}
	while (level < 3) {
		printf ("       ");
		if (rflag)
			printf ("  ");
		++level;
	}
}

/*
 * Print src or dst operand.
 * Return 0 on error.
 */
void
properand (memaddr, code, argcode, argrel)
     unsigned int memaddr, argcode, argrel;
     int code;
{
	int mode = (code >> 3) & 7;
	int reg = code & 7;

	switch (mode) {
	case 0:
		prreg (reg);
		break;
	case 1:
		printf ("(");
		prreg (reg);
		printf (")");
		break;
	case 2:
		if (reg == 7) {
			printf ("$");
			printf ("%d", sign_extend (argcode));
		} else {
			printf ("(");
			prreg (reg);
			printf (")+");
		}
		break;
	case 3:
		if (reg == 7) {
			if (code & JUMP)
				praddr (argcode, argrel);
			else
				printf ("*$%d", argcode);
		} else {
			printf ("*(");
			prreg (reg);
			printf (")+");
		}
		break;
	case 4:
		printf ("-(");
		prreg (reg);
		printf (")");
		break;
	case 5:
		printf ("*-(");
		prreg (reg);
		printf (")");
		break;
	case 6:
	case 7:
		if (reg == 7) {
			unsigned int address = memaddr + sign_extend (argcode);
			if (mode == 7)
				printf ("*");
			if (!(code & JUMP))
				printf ("$");
			praddr (address, argrel);
		} else {
			if (mode == 7)
				printf ("*");
			printf ("%d", sign_extend (argcode));
			printf ("(");
			prreg (reg);
			printf (")");
		}
		break;
	}
}

void
prfoperand (memaddr, code, argcode, argrel)
	unsigned int memaddr, argcode, argrel;
	int code;
{
	if (((code >> 3) & 7) == 0)
		printf ("fr%d", code & 7);
	else
		properand (memaddr, code, argcode, argrel);
}

void
prinsn (memaddr, opcode)
	unsigned int memaddr, opcode;
{
	int i, src, dst;

	src = (opcode >> 6) & 0x3f;
	dst = opcode & 0x3f;
	for (i=0; op[i].mask; i++)
		if ((opcode & op[i].mask) == op[i].opcode)
			break;
	switch (op[i].type) {
	case OPCODE_NO_OPS:
		printf (op[i].name);
		break;
	case OPCODE_REG:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		prreg (dst);
		break;
	case OPCODE_OP:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (strcmp (op[i].name, "jmp") == 0)
			dst |= JUMP;
		properand (memaddr, dst, dstcode, dstrel);
		break;
	case OPCODE_FOP:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (strcmp (op[i].name, "jmp") == 0)
			dst |= JUMP;
		prfoperand (memaddr, dst);
		break;
	case OPCODE_REG_OP:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		prreg (src);
		printf (OPERAND_SEPARATOR);
		if (strcmp (op[i].name, "jsr") == 0)
			dst |= JUMP;
		properand (memaddr, dst, dstcode, dstrel);
		break;
	case OPCODE_REG_OP_REV:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (memaddr, dst, dstcode, dstrel);
		printf (OPERAND_SEPARATOR);
		prreg (src);
		break;
	case OPCODE_AC_FOP: {
		int ac = (opcode & 0xe0) >> 6;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		printf ("fr%d", ac);
		printf (OPERAND_SEPARATOR);
		prfoperand (memaddr, dst);
		break;
	}
	case OPCODE_FOP_AC: {
		int ac = (opcode & 0xe0) >> 6;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		prfoperand (memaddr, dst);
		printf (OPERAND_SEPARATOR);
		printf ("fr%d", ac);
		break;
	}
	case OPCODE_AC_OP: {
		int ac = (opcode & 0xe0) >> 6;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		printf ("fr%d", ac);
		printf (OPERAND_SEPARATOR);
		properand (memaddr, dst, dstcode, dstrel);
		break;
	}
	case OPCODE_OP_AC: {
		int ac = (opcode & 0xe0) >> 6;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (memaddr, dst, dstcode, dstrel);
		printf (OPERAND_SEPARATOR);
		printf ("fr%d", ac);
		break;
	}
	case OPCODE_OP_OP:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (memaddr - 2, src, srccode, srcrel);
		printf (OPERAND_SEPARATOR);
		properand (memaddr, dst, dstcode, dstrel);
		break;
	case OPCODE_DISPL: {
		int displ = (opcode & 0xff) << 8;
		unsigned int address = memaddr + (sign_extend (displ) >> 7);
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		praddr (address, relcode);
		break;
	}
	case OPCODE_REG_DISPL: {
		int displ = (opcode & 0x3f) << 10;
		unsigned int address = memaddr - (displ >> 9);
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		prreg (src);
		printf (OPERAND_SEPARATOR);
		praddr (address, relcode);
		break;
	}
	case OPCODE_IMM8: {
		int code = opcode & 0xff;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		printf ("%d", code);
		break;
	}
	case OPCODE_IMM6: {
		int code = opcode & 0x3f;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		printf ("%d", code);
		break;
	}
	case OPCODE_IMM3: {
		int code = opcode & 7;
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		printf ("%d", code);
		break;
	}
	case OPCODE_ILLEGAL:
		printf (".word");
		printf (AFTER_INSTRUCTION);
		printf ("%d", opcode);
		break;
	default:
		printf ("???");
	}
}

void
prsection (addr, limit)
	unsigned int *addr, limit;
{
	unsigned int opcode;

	while (*addr < limit) {
		prsym (*addr);
		printf ("%6o:", *addr);
		opcode = freadw (textfd);
		if (relfd)
			relcode = freadw (relfd);
		prcode (addr, opcode);
		putchar ('\t');
		prinsn (*addr, opcode);
		putchar ('\n');
	}
}

void
disasm (fname)
	register char *fname;
{
	unsigned int addr;
	struct nlist sym;

	/* Get a.out header. */
	textfd = fopen (fname, "r");
	if (! textfd) {
		fprintf (stderr, "dis: %s not found\n", fname);
		return;
	}
	if (fread (&hdr, sizeof(hdr), 1, textfd) != 1 || N_BADMAG (hdr)) {
		fprintf (stderr, "dis: %s not an object file\n", fname);
		fclose (textfd);
		return;
	}

	/* Read symbol table. */
	fseek (textfd, N_SYMOFF (hdr), 0);
	for (addr=0; addr<hdr.a_syms; addr+=sizeof(struct nlist)) {
		if (fread (&sym, sizeof(sym), 1, textfd) != 1) {
			fprintf (stderr, "dis: error reading symbol table\n");
			fclose (textfd);
			return;
		}
		addsym (sym.n_name, sym.n_type, sym.n_value);
	}
	symtext.n_value = baseaddr;
	symdata.n_value = baseaddr + hdr.a_text;
	symbss.n_value = baseaddr + hdr.a_text + hdr.a_data;

	/* Prepare text and relocation files. */
	fseek (textfd, N_TXTOFF (hdr), 0);
	if (! (hdr.a_flag & A_NRELFLG)) {
		relfd = fopen (fname, "r");
		if (! relfd) {
			fprintf (stderr, "dis: %s not found\n", fname);
			fclose (textfd);
			return;
		}
		fseek (relfd, N_RELOFF (hdr), 0);
	} else if (rflag) {
		fprintf (stderr, "dis: %s is not relocatable\n", fname);
	}

	/* Print header. */
	printf ("         File: %s\n", fname);
	printf ("         Type: ");
	if (hdr.a_magic == A_FMAGIC)
		printf ("FMAGIC");
	else if (hdr.a_magic == A_NMAGIC)
		printf ("NMAGIC");
	else if (hdr.a_magic == A_IMAGIC)
		printf ("IMAGIC");
	else
		printf ("Unknown");
	printf (" (%#o)", hdr.a_magic);
	if (hdr.a_flag & A_NRELFLG)
		printf (" non-relocatable");
	else
		printf (" relocatable");
	printf ("\n");
	printf ("Section .text: %d bytes\n", hdr.a_text);
	printf ("Section .data: %d bytes\n", hdr.a_data);
	printf (" Section .bss: %d bytes\n", hdr.a_bss);
	printf (" Symbol table: %d names (%d bytes)\n",
		(int) (hdr.a_syms / sizeof(struct nlist)), hdr.a_syms);
	printf ("Entry address: %#o\n", hdr.a_entry + baseaddr);

	/* Print sections. */
	addr = baseaddr;
	if (hdr.a_text > 0) {
		printf ("\nDisassembly of section .text:\n");
		prsection (&addr, baseaddr + hdr.a_text);
	}
	if (hdr.a_data > 0) {
		printf ("\nDisassembly of section .data:\n");
		prsection (&addr, baseaddr + hdr.a_text + hdr.a_data);
		prsym (addr);
	}
	if (hdr.a_bss > 0) {
		printf ("\nDisassembly of section .bss:\n");
		printf ("%06o <.bss>:\t\t\t", symbss.n_value);
		if (rflag)
			printf ("\t");
		printf (". = .+%d\n", hdr.a_bss);
	}

	fclose (textfd);
	if (relfd) {
		fclose (relfd);
		relfd = 0;
	}
}

int
main (argc, argv)
	register char **argv;
{
	register char *cp;

	while(--argc) {
		++argv;
		if (**argv != '-') {
			disasm (*argv);
			continue;
		}
		for (cp = *argv+1; *cp; cp++) {
			switch (*cp) {
			case 'r':       /* -r: print relocation info */
				rflag++;
				break;
			case 'a':       /* -aN: base address */
				while (cp[1] >= '0' && cp[1] <= '7') {
					baseaddr <<= 3;
					baseaddr += cp[1] - '0';
					++cp;
				}
				break;
			default:
				fprintf (stderr, "Usage: disasm [-r] [-aN] file...\n");
				return (1);
			}
		}
	}
	return (0);
}
