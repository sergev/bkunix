# Base address of user programs.
BOTUSR		= 02000

AS		= pdp11-asm
LD		= pdp11-ld
SIZE		= pdp11-size -o
LDFLAGS		= -a$(BOTUSR)

ifdef LOWSTACK
LDFLAGS		+= -t1024
endif

PROGS		= as as2

all:		$(PROGS)

clean:
		rm -f *~ *.o *.dis a.out $(PROGS)

%: %.o
		$(LD) $(LDFLAGS) -o $@ $^
		pdp11-disasm -a$(BOTUSR) $@ > $@.dis

as.o: as.h as1?.s
		pdp11-asm -o $@ $^

as2.o: as.h as2?.s
		pdp11-asm -o $@	$^
