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
LIBS		= libstand.a -L/usr/local/lib/pdp11 -lcrt

# Using ported AT&T UNIX utils.
AS		= pdp11-asm
AR		= pdp11-ar
LD		= pdp11-ld
SIZE		= pdp11-size -o
FSUTIL		= u6-fsutil -F
LDFLAGS		= -X

SRCS		= start.S console.c printf.c strlen.c
OBJS		= console.o printf.o strlen.o
PROG		= fdfmt fdgetrom fdinfo hello #\
		  tstsh tstlsh tstmul tstdiv tstrem tstuldiv tstulrem \
		  tstlmul tstldiv tstlrem

CPPFLAGS	= -I. -DKERNEL
HIBASE		= 0120000

# For BK-0011M model.
#CPPFLAGS	+= -DBK0011
#HIBASE		= 0100000

all:		$(PROG)
		$(SIZE) $(PROG)

clean:
		rm -f *~ *.o *.s *.a *.dis a.out $(PROG)

libstand.a:	$(OBJS)
		@-rm -f $@
		$(AR) cru $@ $(OBJS)

hello:		start.o libstand.a hello.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o hello.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstsh:		start.o libstand.a tstsh.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstsh.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstlsh:		start.o libstand.a tstlsh.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstlsh.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstmul:		start.o libstand.a tstmul.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstmul.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstdiv:		start.o libstand.a tstdiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstdiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstrem:		start.o libstand.a tstrem.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstrem.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstuldiv:	start.o libstand.a tstuldiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstuldiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstulrem:	start.o libstand.a tstulrem.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstulrem.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstlmul:	start.o libstand.a tstlmul.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstlmul.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstldiv:	start.o libstand.a tstldiv.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstldiv.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

tstlrem:	start.o libstand.a tstlrem.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o tstlrem.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdgetrom:	start.o libstand.a fdgetrom.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o fdgetrom.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdinfo:		start.o libstand.a fdinfo.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o fdinfo.o $(LIBS)
		pdp11-disasm -a$(HIBASE) $@ > $@.dis

fdfmt:		start.o libstand.a fdfmt.o
		$(LD) $(LDFLAGS) -a$(HIBASE) -o $@ start.o fdfmt.o $(LIBS)
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
