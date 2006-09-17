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
AS		= pdp11-as
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
		${CPP} -E ${CPPFLAGS} $< | $(AS) -o $@
		@${STRIP} -x $*.o

depend:
		@cp Makefile Makefile~~
		(sed '/^### DO NOT DELETE THIS LINE/,$$d' Makefile;\
		echo '### DO NOT DELETE THIS LINE';\
		gcc -MM $(CPPFLAGS) *.S |\
		sed ':1;/\.o: .* \\/{;N;s/ *\\\n */ /;};s/ \/[^ ]*h */ /;t1';\
		echo '# DEPENDENCIES MUST END AT END OF FILE';\
		echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY';\
		echo '# see make depend above') > Makefile~ &&\
		mv Makefile~ Makefile

### DO NOT DELETE THIS LINE
crt0.o: crt0.S
exit.o: exit.S
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
