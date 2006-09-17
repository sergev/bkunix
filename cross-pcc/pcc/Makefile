#
#	Makefile	1.13	86/03/11
#
# Makefile for PCC
#
#	FIXSTRUCT	no trickery (just output structure)
#
# Some symbols are specific to certain compilers:
#	ONEPASS		compile pass 1 and pass 2 together
#	SPRECC		do single precision in single not double
#	FORT		get f77-style pass 2
#
DESTDIR		= /usr/local/lib/pdp11
M		= ../mip
CFLAGS		= -O -g -I. -I$(M) -DONEPASS -DFIXSTRUCT="outstruct" \
		  -DASSTRINGS -DFLEXNAMES
LDFLAGS 	= -g
YACC		= byacc

P1INCS		= $M/pass1.h macdefs.h $M/manifest.h pcclocal.h $M/config.h $M/ndu.h
P1ONEP		= $(P1INCS) $M/onepass.h
P2INCS		= $M/pass2.h macdefs.h mac2defs.h $M/manifest.h pcclocal.h \
		  $M/config.h $M/ndu.h
P2ONEP		= $(P2INCS) $M/onepass.h

all:		ccom

install:	all
		[ -d ${DESTDIR} ] || install -d ${DESTDIR}
		install -s ccom ${DESTDIR}/ccom

clean:
		rm -f *~ $M/*~ *.o ccom cgram.c pcctokens pcclocal.h gram.in y.output
#
# 'ccom' is a one-pass C compiler.
#
ccom:		cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o \
			reader.o local2.o order.o match.o allo.o comm1.o \
			table.o stab.o
		$(CC) $(LDFLAGS) xdefs.o scan.o pftn.o trees.o optim.o \
			code.o local.o reader.o order.o match.o allo.o \
			comm1.o cgram.o local2.o table.o stab.o -o ccom

trees.o:	$(P1ONEP) $M/trees.c
		$(CC) -c $(CFLAGS) $M/trees.c

optim.o:	$(P1ONEP) $M/optim.c
		$(CC) -c $(CFLAGS) $M/optim.c

pftn.o:		$(P1ONEP) $M/pftn.c
		$(CC) -c $(CFLAGS) $M/pftn.c

code.o:		$(P1ONEP) code.c
		$(CC) -c $(CFLAGS) code.c

local.o:	$(P1ONEP) local.c
		$(CC) -c $(CFLAGS) local.c

scan.o:		$(P1ONEP) $M/scan.c
		$(CC) -c $(CFLAGS) $M/scan.c

xdefs.o:	$(P1ONEP) $M/xdefs.c
		$(CC) -c $(CFLAGS) $M/xdefs.c

cgram.o:	$(P1ONEP) cgram.c
		$(CC) -c $(CFLAGS) cgram.c

cgram.c:	$M/cgram.y pcctokens
		cat pcctokens $M/cgram.y > gram.in
		$(YACC) gram.in
		mv y.tab.c cgram.c

comm1.o:	$(P1ONEP) $M/common.c
		ln -s -f $M/common.c comm1.c
		$(CC) -c $(CFLAGS) -DPASS1COMMON comm1.c
		rm -f comm1.c

table.o:	$(P2ONEP) table.c
		$(CC) -c $(CFLAGS) table.c

reader.o:	$(P2ONEP) $M/reader.c
		$(CC) -c $(CFLAGS) $M/reader.c

local2.o:	$(P2ONEP) local2.c
		$(CC) -c $(CFLAGS) local2.c

order.o:	$(P2ONEP) order.c
		$(CC) -c $(CFLAGS) order.c

match.o:	$(P2ONEP) $M/match.c
		$(CC) -c $(CFLAGS) $M/match.c

allo.o:		$(P2ONEP) $M/allo.c
		$(CC) -c $(CFLAGS) $M/allo.c

stab.o:		$(P2ONEP) stab.c
		$(CC) -c $(CFLAGS) stab.c

pcclocal.h:	localdefs.h pcc.h
		rm -f pcclocal.h
		cat pcc.h localdefs.h | egrep '^#[ 	]*(define[ 	][ 	]*PCC(F|T|TM|OM)?_|ifdef|ifndef|endif)' | sed -e 's/PCC[A-Z]*_//' > pcclocal.h

pcctokens:	localdefs.h pcc.h
		rm -f pcctokens
		cat pcc.h localdefs.h | egrep '^#[ 	]*define[ 	][ 	]*PCC_' | sed -e 's/^#[ 	]*define[ 	][ 	]*PCC_/%term	/' > pcctokens
