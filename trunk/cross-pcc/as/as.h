/*
	Common include file for C version of as
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

/*
	Magic Numbers (Limits)
*/
#define	SYMBOLS		200
#define	USERSYMBOLS	400
#define HSHSIZ		1553

/*
	Files
*/
#define OPTABL		DESTDIR "/lib/pdp11/opcode.tbl"
#define PASS2		DESTDIR "/lib/pdp11/asm2"

/*
	Special Flags
*/
#define USYMFLAG	04000
#define PSYMFLAG	01000
#define STRINGFLAG	00400
#define ENDTABFLAG	0100000

/*
	Forward Branch Table constants
*/
#define FBBASE		0141
#define FBFWD		0153	/* FBBASE + 10. */

/*
	Tokens
*/
#define TOKEOF		04
#define TOKINT		01
#define TOKFILE		05
#define TOKLSH		035
#define TOKRSH		036
#define TOKVBAR		037
#define TOKSYMBOL	0200

/*
	Special Character / Token values
*/
#define CHARSTRING	0
#define CHARLF		0376
#define CHARNUM		0374
#define CHARSKIP	0372
#define CHARNAME	0370
#define CHARSQUOTE	0366
#define	CHARGARB	0364
#define CHARDQUOTE	0362
#define CHARTOKEN	0360
#define CHARWHITE	0356
#define CHARESCP	0354
#define CHARFIXOR	0352

/*
	Relocation Types
*/
#define TYPEUNDEF	0
#define TYPEABS		1
#define TYPETXT		2
#define TYPEDATA	3
#define TYPEBSS		4
#define TYPEEXT		040
#define TYPEREGIS	024

/*
	Permanent Symbol / Opcode Classes
*/
#define TYPEOPFD	005
#define TYPEOPBR	006
#define TYPEOPJSR	007
#define TYPEOPRTS	010
#define TYPEOPSYS	011
#define TYPEOPMOVF	012
#define TYPEOPDO	013
#define TYPEOPFF	014
#define TYPEOPSO	015
#define TYPEOPBYTE	016
#define TYPEOPASC	017
#define TYPEOPEVEN	020
#define TYPEOPIF	021
#define TYPEOPEIF	022
#define TYPEOPGLB	023
#define TYPEOPTXT	025
#define TYPEOPDAT	026
#define TYPEOPBSS	027
#define TYPEOPMUL	030
#define TYPEOPSOB	031
#define TYPEOPCOM	032
#define TYPEOPEST	033
#define TYPEOPESD	034
#define TYPEOPJBR	035
#define TYPEOPWORD	036
#define TYPEOPJCC	037

/*
	Certain Opcode values
*/
#define OPCODBR		0400
#define OPCODJMP	0100

/*
	Addressing Modes
*/
#define AMDEFERRED	010
#define AMAUTOINCR	020
#define AMAUTODECR	040
#define AMINDEXED	060
#define AMIXDEFER	070
#define AMIMMED		027
#define AMRELATIVE	067

/*
	Common Data Types
*/

struct value {
	union {
		int i;
		unsigned u;
		char b;
	} type;
	union {
		int i;
		unsigned u;
	} val;
};

union token {
	int i;
	unsigned u;
	struct value *v;
	struct symtab *s;
};

struct fb_tab {
	char rel;
	char lblix;
	int val;
};

#define TRUE (1)
#define FALSE (0)
