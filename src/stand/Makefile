#
# Build standalone programs.
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
LIBS		= -L/usr/local/lib/pdp11 -lcrt

# Using ported AT&T UNIX utils.
AS		= pdp11-asm
LD		= pdp11-ld
SIZE		= pdp11-size -o
FSUTIL		= u6-fsutil -F
LDFLAGS		= -X

SRCS		= start.S console.c
OBJS		= start.o console.o
PROG		= hello tstsh tstlsh tstmul tstdiv tstrem tstuldiv tstulrem \
		  tstlmul tstldiv fdgetrom fdinfo fdfmt

CPPFLAGS	= -I. -DKERNEL

HIBASE		= 0120000

all:		$(PROG)
		$(SIZE) $(PROG)

clean:
		rm -f *~ *.o *.s *.dis a.out $(PROG)

hello:		$(OBJS) hello.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) hello.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstsh:		$(OBJS) tstsh.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstsh.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstlsh:		$(OBJS) tstlsh.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstlsh.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstmul:		$(OBJS) tstmul.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstmul.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstdiv:		$(OBJS) tstdiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstdiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstrem:		$(OBJS) tstrem.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstrem.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstuldiv:	$(OBJS) tstuldiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstuldiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstulrem:	$(OBJS) tstulrem.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstulrem.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstlmul:	$(OBJS) tstlmul.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstlmul.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstldiv:	$(OBJS) tstldiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) tstldiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdgetrom:	$(OBJS) fdgetrom.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) fdgetrom.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdinfo:		$(OBJS) fdinfo.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) fdinfo.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdfmt:		$(OBJS) fdfmt.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ $(OBJS) fdfmt.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

install:	$(PROG)	../root.bkd
		pdp11-strip $(PROG)
		$(FSUTIL) -a ../root.bkd $(PROG)

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
start.o: start.S
console.o: console.c
fdfmt.o: fdfmt.c
fdgetrom.o: fdgetrom.c
fdinfo.o: fdinfo.c
hello.o: hello.c
tstdiv.o: tstdiv.c
tstlsh.o: tstlsh.c
tstmul.o: tstmul.c
tstrem.o: tstrem.c
tstsh.o: tstsh.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
