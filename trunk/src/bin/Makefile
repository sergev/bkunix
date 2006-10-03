# Base address of user programs.
TOPSYS		= 040000

AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
CFLAGS		= -O -I../libc
LDFLAGS		= -a$(TOPSYS)

# GNU compiler.
#CC		= pdp11-gcc -Wall -fomit-frame-pointer -fno-builtin
#LDFLAGS		=

# Johnson's portable compiler.
CC		= pdp11-pcc

# Ritchie's compiler.
#CC		= pdp11-cc

PROGS		= sync echo cal ls sh cp date mkdir mv rm rmdir stty
# TODO: od ed

all:		$(PROGS)

clean:
		/bin/rm -f *~ *.o *.dis a.out $(PROGS)

%: %.o
		$(CC) $(LDFLAGS) -o $@ $^
		pdp11-disasm -a$(TOPSYS) $@ > $@.dis

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
cal.o: cal.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h
check.o: check.c
cp.o: cp.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/string.h ../libc/unistd.h ../libc/fcntl.h ../libc/sys/types.h ../libc/sys/stat.h
cpfp.o: cpfp.c
date.o: date.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h ../libc/time.h
echo.o: echo.c ../libc/stdio.h ../libc/ansidecl.h
ed.o: ed.c ../libc/signal.h ../libc/ansidecl.h ../libc/setjmp.h
hello.o: hello.c ../libc/unistd.h ../libc/ansidecl.h
ls.o: ls.c ../libc/ansidecl.h ../libc/stdlib.h ../libc/stdio.h ../libc/string.h ../libc/unistd.h ../libc/fcntl.h ../libc/time.h ../libc/dirent.h ../libc/sys/types.h ../libc/sys/stat.h
mkdir.o: mkdir.c ../libc/stdio.h ../libc/ansidecl.h ../libc/string.h ../libc/unistd.h ../libc/signal.h ../libc/sys/stat.h
mv.o: mv.c
od.o: od.c
rm.o: rm.c
rmdir.o: rmdir.c ../libc/sys/types.h ../libc/sys/stat.h ../libc/stdio.h
sh.o: sh.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h ../libc/signal.h ../libc/setjmp.h
stty.o: stty.c
sync.o: sync.c ../libc/unistd.h ../libc/ansidecl.h
testarith.o: testarith.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h
testprintf.o: testprintf.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h ../libc/string.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
