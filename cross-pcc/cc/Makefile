CFLAGS		= -O -Wall
DESTDIR		= /usr/local/bin

all:		pdp11-pcc pdp11-cc

install:	pdp11-pcc pdp11-cc
		install -s pdp11-pcc ${DESTDIR}/pdp11-pcc
		install -s pdp11-cc ${DESTDIR}/pdp11-cc

clean:
		rm -f *~ *.o pdp11-pcc pdp11-cc

pdp11-pcc:	pcc.o
		$(CC) ${LDFLAGS} -o pdp11-pcc pcc.o

pdp11-cc:	cc.o
		$(CC) ${LDFLAGS} -o pdp11-cc cc.o
