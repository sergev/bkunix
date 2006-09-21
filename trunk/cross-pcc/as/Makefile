CFLAGS		= -g -Wall
DESTDIR		= /usr/local

all:		pdp11-asm pdp11-asm2

install:	pdp11-asm pdp11-asm2 opcode.tbl
		install -s pdp11-asm ${DESTDIR}/bin/pdp11-asm
		install -s pdp11-asm2 ${DESTDIR}/lib/pdp11/asm2
		install opcode.tbl ${DESTDIR}/lib/pdp11/opcode.tbl

clean:
		rm -f *~ *.o pdp11-asm pdp11-asm2

pdp11-asm: as11.o as12.o as13.o as14.o as15.o as16.o as17.o as18.o
		$(CC) $(CFLAGS) -o $@ as11.o as12.o as13.o as14.o as15.o as16.o as17.o as18.o

pdp11-asm2: as21.o as22.o as23.o as24.o as26.o as27.o as28.o
		$(CC) $(CFLAGS) -o $@ as21.o as22.o as23.o as24.o as26.o as27.o as28.o

depend:
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CFLAGS) *.c |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
as11.o: as11.c as.h as1.h
as12.o: as12.c as.h as1.h
as13.o: as13.c as.h as1.h
as14.o: as14.c as.h as1.h
as15.o: as15.c as.h as1.h
as16.o: as16.c as.h as1.h
as17.o: as17.c as.h as1.h
as18.o: as18.c as.h as1.h
as21.o: as21.c as.h as2.h
as22.o: as22.c as.h as2.h
as23.o: as23.c as.h as2.h
as24.o: as24.c as.h as2.h
as26.o: as26.c as.h as2.h
as27.o: as27.c as.h as2.h
as28.o: as28.c as.h as2.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
