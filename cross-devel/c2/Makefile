DESTDIR		= /usr/local/
CC              = gcc -Werror
CFLAGS		= -O -g -Wall -Wno-deprecated-declarations
LDFLAGS		= -g

all:		c2

install:	c2
		install -s c2 ${DESTDIR}/lib/pdp11/c2

clean:
		rm -f *~ *.o c2

c2:		c20.o c21.o
		$(CC) ${LDFLAGS} -o c2 c20.o c21.o

c20.o c21.o:	c2.h
