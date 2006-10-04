# Base address of user programs.
TOPSYS		= 040000

# GNU compiler.
#CC		= pdp11-gcc -Wall
#CFLAGS		= -Os -fomit-frame-pointer -fno-builtin -I../libc

# Johnson's portable compiler.
CC		= pdp11-pcc

# Ritchie's compiler.
#CC		= pdp11-cc

# Using BSD cross-compiler.
AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
CFLAGS		= -O -I../libc
LDFLAGS		= -a$(TOPSYS)

PROGS		= init glob mknod mkfs
# TODO: fsck

all:		$(PROGS)

clean:
		rm -f *~ *.o *.dis a.out $(PROGS)

%: %.o
		$(CC) $(LDFLAGS) -o $@ $^
		pdp11-disasm -a$(TOPSYS) $@ > $@.dis

mkfs:		mkfs.o ../libfs/libfs.a
		$(CC) $(LDFLAGS) -v -o $@ $^ ../libfs/libfs.a
		pdp11-disasm -a$(TOPSYS) $@ > $@.dis

../libfs/libfs.a:
		$(MAKE) -C ../libfs
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
clri.o: clri.c ../libc/sys/types.h
glob.o: glob.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h ../libc/dirent.h
init.o: init.c ../libc/unistd.h ../libc/ansidecl.h
kdmp.o: kdmp.c
mkfs.o: mkfs.c ../libc/stdio.h ../libc/ansidecl.h ../libc/stdlib.h ../libc/string.h ../libc/time.h ../libc/unistd.h ../libc/fcntl.h ../libc/sys/fs.h ../libc/sys/stat.h
mknod.o: mknod.c ../libc/stdlib.h ../libc/ansidecl.h ../libc/unistd.h ../libc/sys/stat.h
mkpt.o: mkpt.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
