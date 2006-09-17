CFLAGS = -O
LDC0FLAGS= -i
LDC1FLAGS= -n

DESTDIR =
LIB=/lib

all: c0 c1 cvopt
c0: c00.o c01.o c02.o c03.o c04.o c05.o
	${CC} ${LDC0FLAGS} ${CFLAGS} -o c0 c00.o c01.o c02.o c03.o c04.o c05.o

c00.o c01.o c02.o c03.o c04.o c05.o: c0.h

c1: c10.o c11.o c12.o c13.o table.o
	${CC} ${LDC1FLAGS} ${CFLAGS} -o c1 c10.o c11.o c12.o c13.o table.o

c10.o c11.o c12.o c13.o: c1.h

table.o: optable cvopt
	./cvopt < optable > junk.c
	${CC} -S junk.c
	sed 's/\.data/\.text/' < junk.s > junk.i
	as -o table.o junk.i
	rm -f junk.i junk.c junk.s

cvopt:	cvopt.c
	${CC} -n -s -O -o cvopt cvopt.c

install: c0 c1
	-mv ${DESTDIR}${LIB}/c0 ${DESTDIR}${LIB}/oc0
	-mv ${DESTDIR}${LIB}/c1 ${DESTDIR}${LIB}/oc1
	install -s c0 ${DESTDIR}${LIB}/c0
	install -s c1 ${DESTDIR}${LIB}/c1

restore:
	mv ${DESTDIR}${LIB}/oc0 ${DESTDIR}${LIB}/c0
	mv ${DESTDIR}${LIB}/oc1 ${DESTDIR}${LIB}/c1

lint:
	lint -haxc -I. c0?.c > lint.c0
	lint -haxc -I. c1?.c > lint.c1

clean:
	rm -f *.o c0 c1 cvopt lint.c?
