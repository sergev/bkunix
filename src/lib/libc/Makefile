#
# Compile C standard library.
#
DESTDIR		= /usr/local/lib/pdp11
CC		= pdp11-pcc
AS		= pdp11-as
STRIP		= pdp11-strip
RANLIB		= pdp11-ranlib
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	=

SRCS		= crt0.S exit.S
OBJS		= exit.o

all:		crt0.o libc.a

install:	all
		[ -d ${DESTDIR} ] || install -d ${DESTDIR}
		install crt0.o ${DESTDIR}/crt0.o
		install libc.a ${DESTDIR}/libc.a
		${RANLIB} ${DESTDIR}/libc.a

clean:
		rm -f *~ *.o a.out core libc.a


libc.a:		${OBJS}
		ar cru libc.a ${OBJS}

.S.o:
		${CPP} -E ${CPPFLAGS} $< | $(AS) -o $@
		@${STRIP} -x $*.o

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
crt0.o: crt0.S
exit.o: exit.S
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
