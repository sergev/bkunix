# Base address of user programs.
BOTUSR		= 02000

AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size
CFLAGS		= -O -I../libc
LDFLAGS		= -a$(BOTUSR)
ifdef LOWSTACK
LDFLAGS		+= -t1024
endif

# GNU compiler.
#CC		= pdp11-gcc -Wall -fomit-frame-pointer -fno-builtin
#LDFLAGS		=

# Johnson's portable compiler.
CC		= pdp11-pcc

# Ritchie's compiler.
#CC		= pdp11-cc

PROGS		= sync echo cal ls sh cp date mkdir mv rm rmdir stty \
		  od ed cat ln wc pwd df mount umount clock halt

all:		$(PROGS)
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
cal.o: cal.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h
cat.o: cat.c ../libc/stdio.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/fcntl.h ../libc/sys/types.h ../libc/sys/stat.h
clock.o: clock.c ../libc/time.h ../libc/ansidecl.h ../libc/signal.h
cp.o: cp.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/string.h \
  ../libc/unistd.h ../libc/fcntl.h ../libc/sys/types.h ../libc/sys/stat.h
date.o: date.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/time.h
df.o: df.c ../libc/stdio.h ../libc/ansidecl.h ../libc/string.h \
  ../libc/unistd.h ../libc/dirent.h ../libc/mtab.h ../libc/sys/stat.h \
  ../libc/sys/fs.h
echo.o: echo.c ../libc/stdio.h ../libc/ansidecl.h
ed.o: ed.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/signal.h ../libc/fcntl.h ../libc/sgtty.h ../libc/setjmp.h \
  ../libc/sys/wait.h
fcp.o: fcp.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h \
  ../libc/string.h ../libc/unistd.h ../libc/fcntl.h ../libc/sys/types.h \
  ../libc/sys/stat.h
halt.o: halt.c ../libc/unistd.h ../libc/ansidecl.h
hello.o: hello.c ../libc/stdio.h ../libc/ansidecl.h
ln.o: ln.c ../libc/unistd.h ../libc/ansidecl.h ../libc/sys/stat.h
ls.o: ls.c ../libc/ansidecl.h ../libc/stdlib.h ../libc/stdio.h \
  ../libc/string.h ../libc/unistd.h ../libc/fcntl.h ../libc/time.h \
  ../libc/dirent.h ../libc/sys/types.h ../libc/sys/stat.h
mkdir.o: mkdir.c ../libc/stdio.h ../libc/ansidecl.h ../libc/string.h \
  ../libc/unistd.h ../libc/signal.h ../libc/sys/stat.h
mount.o: mount.c ../libc/stdio.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/dirent.h ../libc/mtab.h ../libc/sys/stat.h
mv.o: mv.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h \
  ../libc/unistd.h ../libc/sys/stat.h ../libc/sys/wait.h
od.o: od.c ../libc/stdio.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/fcntl.h
pwd.o: pwd.c ../libc/stdio.h ../libc/ansidecl.h ../libc/stdlib.h \
  ../libc/unistd.h ../libc/sys/stat.h ../libc/dirent.h
rm.o: rm.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h \
  ../libc/unistd.h ../libc/sys/stat.h ../libc/sys/wait.h
rmdir.o: rmdir.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/stdio.h \
  ../libc/string.h ../libc/unistd.h ../libc/fcntl.h ../libc/dirent.h \
  ../libc/sys/types.h ../libc/sys/stat.h
sh.o: sh.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h \
  ../libc/signal.h ../libc/setjmp.h ../libc/sys/wait.h
stty.o: stty.c ../libc/stdio.h ../libc/ansidecl.h ../libc/sgtty.h
sync.o: sync.c ../libc/unistd.h ../libc/ansidecl.h
testarith.o: testarith.c ../libc/stdlib.h ../libc/ansidecl.h \
  ../libc/stdio.h
testprintf.o: testprintf.c ../libc/stdlib.h ../libc/ansidecl.h \
  ../libc/stdio.h ../libc/string.h
umount.o: umount.c ../libc/string.h ../libc/ansidecl.h ../libc/unistd.h
wc.o: wc.c ../libc/stdio.h ../libc/ansidecl.h ../libc/string.h \
  ../libc/unistd.h ../libc/fcntl.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
