# Base address of user programs.
BOTUSR		= 02000

LD		= pdp11-ld
SIZE		= pdp11-size
CFLAGS		= -O -I../libc
LDFLAGS		= -a$(BOTUSR)
ifdef LOWSTACK
LDFLAGS		+= -t1024
endif

# Johnson's portable compiler.
CC		= pdp11-pcc

# Ritchie's compiler.
#CC		= pdp11-cc

PROGS		= reloc

all:		
		$(MAKE) -C as
		$(MAKE) $(PROGS)
		$(SIZE) $(PROGS)

clean:
		/bin/rm -f *~ *.o *.dis a.out $(PROGS)

%: %.o
		$(CC) $(LDFLAGS) -o $@ $^
		pdp11-disasm -a$(BOTUSR) $@ > $@.dis

.c.s:
		$(CC) $(CFLAGS) -S $<

depend:		clean
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CFLAGS) *.c |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
ar.o: ar.c
ld.o: ld.c
reloc.o: reloc.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
