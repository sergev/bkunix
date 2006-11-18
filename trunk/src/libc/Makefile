#
# Compile standard C library.
#
#
DESTDIR		= /usr/local/lib/pdp11
CC		= pdp11-pcc
AS		= pdp11-asm
AR		= pdp11-ar
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	= -I.
ifdef LOWSTACK
CPPFLAGS	+= -DLOWSTACK=1
endif

SRCS		= crt0.S nullsys.S nosys.S \
		  cerror.S _exit.S fork.S read.S write.S \
		  open.S creat.S close.S wait.S link.S unlink.S \
		  execv.S execl.S chdir.S time.S mknod.S chmod.S sbrk.S \
		  stat.S lseek.S getpid.S getuid.S stime.S fstat.S \
		  stty.S gtty.S sync.S dup.S signal.S setjmp.S \
		  vprintf.c printf.c atoi.c ctime.c malloc.c calloc.c \
		  strcat.c strchr.c strcmp.c strcpy.c strlen.c \
		  strncat.c strncmp.c strncpy.c strrchr.c \
		  memccpy.c memchr.c memcmp.c memcpy.c memset.c qsort.c \
		  opendir.c closedir.c readdir.c isatty.c mkstemp.c \
		  readmtab.c mount.S umount.S sleep.S alarm.S pipe.c #execvp.c

OBJS		= nullsys.o nosys.o cerror.o _exit.o fork.o read.o write.o \
		  open.o creat.o close.o wait.o link.o unlink.o \
		  execv.o execl.o chdir.o time.o mknod.o chmod.o sbrk.o \
		  stat.o lseek.o getpid.o getuid.o stime.o fstat.o \
		  stty.o gtty.o sync.o dup.o signal.o setjmp.o \
		  vprintf.o printf.o atoi.o ctime.o malloc.o calloc.o \
		  strcat.o strchr.o strcmp.o strcpy.o strlen.o \
		  strncat.o strncmp.o strncpy.o strrchr.o \
		  memccpy.o memchr.o memcmp.o memcpy.o memset.o qsort.o \
		  opendir.o closedir.o readdir.o isatty.o mkstemp.o \
		  readmtab.o mount.o umount.o sleep.o alarm.o pipe.o #execvp.o

# stdio
SRCS		+= exit.c fprintf.c sprintf.c vfprintf.c flsbuf.c filbuf.c \
		   fopen.c fdopen.c freopen.c rdwr.c clrerr.c fgetc.c fgets.c \
	           fputc.c fputs.c fseek.c ftell.c getchar.c gets.c putchar.c \
                   puts.c rew.c setbuf.c ungetc.c

OBJS		+= exit.o fprintf.o sprintf.o vfprintf.o flsbuf.o filbuf.o \
		   fopen.o fdopen.o freopen.o rdwr.o clrerr.o fgetc.o fgets.o \
	           fputc.o fputs.o fseek.o ftell.o getchar.o gets.o putchar.o \
                   puts.o rew.o setbuf.o ungetc.o

all:		crt0.o libc.a

install:	all
		[ -d ${DESTDIR} ] || install -d ${DESTDIR}
		install crt0.o ${DESTDIR}/crt0.o
		install libc.a ${DESTDIR}/libc.a

clean:
		rm -f *~ sys/*~ *.o a.out core libc.a


libc.a:		${OBJS}
		@-rm -f $@
		${AR} cru $@ ${OBJS}

.S.o:
		${CC} ${CPPFLAGS} -c $<

.c.s:
		${CC} ${CPPFLAGS} ${CFLAGS} -S $<

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
alarm.o: alarm.S syscall.h
cerror.o: cerror.S
chdir.o: chdir.S syscall.h
chmod.o: chmod.S syscall.h
close.o: close.S syscall.h
creat.o: creat.S syscall.h
crt0.o: crt0.S syscall.h
dup.o: dup.S syscall.h
execl.o: execl.S syscall.h
execv.o: execv.S syscall.h
_exit.o: _exit.S syscall.h
fork.o: fork.S syscall.h
fstat.o: fstat.S syscall.h
getpid.o: getpid.S syscall.h
getuid.o: getuid.S syscall.h
gtty.o: gtty.S syscall.h
link.o: link.S syscall.h
mknod.o: mknod.S syscall.h
mount.o: mount.S syscall.h
nosys.o: nosys.S
nullsys.o: nullsys.S
open.o: open.S syscall.h
read.o: read.S syscall.h
sbrk.o: sbrk.S syscall.h
lseek.o: lseek.S syscall.h
setjmp.o: setjmp.S
signal.o: signal.S syscall.h
sleep.o: sleep.S syscall.h
stat.o: stat.S syscall.h
stime.o: stime.S syscall.h
stty.o: stty.S syscall.h
sync.o: sync.S syscall.h
time.o: time.S syscall.h
umount.o: umount.S syscall.h
unlink.o: unlink.S syscall.h
wait.o: wait.S syscall.h
write.o: write.S syscall.h
atoi.o: atoi.c stdlib.h ansidecl.h
calloc.o: calloc.c stdlib.h ansidecl.h
closedir.o: closedir.c dirent.h ansidecl.h
ctime.o: ctime.c time.h ansidecl.h string.h
isatty.o: isatty.c unistd.h ansidecl.h sgtty.h
malloc.o: malloc.c stdlib.h ansidecl.h unistd.h
memccpy.o: memccpy.c string.h ansidecl.h
memchr.o: memchr.c string.h ansidecl.h
memcmp.o: memcmp.c string.h ansidecl.h
memcpy.o: memcpy.c string.h ansidecl.h
memset.o: memset.c string.h ansidecl.h
mkstemp.o: mkstemp.c stdlib.h ansidecl.h string.h
opendir.o: opendir.c dirent.h ansidecl.h
printf.o: printf.c stdlib.h ansidecl.h stdarg.h
qsort.o: qsort.c stdlib.h ansidecl.h
readdir.o: readdir.c dirent.h ansidecl.h
readmtab.o: readmtab.c string.h ansidecl.h unistd.h mtab.h a.out.h
strcat.o: strcat.c string.h ansidecl.h
strchr.o: strchr.c string.h ansidecl.h
strcmp.o: strcmp.c string.h ansidecl.h
strcpy.o: strcpy.c string.h ansidecl.h
strlen.o: strlen.c string.h ansidecl.h
strncat.o: strncat.c string.h ansidecl.h
strncmp.o: strncmp.c string.h ansidecl.h
strncpy.o: strncpy.c string.h ansidecl.h
strrchr.o: strrchr.c string.h ansidecl.h
vprintf.o: vprintf.c stdlib.h ansidecl.h stdarg.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
