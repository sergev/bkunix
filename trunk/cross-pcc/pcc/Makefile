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
CONFIG=	-DONEPASS "-DFIXSTRUCT=outstruct"
O = -O
CFLAGS = $(O) -I. -I$(M) $(CONFIG)
COPTS = $(CFLAGS)
LDFLAGS = -i

RM=	/bin/rm -f
LN=	/bin/ln -s

P1INCS=	$M/pass1.h macdefs.h $M/manifest.h pcclocal.h $M/config.h $M/ndu.h \
	/usr/include/stdio.h
P1ONEP=	$(P1INCS) $M/onepass.h
P2INCS=	$M/pass2.h macdefs.h mac2defs.h $M/manifest.h pcclocal.h $M/config.h \
	$M/ndu.h /usr/include/stdio.h
P2ONEP=	$(P2INCS) $M/onepass.h

M=../mip
TESTDIR = .

all: ccom 

#
# 'ccom' is a one-pass C compiler.
#
ccom: cgram.o xdefs.o scan.o pftn.o trees.o optim.o code.o local.o \
	reader.o local2.o order.o match.o allo.o comm1.o table.o stab.o
	$(CC) $(LDFLAGS) xdefs.o scan.o pftn.o trees.o \
		optim.o code.o local.o reader.o order.o match.o \
		allo.o -Z comm1.o cgram.o -Z local2.o -Y table.o stab.o \
		-o $(TESTDIR)/ccom

trees.o: $(P1ONEP) $M/trees.c
	$(CC) -c $(COPTS) $M/trees.c
optim.o: $(P1ONEP) $M/optim.c
	$(CC) -c $(COPTS) $M/optim.c
pftn.o: $(P1ONEP) $M/pftn.c
	$(CC) -c $(COPTS) $M/pftn.c
code.o: $(P1ONEP) code.c
	$(CC) -c $(COPTS) code.c
local.o: $(P1ONEP) local.c
	$(CC) -c $(COPTS) local.c
scan.o: $(P1ONEP) $M/scan.c
	$(CC) -c $(COPTS) $M/scan.c
xdefs.o: $(P1ONEP) $M/xdefs.c
	$(CC) -c $(COPTS) $M/xdefs.c
cgram.o: $(P1ONEP) cgram.c
	$(CC) -c $(COPTS) cgram.c
cgram.c: $M/cgram.y pcctokens
	cat pcctokens $M/cgram.y > gram.in
	$(YACC) gram.in
	mv y.tab.c cgram.c
comm1.o: $(P1ONEP) $M/common.c
	$(LN) $M/common.c comm1.c
	$(CC) -c $(COPTS) -DPASS1COMMON comm1.c
	$(RM) comm1.c
table.o: $(P2ONEP) table.c
	$(CC) -c $(COPTS) table.c
reader.o: $(P2ONEP) $M/reader.c
	$(CC) -c $(COPTS) $M/reader.c
local2.o: $(P2ONEP) local2.c
	$(CC) -c $(COPTS) local2.c
order.o: $(P2ONEP) order.c
	$(CC) -c $(COPTS) order.c
match.o: $(P2ONEP) $M/match.c
	$(CC) -c $(COPTS) $M/match.c
allo.o: $(P2ONEP) $M/allo.c
	$(CC) -c $(COPTS) $M/allo.c
stab.o: $(P2ONEP) stab.c
	$(CC) -c $(COPTS) stab.c

install: all
	install -s ${TESTDIR}/ccom ${DESTDIR}/lib/ccom

GREP=	egrep

pcclocal.h: localdefs.h /usr/include/pcc.h
	$(RM) pcclocal.h
	cat /usr/include/pcc.h localdefs.h | $(GREP) '^#[ 	]*(define[ 	][ 	]*PCC(F|T|TM|OM)?_|ifdef|ifndef|endif)' | sed -e 's/PCC[A-Z]*_//' > pcclocal.h 

pcctokens: localdefs.h /usr/include/pcc.h
	$(RM) pcctokens
	cat /usr/include/pcc.h localdefs.h | $(GREP) '^#[ 	]*define[ 	][ 	]*PCC_' | sed -e 's/^#[ 	]*define[ 	][ 	]*PCC_/%term	/' > pcctokens

shrink:
	$(RM) *.o ccom
clean:
	$(RM) *.o ccom cgram.c pcctokens pcclocal.h gram.in
lintall:
	lint -hax -I. -I$M -DONEPASS -DPASS1COMMON cgram.c $M/xdefs.c \
		$M/scan.c $M/pftn.c $M/trees.c $M/optim.c code.c \
		local.c $M/reader.c local2.c order.c $M/match.c $M/allo.c \
		$M/common.c table.c stab.c
