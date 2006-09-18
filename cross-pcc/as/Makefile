CFLAGS		= -g -Wall
DESTDIR		= /usr/local

all:		pdp11-asm pdp11-asm2 pdp11-reloc

install:	pdp11-asm pdp11-asm2 pdp11-reloc opcode.tbl
		install -s pdp11-asm ${DESTDIR}/lib/pdp11/asm
		install -s pdp11-asm2 ${DESTDIR}/lib/pdp11/asm2
		install -s pdp11-reloc ${DESTDIR}/bin/pdp11-reloc
		install opcode.tbl ${DESTDIR}/lib/pdp11/opcode.tbl

clean:
		rm -f *~ *.o pdp11-asm pdp11-asm2 pdp11-reloc

pdp11-asm: as11.o as12.o as13.o as14.o as15.o as16.o as17.o as18.o
		$(CC) $(CFLAGS) -o $@ as11.o as12.o as13.o as14.o as15.o as16.o as17.o as18.o

as11.o as12.o as13.o as14.o as15.o as16.o as17.o as18.o: as.h as1.h

pdp11-asm2: as21.o as22.o as23.o as24.o as26.o as27.o as28.o
		$(CC) $(CFLAGS) -o $@ as21.o as22.o as23.o as24.o as26.o as27.o as28.o

as21.o as22.o as23.o as24.o as26.o as27.o as28.o: as.h as2.h

pdp11-reloc: reloc.o
		$(CC) $(CFLAGS) -o $@ reloc.o
