ETC		= etc/init
BIN		= bin/sh bin/ls bin/echo bin/cal bin/cp bin/date bin/mkdir \
		  bin/sync bin/mv bin/rm bin/rmdir bin/stty bin/od bin/ed \
		  bin/cat bin/ln bin/wc bin/pwd
SYS		= sys/lsx sys/boot1 sys/boot2lo

DEV		= dev/tty8!c0:0 dev/fd0!b0:0 dev/fd1!b0:1
DSK		= root.dsk usr.dsk
FSUTIL		= ../fsutil/lsx-util

all:		$(DSK)
		$(FSUTIL) -v root.dsk

clean:
		rm -f *~ $(DSK) lsx

root.dsk:	$(SYS) $(ETC) $(BIN)
		pdp11-strip $(ETC) $(BIN)
		$(FSUTIL) -n -s256000 -bsys/boot1 -Bsys/boot2lo $@
		cp sys/lsx .
		$(FSUTIL) -a $@ lsx usr/ tmp/
		@rm -f lsx
		$(FSUTIL) -a $@ etc/ $(ETC)
		$(FSUTIL) -a $@ bin/ $(BIN)
		$(FSUTIL) -a $@ dev/ $(DEV)
		$(FSUTIL) -c $@

usr.dsk:
		$(FSUTIL) -n -s256000 $@
		$(FSUTIL) -c $@

$(SYS):
		$(MAKE) -C sys

$(BIN):
		$(MAKE) -C bin

$(ETC):
		$(MAKE) -C etc
