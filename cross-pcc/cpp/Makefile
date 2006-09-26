DESTDIR		= /usr/local
CFLAGS		= -O -g -Wall -DFLEXNAMES

all:		cpp

install:	cpp
		install -s cpp $(DESTDIR)/lib/pdp11/cpp

clean:
		rm -f *~ *.s *.o cpy.c cpp

cpp:		cpp.o cpy.o
		$(CC) $(CFLAGS) -o $@ cpp.o cpy.o

cpy.c:		cpy.y
		byacc cpy.y
		mv y.tab.c cpy.c

cpy.o:		cpy.c yylex.c

depend:		cpy.c
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
cpp.o: cpp.c
cpy.o: cpy.c yylex.c
yylex.o: yylex.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
