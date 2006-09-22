#
# Compile PCC run time library.
#
DESTDIR		= /usr/local
LDESTDIR	= $(DESTDIR)/lib/pdp11
CC		= $(DESTDIR)/bin/pdp11-pcc
AS		= $(DESTDIR)/bin/pdp11-asm
STRIP		= $(DESTDIR)/bin/pdp11-strip
AR		= $(DESTDIR)/bin/pdp11-ar
RANLIB		= $(DESTDIR)/bin/pdp11-ranlib
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	=

SRCS		= aldiv.S almul.S alrem.S csv.S ldiv.S lmul.S lrem.S udiv.S \
		  uldiv.S ulrem.S ulsh.S
OBJS		= aldiv.o csv.o almul.o alrem.o ldiv.o lmul.o lrem.o udiv.o \
		  uldiv.o ulrem.o ulsh.o

all:		libpcc.a

install:	all
		[ -d ${LDESTDIR} ] || install -d ${LDESTDIR}
		install libpcc.a ${LDESTDIR}/libpcc.a
		${RANLIB} ${LDESTDIR}/libpcc.a

clean:
		rm -f *~ *.o a.out core libpcc.a


libpcc.a:	${OBJS}
		${AR} cru libpcc.a ${OBJS}

.S.o:
		${CPP} -E ${CPPFLAGS} $< | $(AS) -o $@
		# @${STRIP} -x $*.o

depend:
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CPPFLAGS) *.S |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
aldiv.o: aldiv.S DEFS.h
almul.o: almul.S DEFS.h
alrem.o: alrem.S DEFS.h
csv.o: csv.S DEFS.h
ldiv.o: ldiv.S DEFS.h
lmul.o: lmul.S DEFS.h
lrem.o: lrem.S DEFS.h
udiv.o: udiv.S DEFS.h
uldiv.o: uldiv.S DEFS.h
ulrem.o: ulrem.S DEFS.h
ulsh.o: ulsh.S DEFS.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
