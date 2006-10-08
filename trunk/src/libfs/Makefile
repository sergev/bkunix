#
# Unix V6 filesystem library.
#
DESTDIR		= /usr/local/lib/pdp11

# GNU compiler.
#CC             = pdp11-gcc -Wall
#CFLAGS         = -Os -fomit-frame-pointer -fno-builtin -I../libc

CC		= pdp11-pcc
AS		= pdp11-asm
AR		= pdp11-ar
CPP		= cpp
CFLAGS		= -O
CPPFLAGS	= -I../libc

SRCS		= fs_seek.c fs_open.c fs_close.c fs_sync.c \
		  fs_read.c fs_write.c fs_print.c \
		  fs_iget.c fs_isave.c fs_iclear.c \
		  fs_bread.c fs_bwrite.c fs_balloc.c fs_bfree.c

OBJS		= fs_seek.o fs_open.o fs_close.o fs_sync.o \
		  fs_read.o fs_write.o fs_print.o \
		  fs_iget.o fs_isave.o fs_iclear.o \
		  fs_bread.o fs_bwrite.o fs_balloc.o fs_bfree.o

all:		libfs.a

install:	all
		[ -d ${DESTDIR} ] || install -d ${DESTDIR}
		install libfs.a ${DESTDIR}/libfs.a

clean:
		rm -f *~ *.o a.out core libfs.a


libfs.a:		${OBJS}
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
