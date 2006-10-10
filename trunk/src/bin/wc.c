/*
 * wc - line and word count.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

char buf[512+4];
long wordct;
long twordct;
long linect;
long tlinect;

void
diag(s)
	char *s;
{
	write(2, s, strlen(s));
}

int
main(argc, argv)
	char **argv;
{
	int i, token, fd;
	register char *p1, *p2;
	register int c;

	i = 1;
	do {
		if (argc <= 1)
			fd = 0;
		else if ((fd = open(argv[i], 0)) < 0) {
			diag(argv[i]);
			diag(": cannot open\n");
			continue;
		}
		p1 = 0;
		p2 = 0;
		linect = 0;
		wordct = 0;
		token = 0;
		for (;;) {
			if (p1 >= p2) {
				c = read(fd, buf, 512);
				if (c <= 0)
					break;
				p1 = buf;
				p2 = buf + c;
			}
			c = *p1++;
			if (' ' < c && c < 0177) {
				if (! token++)
					wordct++;
			} else {
				if (c == '\n')
					linect++;
				else if (c != ' ' && c != '\t')
					continue;
				token = 0;
			}
		}
		printf("%7ld %7ld %s\n", linect, wordct,
			argc <= 1 ? "" : argv[i]);
		close(fd);
		tlinect += linect;
		twordct += wordct;
	} while (++i<argc);

	if (argc > 2)
		printf("%7ld %7ld total\n", tlinect, twordct);
	return 0;
}
