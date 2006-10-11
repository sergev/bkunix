CC		= gcc -g
CFLAGS		= -O -Wall
DESTDIR		= /usr/local
OBJS		= fsutil.o superblock.o block.c inode.o create.o check.o file.o
PROG		= u6-fsutil

# For Mac OS X
#LIBS		= -largp

all:		$(PROG)

install:	$(PROG)
		install -s $(PROG) ${DESTDIR}/bin/$(PROG)
clean:
		rm -f *~ *.o *.lst *.dis $(PROG)

$(PROG):	$(OBJS)
		$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
