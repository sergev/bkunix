#
# Copyright (c) 1987 Regents of the University of California.
# All rights reserved.  The Berkeley software License Agreement
# specifies the terms and conditions for redistribution.
#
#	@(#)Makefile	5.7.1 (2.11BSD GTE) 4/9/94
#
#
SRCS=	aldiv.s almul.s alrem.s csv.s ldiv.s lmul.s lrem.s udiv.s \
	uldiv.s ulrem.s ulsh.s ultof.s
OBJS=	aldiv.o almul.o alrem.o csv.o ldiv.o lmul.o lrem.o udiv.o \
	uldiv.o ulrem.o ulsh.o ultof.o

CFLAGS=	-O ${DEFS}
TAGSFILE=tags

.s.o:
	/lib/cpp -E -DPROF ${DEFS} $< | sed -e 's;^#;/;' | as -V -o $@
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	/lib/cpp -E ${DEFS} $< | sed -e 's;^#;/;' | as -V -o $@
	-ld -x -r $*.o
	mv a.out $*.o

.c.o:
	cc -p -c ${CFLAGS} $*.c
	-ld -X -r $*.o
	mv a.out profiled/$*.o
	cc -c ${CFLAGS} $*.c
	-ld -x -r $*.o
	mv a.out $*.o

crtlib crtlib_p: ${OBJS}
	@echo "building profiled crtlib"
	@cd profiled; ar cru ../crtlib_p ${OBJS}
	@echo "building normal crtlib"
	@ar cru crtlib ${OBJS}

tags:

clean:
	rm -f *.o profiled/*.o errs a.out core crtlib crtlib_p \
		tags Makefile.bak

depend:
	for i in ${SRCS}; do \
	    cc -M ${CFLAGS} $$i | awk ' { if ($$1 != prev) \
		{ if (rec != "") print rec; rec = $$0; prev = $$1; } \
		else { if (length(rec $$2) > 78) { print rec; rec = $$0; } \
		else rec = rec " " $$2 } } \
		END { print rec } ' >> makedep; done
	echo '/^# DO NOT DELETE THIS LINE/+2,$$d' >eddep
	echo '$$r makedep' >>eddep
	echo 'w' >>eddep
	cp Makefile Makefile.bak
	ed - Makefile < eddep
	rm eddep makedep
	echo '# DEPENDENCIES MUST END AT END OF FILE' >> Makefile
	echo '# IF YOU PUT STUFF HERE IT WILL GO AWAY' >> Makefile
	echo '# see make depend above' >> Makefile

# DO NOT DELETE THIS LINE -- make depend uses it

aldiv.o: aldiv.s ./DEFS.h
almul.o: almul.s ./DEFS.h
alrem.o: alrem.s ./DEFS.h
csv.o: csv.s ./DEFS.h
ldiv.o: ldiv.s ./DEFS.h
lmul.o: lmul.s ./DEFS.h
lrem.o: lrem.s ./DEFS.h
udiv.o: udiv.s ./DEFS.h
uldiv.o: uldiv.s ./DEFS.h
ulrem.o: ulrem.s ./DEFS.h
ulsh.o: ulsh.s ./DEFS.h
ultof.o: ultof.s ./DEFS.h
# DEPENDENCIES MUST END AT END OF FILE
# IF YOU PUT STUFF HERE IT WILL GO AWAY
# see make depend above
