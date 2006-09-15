CFLAGS		= -O -Wall
DESTDIR		= /usr/local/bin

all:		pdp11-pcc

install:	pdp11-pcc
		install -s pdp11-pcc ${DESTDIR}/pdp11-pcc

clean:
		rm -f *~ *.o pdp11-pcc

pdp11-pcc:	pcc.o
		$(CC) ${LDFLAGS} -o pdp11-pcc pcc.o
