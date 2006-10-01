#
# Compile PCC run time library.
#
DESTDIR		= /usr/local
LDESTDIR	= $(DESTDIR)/lib/pdp11
CC		= $(DESTDIR)/bin/pdp11-pcc
AS		= $(DESTDIR)/bin/pdp11-asm
AR		= $(DESTDIR)/bin/pdp11-ar
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	=

SRCS		= aldiv.S almul.S alrem.S csv.S ldiv.S lmul.S lrem.S udiv.S \
		  uldiv.S ulrem.S ulsh.S
OBJS		= aldiv.o csv.o almul.o alrem.o ldiv.o lmul.o lrem.o udiv.o \
		  uldiv.o ulrem.o ulsh.o

all:		libcrt.a

install:	all
		[ -d ${LDESTDIR} ] || install -d ${LDESTDIR}
		install libcrt.a ${LDESTDIR}/libcrt.a

clean:
		rm -f *~ *.o a.out core libcrt.a


libcrt.a:	${OBJS}
		@-rm -f $@
		${AR} cru $@ ${OBJS}

.S.o:
		${CC} ${CPPFLAGS} -c -o $@ $<

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
aldiv.o: aldiv.S
almul.o: almul.S
alrem.o: alrem.S
csv.o: csv.S
ldiv.o: ldiv.S
lmul.o: lmul.S
lrem.o: lrem.S
udiv.o: udiv.S
ulrem.o: ulrem.S
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
