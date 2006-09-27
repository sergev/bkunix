#
# Make LSI-UNIX system.
#

# GNU compiler.
#CC		= pdp11-gcc -Wall
#CFLAGS		= -Os -fomit-frame-pointer -fno-builtin -I. -DKERNEL -DDEBUG
#LIBS		=

# Johnson's portable compiler.
CC		= pdp11-pcc -O
CFLAGS		= -I. -DKERNEL
LIBS		= -L/usr/local/lib/pdp11 -lcrt

# Using GNU binutils.
AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
LDFLAGS		= -X

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

# System configuration.
# -DBGOPTION  - background processes
# -DCLOCK     - set to be 0177546 or 0172540 for the line
#		frequency or programmable clock, respectively.
#
#CPPFLAGS	+= -DCLOCK=0177540

# Configuration of device drivers.
# -DAED	 - AED 6200lp floppy
# -DDEC  - DEC floppy
# -DPER  - PER floppy
# -DSYK  - Sykes floppy
# -DRF   - RF floppy
# -DKL   - KL/DL-11 console
# -DTVT  - TV terminal
#
CPPFLAGS	+= -DDEC -DKL
CSRCS		+= dev/kl.c dev/decfd.c
OBJS		+= kl.o decfd.o

all:		lsx #lsx1
		@echo "Warning: For 10K LSX system, size must be less than 046000"
		@echo "         for 8k lsx system size must be less than 036500"
		$(SIZE) lsx #lsx1

clean:
		rm -f *~ dev/*~ *.o *.s lsx lsx1 debug a.out lsx.dis debug.dis

lsx:		$(OBJS)
		$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)
		pdp11-disasm $@ > $@.dis

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
		$(CC) $(CFLAGS) -S $<

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
low.o: low.S
mch.o: mch.S param.h
alloc.o: alloc.c param.h systm.h filsys.h buf.h inode.h user.h
bio.o: bio.c param.h user.h buf.h systm.h proc.h
clock.o: clock.c param.h systm.h user.h proc.h
debug1.o: debug1.c
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
