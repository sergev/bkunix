/*
 * set teletype modes
 */
#include <stdio.h>
#include <sgtty.h>

/*
 * tty flags
 */
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020

struct {
	char	*string;
	int	set;
	int	reset;
} modes[] = {
	"-nl",
	CRMOD, 0,

	"nl",
	0, CRMOD,

	"echo",
	ECHO, 0,

	"-echo",
	0, ECHO,

	"LCASE",
	LCASE, 0,

	"lcase",
	LCASE, 0,

	"-LCASE",
	0, LCASE,

	"-lcase",
	0, LCASE,

	0,
};

int
eq(arg, string)
	char *arg, *string;
{
	int i;

	i = 0;
	while (arg[i] == string[i]) {
		if (arg[i++] == '\0')
			return 1;
	}
	return 0;
}

void
prmodes(m)
	register int m;
{
	if (m & 020)
		printf("-nl ");
	if (m & 010)
		printf("echo ");
	if (m & 04)
		printf("lcase ");
	printf("\n");
}

int
main(argc, argv)
	char **argv;
{
	int i;
	char *arg;
	struct sgttyb mode;

	gtty(1, &mode);
	if (argc == 1) {
		prmodes(mode.sg_flags);
		return 0;
	}
	while (--argc > 0) {
		arg = *++argv;
		for(i = 0; modes[i].string; i++)
			if (eq(arg, modes[i].string)) {
				mode.sg_flags &= ~modes[i].reset;
				mode.sg_flags |= modes[i].set;
				arg = 0;
			}
		if (arg)
			printf("unknown mode: %s\n", arg);
	}
	stty(1, &mode);
	return 0;
}
