#	Makefile	1.3	(2.11BSD) 96/7/11
#

# Defined SHARED_DATA to be whatever will make your compiler make initialized
# variables shared and read only.  Don't do this on a PDP-11 if you're also
# compiling separate I&D since doing so puts the shared data in text space
# which can't be accessed normally.
#
#SHARED_DATA=-R

# CFLAGS=-O -Dunix=1 -DFLEXNAMES
# If you don't have split I/D, you'll have to edit the constant
# SBSIZE in cpp.c to reflect this.

CFLAGS=-O -Dpdp11=1 -DFLEXNAMES -DBSD2_11 -UBSD2_10
SEPFLAG=-i

cpp :	cpp.o cpy.o rodata.o
	$(CC) $(CFLAGS) $(SEPFLAG) -o cpp rodata.o cpp.o cpy.o

cpy.c : cpy.y
	yacc cpy.y
	../pcc/:yyfix yyexca yyact yypact yypgo yyr1 yyr2 yychk yydef
	mv y.tab.c cpy.c

cpy.o :	cpy.c yylex.c

rodata.o : cpy.c
	$(CC) $(CFLAGS) ${SHARED_DATA} -c rodata.c

clean :
	rm -f *.s *.o cpy.c rodata.c cpp

install : cpp
	install -s cpp $(DESTDIR)/lib

sources : cpp.c cpy.y yylex.c README
cpp.c cpy.y yylex.c README :
	sccs get $@
