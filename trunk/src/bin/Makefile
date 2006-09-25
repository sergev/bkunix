# GNU compiler.
#CC		= pdp11-gcc -Wall
#CFLAGS		= -Os -fomit-frame-pointer -fno-builtin -I../libc

# Johnson's portable compiler.
CC		= pdp11-pcc
CFLAGS		= -I../libc

# Using GNU binutils.
AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
LDFLAGS		= -a$(TOPSYS) -X /usr/local/lib/pdp11/crt0.o
LIBS		= -L/usr/local/lib/pdp11 -lc -lcrt

TOPSYS		= 040000
PROGS		= hello sync #ls sh
# TODO: check cp date ed mkdir mv od rm rmdir size strip stty

all:		$(PROGS)

clean:
		rm -f *~ *.o a.out $(PROGS)

%: %.o
		$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
		pdp11-disasm -b$(TOPSYS) $@ > $@.dis

.c.s:
		$(CC) $(CFLAGS) -S $<
depend:
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
check.o: check.c
cp.o: cp.c
cpfp.o: cpfp.c
date.o: date.c
ed.o: ed.c
hello.o: hello.c ../libc/stdlib.h ../libc/ansidecl.h
ls.o: ls.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/string.h ../libc/unistd.h ../libc/time.h
mkdir.o: mkdir.c
mv.o: mv.c
od.o: od.c
rm.o: rm.c
rmdir.o: rmdir.c
sh.o: sh.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h
size.o: size.c
strip.o: strip.c ../libc/a.out.h
stty.o: stty.c
sync.o: sync.c ../libc/unistd.h ../libc/ansidecl.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
