CC		= gcc -g
CFLAGS		= -O -Wall
OBJS		= lsxfs-open.o lsxfs-inode.o lsxfs-create.o lsxfs-check.o
PROGS		= lsx-util
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

lsx-util:	fsutil.o $(OBJS)
		$(CC) $(LDFLAGS) -o $@ fsutil.o $(OBJS) $(LIBS)

floppy:		$(PROGS) rxboot rxboot2
		./lsx-util -n -s256000 -brxboot -Brxboot2 -v floppy.dsk
		./lsx-util floppy.dsk
		./lsx-util -c -v floppy.dsk
