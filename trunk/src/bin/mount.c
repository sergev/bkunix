#define	NMOUNT	16
#define	NAMSIZ	32

struct mtab {
	char	file[NAMSIZ];
	char	spec[NAMSIZ];
} mtab[NMOUNT];

main(argc, argv)
char **argv;
{
	register int ro;
	register struct mtab *mp;
	register char *np;
	int n, mf;

	mf = open("/etc/mtab", 0);
	read(mf, mtab, NMOUNT*2*NAMSIZ);
	if (argc==1) {
		for (mp = mtab; mp < &mtab[NMOUNT]; mp++)
			if (mp->file[0])
				printf("%s on %s\n", mp->spec, mp->file);
		return;
	}
	if(argc < 3) {
		printf("arg count\n");
		return;
	}
	ro = 0;
	if(argc > 3)
		ro++;
	if(mount(argv[1], argv[2], ro) < 0) {
		perror("mount");
		return;
	}
	np = argv[1];
	while(*np++)
		;
	np--;
	while(*--np == '/')
		*np = '\0';
	while(np > argv[1] && *--np != '/')
		;
	if(*np == '/')
		np++;
	argv[1] = np;
	for (mp = mtab; mp < &mtab[NMOUNT]; mp++) {
		if (mp->file[0] == 0) {
			for (np = mp->spec; np < &mp->spec[NAMSIZ-1];)
				if ((*np++ = *argv[1]++) == 0)
					argv[1]--;
			for (np = mp->file; np < &mp->file[NAMSIZ-1];)
				if ((*np++ = *argv[2]++) == 0)
					argv[2]--;
			mp = &mtab[NMOUNT];
			while ((--mp)->file[0] == 0);
			mf = creat("/etc/mtab", 0644);
			write(mf, mtab, (mp-mtab+1)*2*NAMSIZ);
			return;
		}
	}
}
