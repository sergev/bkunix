CC		= gcc -g
CFLAGS		= -O -Wall
OBJS		= superblock.o block.c inode.o create.o check.o file.o
PROGS		= u6fs-util

# For Mac OS X
#LIBS		= -largp

all:		$(PROGS)

clean:
		rm -f *~ *.o *.lst *.dis $(PROGS) floppy.dsk

u6fs-util:	fsutil.o $(OBJS)
		$(CC) $(LDFLAGS) -o $@ fsutil.o $(OBJS) $(LIBS)
