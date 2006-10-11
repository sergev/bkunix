ETC		= etc/init etc/glob etc/mknod etc/mkfs etc/fsck
BIN		= bin/sh bin/ls bin/echo bin/cal bin/cp bin/date bin/mkdir \
		  bin/sync bin/mv bin/rm bin/rmdir bin/stty bin/od bin/ed \
		  bin/cat bin/ln bin/wc bin/pwd bin/df bin/mount bin/umount
SYS		= sys/lsx sys/boot1 sys/boot2lo
BKSYS		= sys/bsx sys/bkboot

DEV		= dev/tty8!c0:0 dev/fd0!b0:0 dev/fd1!b0:1
DSK		= root.dsk usr.dsk
FSUTIL		= u6-fsutil

all:		$(DSK)
		$(FSUTIL) -v root.dsk

clean:
		rm -f *~ $(DSK) lsx bsx lsx.ini
		$(MAKE) -C bin clean
		$(MAKE) -C etc clean
		$(MAKE) -C sys clean
		$(MAKE) -C libc clean
		$(MAKE) -C libfs clean

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

bkbuild:	$(BKSYS) $(ETC) $(BIN)

bkroot.dsk:	clean
		$(MAKE) BOTUSR=02000 LOWSTACK=1 bkbuild
		pdp11-strip $(ETC) $(BIN)
		$(FSUTIL) -F -n -s819200 -bsys/bkboot $@
		cp sys/bsx .
		$(FSUTIL) -F -a $@ bsx usr/ tmp/
		@rm -f bsx
		$(FSUTIL) -F -a $@ etc/ $(ETC)
		$(FSUTIL) -F -a $@ bin/ $(BIN)
		$(FSUTIL) -F -a $@ dev/ $(DEV)
		$(FSUTIL) -F -c $@

usr.dsk:
		$(FSUTIL) -n -s256000 $@
		$(FSUTIL) -c $@

$(SYS):
		$(MAKE) -C sys

$(BKSYS):
		$(MAKE) -C sys bsx bkboot

$(BIN):
		$(MAKE) -C bin

$(ETC):
		$(MAKE) -C etc

run:		lsx.ini root.dsk
		pdp11 lsx.ini

lsx.ini:
		echo set log log > $@
		echo set cpu 48k >> $@
		echo attach rx0 root.dsk >> $@
		echo attach rx1 usr.dsk >> $@
		echo set cpu history=2000 >> $@
		echo echo ===== Running boot: type '"lsx"' ===== >> $@
		echo boot rx0 >> $@
		echo echo When stopped, type: show cpu history >> $@
