SEPFLAG=-i
CFLAGS=	-O

DESTDIR=
LIB=	/lib

c2: c20.o c21.o
	cc ${CFLAGS} ${SEPFLAG} -o c2 c20.o c21.o

c20.o c21.o: c2.h

install: c2
	-mv ${DESTDIR}${LIB}/c2 ${DESTDIR}${LIB}/oc2
	install -s c2 ${DESTDIR}${LIB}

restore:
	mv ${DESTDIR}${LIB}/oc2 ${DESTDIR}${LIB}/c2

clean:
	rm -f *.o c2
