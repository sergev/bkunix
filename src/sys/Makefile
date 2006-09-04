#
# Make LSI-UNIX system.
#
CC		= pdp11-gcc -Wall
AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
CFLAGS		= -Os -fomit-frame-pointer -DKERNEL -I.
LDFLAGS		= -nostdlib

ASRCS		= low.s mch.s emul.s
CSRCS		= alloc.c bio.c \
		  clock.c fio.c iget.c main.c nami.c rdwri.c sig.c slp.c \
		  subr.c sys1.c sys2.c sys3.c sys4.c sysent.c trap.c
OBJS		= low.o mch.o emul.o alloc.o bio.o \
		  clock.o fio.o iget.o main.o nami.o rdwri.o sig.o slp.o \
		  subr.o sys1.o sys2.o sys3.o sys4.o sysent.o trap.o \

vpath %.c . dev

# Parameter settings for for DEC RX11 floppy disks and LSI-11 processor.
ASFLAGS		= --defsym LSI=1 \
		  --defsym EIS=1 \
		  --defsym AED=0 \
		  --defsym DEC=1 \
		  --defsym IBMS=1 \
		  --defsym PER=0 \
		  --defsym SYK=0 \
		  --defsym RF=0 \
		  --defsym FLTVECT=0 \
		  --defsym KL=1 \
		  --defsym TVT=0 \
		  --defsym CLOCK=1 \
		  --defsym BGOPTION=0 \
		  --defsym RXROM=0
CSRCS		+= dev/kl.c dev/decfd.c
OBJS		+= kl.o decfd.o

# Parameter settings for for DEC RX11 floppy disks and
# non-LSI PDP-11 processor, e.g. 11/05.
ASFLAGS_DEC	= --defsym LSI=0 \
		  --defsym EIS=0 \
		  --defsym AED=0 \
		  --defsym DEC=1 \
		  --defsym IBMS=1 \
		  --defsym PER=0 \
		  --defsym SYK=0 \
		  --defsym RF=0 \
		  --defsym FLTVECT=0 \
		  --defsym KL=1 \
		  --defsym TVT=0 \
		  --defsym CLOCK=1 \
		  --defsym BGOPTION=0 \
		  --defsym RXROM=1 \
#CSRCS		+= dev/kl.c dev/decfd.c
#OBJS		+= kl.o decfd.o

# Parameter settings for for RF floppy disks and
# non-LSI PDP-11 processor, e.g. 11/05.
ASFLAGS_RF	= --defsym LSI=0 \
		  --defsym EIS=0 \
		  --defsym AED=0 \
		  --defsym DEC=0 \
		  --defsym IBMS=0 \
		  --defsym PER=0 \
		  --defsym SYK=0 \
		  --defsym RF=1 \
		  --defsym FLTVECT=0 \
		  --defsym KL=1 \
		  --defsym TVT=0 \
		  --defsym CLOCK=0 \
		  --defsym BGOPTION=0
#CSRCS		+= dev/kl.c dev/rffd.c
#OBJS		+= kl.o rffd.o

all:		lsx
		@echo "Warning: For 10K LSX system, size must be less than 046000"
		@echo "         for 8k lsx system size must be less than 036500"
		$(SIZE) lsx

clean:
		rm -f *~ dev/*~ *.o lsx a.out

lsx:		$(OBJS)
		$(LD) -o $@ $(OBJS)

count:		$(ASRCS) $(CSRCS)
		wc $(ASRCS) $(CSRCS)

.c.s:
		$(CC) $(CFLAGS) -S $<

###
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
sys1.o: sys1.c param.h systm.h user.h proc.h file.h buf.h reg.h inode.h
sys2.o: sys2.c param.h systm.h user.h reg.h file.h inode.h
sys3.o: sys3.c param.h systm.h reg.h buf.h filsys.h user.h inode.h file.h
sys4.o: sys4.c param.h user.h reg.h inode.h systm.h proc.h
sysent.o: sysent.c param.h systm.h
trap.o: trap.c param.h systm.h user.h proc.h reg.h
aedfd.o: dev/aedfd.c param.h buf.h user.h
decfd.o: dev/decfd.c param.h systm.h buf.h user.h
kl.o: dev/kl.c param.h user.h systm.h inode.h file.h reg.h tty.h proc.h
perfd.o: dev/perfd.c param.h buf.h user.h
rffd.o: dev/rffd.c param.h buf.h user.h
sykfd.o: dev/sykfd.c param.h buf.h user.h
tv.o: dev/tv.c param.h user.h systm.h inode.h file.h reg.h tty.h proc.h \
  dev/screen.h
