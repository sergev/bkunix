#
# Make LSI-UNIX system.
#
CC		= pdp11-gcc
AS		= pdp11-as
STRIP		= pdp11-strip
SIZE		= pdp11-size -o
CFLAGS		= -DKERNEL -I.
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
		$(CC) $(LDFLAGS) -o $@ $(OBJS) -L/usr/local/pdp11/lib/gcc/pdp11/4.1.1 -lgcc
		$(STRIP) $@

count:		$(ASRCS) $(CSRCS)
		wc $(ASRCS) $(CSRCS)
