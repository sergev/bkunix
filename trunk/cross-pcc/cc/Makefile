DESTDIR		= /usr/local
CFLAGS		= -O -Wall -DDESTDIR=\"$(DESTDIR)\"

all:		pdp11-pcc pdp11-cc

install:	pdp11-pcc pdp11-cc
		install -s pdp11-pcc ${DESTDIR}/bin/pdp11-pcc
		install -s pdp11-cc ${DESTDIR}/bin/pdp11-cc

clean:
		rm -f *~ *.o pdp11-pcc pdp11-cc

pdp11-pcc:	pcc.o
		$(CC) ${LDFLAGS} -o pdp11-pcc pcc.o

pdp11-cc:	cc.o
		$(CC) ${LDFLAGS} -o pdp11-cc cc.o
