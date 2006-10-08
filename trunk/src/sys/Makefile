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
LDFLAGS		= -X
WHERE		= LOW

ASRCS		= low.S mch.S
ASOBJS		= low.o mch.o
CSRCS		= alloc.c bio.c \
		  clock.c fio.c iget.c main.c nami.c rdwri.c sig.c slp.c \
		  subr.c sys1.c sys2.c sys3.c sys4.c sysent.c trap.c
COBJS		= alloc.o bio.o \
		  clock.o fio.o iget.o main.o nami.o rdwri.o sig.o slp.o \
		  subr.o sys1.o sys2.o sys3.o sys4.o sysent.o trap.o
OBJS		= $(ASOBJS) $(COBJS)

# Debug mode: use debuf_printf().
#CFLAGS		+= -DDEBUG
#CSRCS		+= debug.c
#OBJS		+= debug.o

vpath %.c . dev
vpath %.S . boot

# System configuration.
# -DBGOPTION  - background processes
# -DCLOCK     - set to be 0177546 or 0172540 for the line
#		frequency or programmable clock, respectively.
#
#CPPFLAGS	+= -DCLOCK=0177546

# Configuration of device drivers.
# -DAED	 - AED 6200lp floppy
# -DDEC  - DEC floppy
# -DPER  - PER floppy
# -DSYK  - Sykes floppy
# -DRF   - RF floppy
# -DKL   - KL/DL-11 console
# -DTVT  - TV terminal
#
CPPFLAGS	= -I. -DKERNEL -DKL -D$(WHERE) -D$(SYS)
CSRCS		+= dev/kl.c dev/decfd.c
DEVOBJS		= kl.o decfd.o
BKDEVOBJS	= bktty.o bkfd.o
OBJS		+= $(DEVOBJS)
HIOBJS		= high.o mch.o $(COBJS) $(DEVOBJS)
BKOBJS		= high.o mch.o $(COBJS) $(BKDEVOBJS)
HIBASE		= 0120000

all:		bsx boot1 boot2lo boot2hi #lsx1
		@echo "Warning: For 10K LSX system, size must be less than 046000"
		@echo "         for 8k lsx system size must be less than 036500"
		$(SIZE) bsx #lsx1

clean:
		rm -f *~ dev/*~ *.o *.s lsx lsx1 hsx bsx boot1 boot2lo boot2hi debug a.out lsx.dis debug.dis hsx.dis

lsx:
		$(MAKE) WHERE=LOW SYS=DEC $(OBJS)
		$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
		pdp11-disasm $@ > $@.dis

hsx:
		$(MAKE) WHERE=HIGH SYS=DEC $(HIOBJS)
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(HIOBJS) $(LIBS)
		$(SIZE) $@
		pdp11-nm $@ > $@.map
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

bsx:
		$(MAKE) WHERE=HIGH SYS=BK $(BKOBJS)
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(BKOBJS) $(LIBS)
		$(SIZE) $@
		pdp11-nm $@ > $@.map
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

boot1:		rxboot.o
		mv $< $@
		pdp11-strip $@

boot2hi:	boot/rxboot2.S
		rm -f rxboot2.o
		$(MAKE) WHERE=HIGH rxboot2.o
		mv rxboot2.o $@
		pdp11-strip $@

boot2lo:	boot/rxboot2.S
		rm -f rxboot2.o
		$(MAKE) WHERE=LOW rxboot2.o
		mv rxboot2.o $@
		pdp11-strip $@

bkboot:		bkboot.o
		mv $< $@
		pdp11-strip $@

hinstall:	clean hsx boot1 boot2hi
		lsx-util -b boot1 -B boot2hi -a root.dsk hsx

linstall:	clean lsx boot1 boot2lo
		lsx-util -b boot1 -B boot2lo -a root.dsk lsx

bkinstall:	clean bsx bkboot
		lsx-util -F -b bkboot -a bkroot.dsk bsx

lsx1:		$(ASOBJS) $(CSRCS)
		cat $(CSRCS) > kernel.c
		$(CC) $(CFLAGS) $(CPPFLAGS) -c kernel.c
		$(LD) $(LDFLAGS) -o $@ $(ASOBJS) kernel.o $(LIBS)
		@rm -f kernel.c

debug:		debug.c
		$(CC) -DSTANDALONE -DDEBUG -S $<
		$(CC) -DSTANDALONE -DDEBUG -c -o $@-sa.o $<
		$(LD) $(LDFLAGS) -o $@ $@-sa.o $(LIBS)
		pdp11-objdump -D $@ > $@.dis

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
rxboot2.o: rxboot2.S param.h
rxboot.o: rxboot.S
alloc.o: alloc.c param.h systm.h filsys.h buf.h inode.h user.h
bio.o: bio.c param.h user.h buf.h systm.h proc.h
clock.o: clock.c param.h systm.h user.h proc.h
debug.o: debug.c
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
