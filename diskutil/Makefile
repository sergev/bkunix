OBJS		= bkdisk.o
CC		= gcc

CXXFLAGS	= -Wall -g -O2

all:		bkdisk.exe

# Use MinGW (www.mingw.org)
bkdisk.exe:	$(OBJS)
		$(CC) -s -o $@ $(OBJS)

clean:
		rm -f *~ *.o core

###
bkdisk.o: bkdisk.cpp fdrawcmd.h
