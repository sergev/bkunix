#
struct inode
{
	int	i_dev;
	int	i_number;
	int	i_mode;
	char	i_nlink;
	char	i_uid;
	char	i_gid;
	char	i_size0;
	char	*i_size1;
	int	i_addr[8];
	int	i_time[4];
};

/* modes */
#define	IALLOC	0100000
#define	IFMT	070000
#define		IFDIR	040000
#define		IFCHR	020000
#define		IFBLK	060000
#define		IFREC	070000
#define ICONT	010000
#define	ILARG	010000
#define	ISUID	04000
#define	ISGID	02000
#define	IREAD	0400
#define	IWRITE	0200
#define	IEXEC	0100

int	fin;
int	fout;
int	fso;
int	dlvl;
char	*charp;
char	*dptr;
char	dirstr[100];
char	string[50];
char	*pspec;
char	*proto;

main(argc, argv)
char **argv;
{
	int i, c, j, x;
	char *a;

	/*
	 * open relevent files
	 */

	if(argc != 3) {
		printf("try: mkpt specification proto\n");
		exit();
	}
	pspec = argv[1];
	proto = argv[2];
	fin = open(pspec, 0);
	if(fin < 0) {
		printf("%s: cannot open\n", pspec);
		exit();
	}
	fso = creat(proto, 0666);
	if(fso < 0) {
		printf("%s: cannot create\n", proto);
		exit();
	}
	fout = fso;
	getstr();
	printf("%s\n",string);	/* boot file */
	getstr();
	printf("%s",string);	/* file system size */
	getstr();
	printf(" %s\n",string);	/* inode blocks */
	getstr();
	printf("%s",string);	/* root directory flags */
	getstr();
	printf(" %s",string);	/* root uid */
	getstr();
	printf(" %s\n",string);	/* root gid */
	while(c = getstr()) {
		if(string[c-1] == ':') {
			if(dlvl) {
				for(i = 0; i < dlvl; i++)
					printf("\t");
				printf("$\n");
				dlvl--;
			}
			string[c-1] = 0;
			if((c == 1) || (c == 2 && string[0] == '/'))
				continue;
			if(string[0] == '.' && string[1] == '.') {
				if(dlvl) {
					for(i = 0; i < dlvl; i++)
						printf("\t");
					printf("$\n");
					dlvl--;
				}
				continue;
			}
			for(x = i = 0; i < c; i++) {
				if(string[i] == '/') {
					string[i] = 0;
					if(dlvl)
						for(j = 0; j < dlvl; j++)
							printf("\t");
					printf("%s\td--777 3 1\n",&string[x]);
					x = i+1;
					dlvl++;
				}
			}
			if(dlvl)
				for(j = 0; j < dlvl; j++)
					printf("\t");
			printf("%s\td--777 3 1\n",&string[x]);
			dlvl++;
			continue;
		}
		dptr = dirstr;
		a = string;
		while(*a)
			*dptr++ = *a++;
		*dptr = 0;
		cfile(dirstr);
	}
	if(dlvl)
		printf("\t$\n");
	printf("$\n");
	flush();
}

cfile(dirp)
char *dirp;
{
	struct dirent {
		int ino;
		char name[14];
	}dbuf;
	struct inode in;
	int dir, i;
	char *a;

	if(chdir(dirp) < 0) {
		flush();
		fout = 2;
		printf("%s not a directory\n",dirstr);
		flush();
		fout = fso;
		return;
	}
	if((dir = open(dptr,0)) < 0) {
		flush();
		fout = 2;
		printf("can't open: %s\n",dirp);
		flush();
		fout = fso;
		return;
	}
	while(read(dir,&dbuf,16) > 0) {
		if(dbuf.ino == 0)
			continue;
		if(dbuf.name[0] == '.' && dbuf.name[1] == '\0')
			continue;
		if(dbuf.name[0] == '.' && dbuf.name[1] == '.' && dbuf.name[2] == '\0')
			continue;
		stat(dbuf.name,&in);
		for(i = 0; i < dlvl; i++)
			printf("\t");
		printf("%s\t",dbuf.name);
		if(in.i_mode&ISUID)
			string[1] = 'u';
		else
			string[1] = '-';
		if(in.i_mode&ISGID)
			string[2] = 'g';
		else
			string[2] = '-';
		string[3] = 060+((in.i_mode>>6)&07);
		string[4] = 060+((in.i_mode>>3)&07);
		string[5] = 060+(in.i_mode&07);
		switch(in.i_mode&IFMT) {
		case IFCHR:
			string[0] = 'c';
			break;
		case IFBLK:
			string[0] = 'b';
			break;
		case IFREC:
			string[0] = 'r';
			break;
		case IFDIR:
			string[0] = 'd';
			break;
		default:
		case 0:
			string[0] = '-';
		}
		string[6] = 0;
		printf("%s",string);
		printf(" %d %d",in.i_uid&0377,in.i_gid&0377);
		switch(in.i_mode&IFMT) {
		case IFCHR:
		case IFBLK:
		case IFREC:
			printf(" %d %d\n",(in.i_addr[0]>>8)&0377,in.i_addr[0]&0377);
			break;
		case IFDIR:
			printf("\n");
			dlvl++;
			a = dbuf.name;
			if(*(dptr-1) != '/')
				*dptr++ = '/';
			while(*a)
				*dptr++ = *a++;
			*dptr = 0;
			cfile(dbuf.name);
			for(i = 0; i < dlvl; i++)
				printf("\t");
			printf("$\n");
			dlvl--;
			break;
		default:
		case 0:
			printf(" %s/%s\n",dirstr,dbuf.name);
			break;
		}
	}
	close(dir);
	while(dptr > dirstr) {
		if(*--dptr == '/') {
			if(dptr == dirstr) {
				dptr++;
				break;
			} else {
				*dptr = 0;
				break;
			}
		}
		*dptr = 0;
	}
	chdir(dirstr);
}

getstr()
{
	int i, c;

loop:
	switch(c=getchar()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		return(0);

	}
	i = 0;

	do {
		string[i++] = c;
		c = getchar();
	} while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
	return(i);
}
