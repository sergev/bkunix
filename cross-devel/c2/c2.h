/*
 * Header for object code improver
 *
 * Several character buffers (used to store contents of registers,
 * constants, etc) needed to be increased in size to handle the
 * larger symbols passed thru from the compiler.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	MAXCPS	32

#ifndef	CHECK
#define	CHECK(x)
#endif

#define	JBR	1
#define	CBR	2
#define	JMP	3
#define	LABEL	4
#define	DLABEL	5
#define	FLABEL	6
#define	EROU	7
#define	JSW	9
#define	MOV	10
#define	CLR	11
#define	COM	12
#define	INC	13
#define	DEC	14
#define	NEG	15
#define	TST	16
#define	ASR	17
#define	ASL	18
#define	SXT	19
#define	CMP	20
#define	ADD	21
#define	SUB	22
#define	BIT	23
#define	BIC	24
#define	BIS	25
#define	MUL	26
#define	DIV	27
#define	ASH	28
#define	XOR	29
#define	TEXT	30
#define	DATA	31
#define	BSS	32
#define	EVEN	33
#define	MOVF	34
#define	MOVOF	35
#define	MOVFO	36
#define	ADDF	37
#define	SUBF	38
#define	DIVF	39
#define	MULF	40
#define	CLRF	41
#define	CMPF	42
#define	NEGF	43
#define	TSTF	44
#define	CFCC	45
#define	SOB	46
#define	JSR	47
#define	SWAB	48
#define	END	49

#define	JEQ	0
#define	JNE	1
#define	JLE	2
#define	JGE	3
#define	JLT	4
#define	JGT	5
#define	JLO	6
#define	JHI	7
#define	JLOS	8
#define	JHIS	9
#define	JPL	10
#define	JMI	11

#define	BYTE	100
#define	LSIZE	512

struct node {
	char	op;
	char	subop;
	struct	node	*forw;
	struct	node	*back;
	struct	node	*ref;
	int	labno;
	char	*code;
	int	refc;
};

extern struct optab {
	char	*opstring;
	int	opcode;
} optab[];

extern char	line[LSIZE];
extern struct	node	first;
extern char	*curlp;
extern int	nbrbr;
extern int	nsaddr;
extern int	redunm;
extern int	iaftbr;
extern int	njp1;
extern int	nrlab;
extern int	nxjump;
extern int	ncmot;
extern int	nrevbr;
extern int	loopiv;
extern int	nredunj;
extern int	nskip;
extern int	ncomj;
extern int	nsob;
extern int	nrtst;
extern int	nlit;

extern int	nchange;
extern int	isn;
extern int	debug;
extern int	lastseg;
extern char	*lasta;
extern char	*lastr;
extern char	*alasta;
extern char	*alastr;
extern char	*firstr;
extern char	revbr[];
extern char	regs[12][MAXCPS + 1];
extern char	conloc[MAXCPS + 1];
extern char	conval[MAXCPS + 1];
extern char	ccloc[MAXCPS + 1];

#define	RT1	10
#define	RT2	11
#define	FREG	5
#define	NREG	5
#define	LABHS	127
#define	OPHS	57

extern struct optab *ophash[OPHS];
struct	node *nonlab(struct node *);
char	*copy(int, char *, char *);
char	*findcon(int);
struct	node *insertl(struct node *);
struct	node *codemove(struct node *);
char	*alloc(int);
void	movedat(void);
void	clearreg(void);
void	rmove(void);
int	jumpsw(void);
void	addsob(void);
void	decref(struct node *);
int	equop(struct node *, struct node *);
int	getnum(char *);
void	reducelit(struct node *);
void	output(void);
int	main(int, char **);
