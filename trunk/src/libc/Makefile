#
# Compile standard C library.
#
# System calls alarm() and pause() are not supported.
# It seems that nobody uses them. The sources are lost.
# The only way to restore them is extracting from old libc.a
# and disassembling.
#
DESTDIR		= /usr/local/lib/pdp11
CC		= pdp11-pcc
AS		= pdp11-asm
STRIP		= pdp11-strip
RANLIB		= pdp11-ranlib
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	= -I.

#qsort.c
SRCS		= crt0.S cerror.S exit.S fork.S read.S write.S \
		  open.S creat.S close.S wait.S link.S unlink.S \
		  execv.S execl.S chdir.S time.S mknod.S chmod.S sbrk.S \
		  stat.S seek.S getpid.S getuid.S stime.S fstat.S \
		  stty.S gtty.S sync.S dup.S signal.S \
		  vprintf.c printf.c atoi.c ctime.c \
		  strcat.c strchr.c strcmp.c strcpy.c strlen.c \
		  strncat.c strncmp.c strncpy.c strrchr.c \
		  memccpy.c memchr.c memcmp.c memcpy.c memset.c
#qsort.o
OBJS		= cerror.o exit.o fork.o read.o write.o \
		  open.o creat.o close.o wait.o link.o unlink.o \
		  execv.o execl.o chdir.o time.o mknod.o chmod.o sbrk.o \
		  stat.o seek.o getpid.o getuid.o stime.o fstat.o \
		  stty.o gtty.o sync.o dup.o signal.o \
		  vprintf.o printf.o atoi.o ctime.o \
		  strcat.o strchr.o strcmp.o strcpy.o strlen.o \
		  strncat.o strncmp.o strncpy.o strrchr.o \
		  memccpy.o memchr.o memcmp.o memcpy.o memset.o

all:		crt0.o libc.a

install:	all
		[ -d ${DESTDIR} ] || install -d ${DESTDIR}
		install crt0.o ${DESTDIR}/crt0.o
		install libc.a ${DESTDIR}/libc.a
		${RANLIB} ${DESTDIR}/libc.a

clean:
		rm -f *~ *.o a.out core libc.a


libc.a:		${OBJS}
		ar cru libc.a ${OBJS}

.S.o:
		${CC} ${CPPFLAGS} -c $<
		@${STRIP} -x $*.o

depend:
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CPPFLAGS) *.S *.c |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
cerror.o: cerror.S
chdir.o: chdir.S syscall.h
chmod.o: chmod.S syscall.h
close.o: close.S syscall.h
creat.o: creat.S syscall.h
crt0.o: crt0.S syscall.h
dup.o: dup.S syscall.h
execl.o: execl.S syscall.h
execv.o: execv.S syscall.h
exit.o: exit.S syscall.h
fork.o: fork.S syscall.h
fstat.o: fstat.S syscall.h
getpid.o: getpid.S syscall.h
getuid.o: getuid.S syscall.h
gtty.o: gtty.S syscall.h
link.o: link.S syscall.h
mknod.o: mknod.S syscall.h
open.o: open.S syscall.h
read.o: read.S syscall.h
sbrk.o: sbrk.S syscall.h
seek.o: seek.S syscall.h
signal.o: signal.S syscall.h
stat.o: stat.S syscall.h
stime.o: stime.S syscall.h
stty.o: stty.S syscall.h
sync.o: sync.S syscall.h
time.o: time.S syscall.h
unlink.o: unlink.S syscall.h
wait.o: wait.S syscall.h
write.o: write.S syscall.h
atoi.o: atoi.c stdlib.h
ctime.o: ctime.c stdlib.h
memccpy.o: memccpy.c string.h
memchr.o: memchr.c string.h
memcmp.o: memcmp.c string.h
memcpy.o: memcpy.c string.h
memset.o: memset.c string.h
printf.o: printf.c stdlib.h
qsort-211.o: qsort-211.c stdlib.h
qsort.o: qsort.c stdlib.h
strcat.o: strcat.c string.h
strchr.o: strchr.c string.h
strcmp.o: strcmp.c string.h
strcpy.o: strcpy.c string.h
strlen.o: strlen.c string.h
strncat.o: strncat.c string.h
strncmp.o: strncmp.c string.h
strncpy.o: strncpy.c string.h
strrchr.o: strrchr.c string.h
vprintf.o: vprintf.c stdlib.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
