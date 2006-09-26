DESTDIR		= /usr/local
CFLAGS		= -O -g -Wall -DDESTDIR=\"$(DESTDIR)\"

all:		c0 c1

install:	c0 c1
		install -s c0 ${DESTDIR}/lib/pdp11/c0
		install -s c1 ${DESTDIR}/lib/pdp11/c1

clean:
		rm -f *~ *.o c0 c1 cvopt table.c

c0:		c00.o c01.o c02.o c03.o c04.o c05.o
		${CC} ${LDFLAGS} -o c0 c00.o c01.o c02.o c03.o c04.o c05.o

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h

c1:		c10.o c11.o c12.o c13.o table.o
		${CC} ${LDFLAGS} -o c1 c10.o c11.o c12.o c13.o table.o

c10.o c11.o c12.o c13.o: c1.h

table.c:	optable cvopt
		./cvopt < optable > table.c

cvopt:		cvopt.c
		${CC} $(CFLAGS) $(LDFLAGS) -o cvopt cvopt.c
