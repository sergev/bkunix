#
/*
 * set teletype modes
 */

/*
 * tty flags
 */
#define	LCASE	04
#define	ECHO	010
#define	CRMOD	020
struct
{
	char	*string;
	int	set;
	int	reset;
} modes[]
{
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
char	*arg;
int	mode[3];

struct { char lobyte, hibyte; };

main(argc, argv)
char	*argv[];
{
	int i;

	gtty(1, mode);
	if(argc == 1) {
		prmodes();
		exit(0);
	}
	while(--argc > 0) {

		arg = *++argv;
		for(i = 0; modes[i].string; i++)
			if(eq(modes[i].string)) {
				mode[2] =& ~modes[i].reset;
				mode[2] =| modes[i].set;
			}
		if(arg)
			printf("unknown mode: %s\n", arg);
	}
	stty(1,mode);
}

eq(string)
char *string;
{
	int i;

	if(!arg)
		return(0);
	i = 0;
loop:
	if(arg[i] != string[i])
		return(0);
	if(arg[i++] != '\0')
		goto loop;
	arg = 0;
	return(1);
}

prmodes()
{
	register m;

	m = mode[2];
	if(m & 020) printf("-nl ");
	if(m & 010) printf("echo ");
	if(m & 04) printf("lcase ");
	printf("\n");
}

putchar(c)
{

	write(2, &c, 1);
}
