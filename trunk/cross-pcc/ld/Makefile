CC		= gcc -g -Wall
CFLAGS		=
DESTDIR		= /usr/local
PROG		= pdp11-size pdp11-strip pdp11-disasm pdp11-reloc \
		  pdp11-nm pdp11-ld #pdp11-ar

all:		$(PROG)

install:	$(PROG)
		install -s pdp11-size   ${DESTDIR}/bin/pdp11-size
		install -s pdp11-strip  ${DESTDIR}/bin/pdp11-strip
		install -s pdp11-disasm ${DESTDIR}/bin/pdp11-disasm
		install -s pdp11-reloc  ${DESTDIR}/bin/pdp11-reloc
		install -s pdp11-nm     ${DESTDIR}/bin/pdp11-nm
		install -s pdp11-ld     ${DESTDIR}/bin/pdp11-ld
		install -s pdp11-ar     ${DESTDIR}/bin/pdp11-ar

clean:
		rm -f *~ *.o $(PROG)

pdp11-size:	size.o
		$(CC) $(LDFLAGS) -o $@ $<

pdp11-strip:	strip.o
		$(CC) $(LDFLAGS) -o $@ $<

pdp11-reloc:	reloc.o
		$(CC) $(LDFLAGS) -o $@ $<

pdp11-disasm:	disasm.o
		$(CC) $(LDFLAGS) -o $@ $<

pdp11-nm:	nm.o
		$(CC) $(LDFLAGS) -o $@ $<

pdp11-ld:	ld.o
		$(CC) $(LDFLAGS) -o $@ $<

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
disasm.o: disasm.c a.out.h
size.o: size.c a.out.h
strip.o: strip.c a.out.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
