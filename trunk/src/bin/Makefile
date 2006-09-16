# GNU compiler.
#CC		= pdp11-gcc -Wall
#CFLAGS		= -Os -fomit-frame-pointer -fno-builtin -I../include
#LIBS		=

# Johnson's portable compiler.
CC		= pdp11-pcc
CFLAGS		= -I../include
LIBS		= -L/usr/local/lib/pdp11 -lpcc

# Using GNU binutils.
AS		= pdp11-as
LD		= pdp11-ld
SIZE		= pdp11-size -o
LDFLAGS		= --omagic

PROGS		= ls sh sync
# TODO: check cp date ed mkdir mv od rm rmdir size strip stty

all:		$(PROGS)

clean:
		rm -f *~ *.o

#sh:		sh.o
#		$(CC) $(LDFLAGS) $< -o $@

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
ls.o: ls.c
mv.o: mv.c
od.o: od.c
rm.o: rm.c
sh.o: sh.c ../include/unistd.h
size.o: size.c
stty.o: stty.c
sync.o: sync.c
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
