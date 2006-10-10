CC		= gcc -g
CFLAGS		= -O -Wall
OBJS		= superblock.o block.c inode.o create.o check.o file.o
PROGS		= u6fs-util

# For Mac OS X
#LIBS		= -largp

all:		$(PROGS) rxboot rxboot2

rxboot:		rxboot.s
		pdp11-as $< -o $@
		pdp11-strip $@
		pdp11-objdump -D $@ > $@.dis

rxboot2:	rxboot2.s
		pdp11-as $< -o $@
		pdp11-strip $@
		pdp11-objdump -D $@ > $@.dis

clean:
		rm -f *~ *.o *.lst *.dis $(PROGS) floppy.dsk

u6fs-util:	fsutil.o $(OBJS)
		$(CC) $(LDFLAGS) -o $@ fsutil.o $(OBJS) $(LIBS)

floppy:		$(PROGS) rxboot rxboot2
		./u6fs-util -n -s256000 -brxboot -Brxboot2 -v floppy.dsk
		./u6fs-util floppy.dsk
		./u6fs-util -c -v floppy.dsk
