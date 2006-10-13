#
# Make LSI-UNIX system.
#

# GNU compiler.
#CC		= pdp11-gcc -Wall
#CFLAGS		= -Os -fomit-frame-pointer -fno-builtin -I. -DKERNEL -DDEBUG
#LIBS		=

# Ritchie's compiler.
#CC		= pdp11-cc

# Johnson's portable compiler.
CC		= pdp11-pcc

CFLAGS		= -O
LIBS		= $(shell sh findlib.sh $(CC) ) -lcrt

# Using ported AT&T UNIX utils.
AS		= pdp11-asm
LD		= pdp11-ld
SIZE		= pdp11-size -o
FSUTIL		= u6-fsutil -F
LDFLAGS		= -X

ASRCS		= high.S mch.S
ASOBJS		= high.o mch.o
CSRCS		= alloc.c bio.c \
		  clock.c fio.c iget.c main.c nami.c rdwri.c sig.c slp.c \
		  subr.c sys1.c sys2.c sys3.c sys4.c sysent.c trap.c
COBJS		= alloc.o bio.o \
		  clock.o fio.o iget.o main.o nami.o rdwri.o sig.o slp.o \
		  subr.o sys1.o sys2.o sys3.o sys4.o sysent.o trap.o
OBJS		= $(ASOBJS) $(COBJS)

vpath %.c . dev
vpath %.S . boot

CPPFLAGS	= -I. -DKERNEL

CSRCS		+= dev/bktty.c dev/bkfd.c
OBJS		+= bktty.o bkfd.o
HIBASE		= 0120000

all:		bkunix bkboot
		@echo "Warning: For BK-0011, size must be less than 040000"
		$(SIZE) bkunix

clean:
		rm -f *~ dev/*~ boot/*~ *.o *.s a.out bkunix bkboot bkunix.dis bkunix.map

bkunix:		$(OBJS)
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) $(LIBS)
		$(SIZE) $@
		pdp11-nm $@ > $@.map
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

bkboot:		bkboot.o
		mv $< $@
		pdp11-strip $@

install:	bkunix bkboot
		$(FSUTIL) -b bkboot -a root.bkd bkunix

count:		$(ASRCS) $(CSRCS)
		wc $(ASRCS) $(CSRCS)

.c.s:
		$(CC) $(CFLAGS) $(CPPFLAGS) -S $<

depend:
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CFLAGS) *.S *.c |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
high.o: high.S param.h
low.o: low.S param.h
mch.o: mch.S param.h
alloc.o: alloc.c param.h systm.h filsys.h buf.h inode.h user.h
bio.o: bio.c param.h user.h buf.h systm.h proc.h
clock.o: clock.c param.h systm.h user.h proc.h
fio.o: fio.c param.h user.h filsys.h file.h inode.h reg.h
iget.o: iget.c param.h systm.h user.h inode.h filsys.h buf.h
main.o: main.c param.h user.h systm.h proc.h inode.h
nami.o: nami.c param.h inode.h user.h systm.h buf.h
rdwri.o: rdwri.c param.h inode.h user.h buf.h systm.h
sig.o: sig.c param.h systm.h user.h proc.h inode.h reg.h
slp.o: slp.c param.h user.h proc.h systm.h file.h inode.h buf.h
subr.o: subr.c param.h inode.h user.h buf.h systm.h
sys1.o: sys1.c param.h systm.h user.h proc.h buf.h reg.h inode.h file.h
sys2.o: sys2.c param.h systm.h user.h reg.h file.h inode.h
sys3.o: sys3.c param.h systm.h reg.h buf.h filsys.h user.h inode.h file.h
sys4.o: sys4.c param.h user.h reg.h inode.h systm.h proc.h
sysent.o: sysent.c param.h systm.h
trap.o: trap.c param.h systm.h user.h proc.h reg.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
