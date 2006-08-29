#include "screen.h"

/*
 * etc/init version to initialize tv terminal before running shell
 */

char cursor[12] {
	0,
	0176,
	0176,
	0176,
	0176,
	0176,
	0176,
	0176,
	00,
	00,
	00,
	0,
};
char shell[] "/bin/sh";
char minus[] "-";
char ctty[] "/dev/tty8";

main()
{
	register char *cp;
	int i;
	int fc;

	open(ctty, 2);
	dup(0);
	dup(0);
	cp = DISPLIST;
	while(cp < SCNCNTRL)
		*cp++ = 0;
	cp->fscroll = 0;
	cp->lscroll = NLINE-1;
	cp->chrloff = 0;
	cp->rstloff = 0;
	cp->curclnp = NLINE-1;
	cp->currloff = 0;
	cp->curhcpos = 0;
	cp->curhloff = 0;
	cp->grp1dis = 0;
	cp->grp1chr = 0;
	cp->grp2dis = 0;
	cp->grp2chr = 0;
	cp = &SCNCNTRL->curdefn[0];
	for(i = 0; i < 12; i++)
		*cp++ = cursor[i];
	if((fc = open("/etc/cset", 0)) < 0) {
		printf("can't open character set file\n");
		exit(1);
	}
	cp = CHARMEM;
	for(i = 0; i < 4; i++) {
		read(fc, cp, 512);
		cp =+ 512;
	}
	close(fc);
	chdir("/usr");
	execl(shell, minus, 0);
	exit();
}
