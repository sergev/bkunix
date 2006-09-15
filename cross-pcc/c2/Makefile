CFLAGS		= -O -g -Wall
LDFLAGS		= -g
DESTDIR		= /usr/local/lib/pdp11

all:		c2

install:	c2
		install -s c2 ${DESTDIR}/c2

clean:
		rm -f *~ *.o c2

c2:		c20.o c21.o
		$(CC) ${LDFLAGS} -o c2 c20.o c21.o

c20.o c21.o:	c2.h
