#define NBLKS	20

int	buf[256*NBLKS];

main(argc,argv)
char *argv[];
{
	int nread, nblks;
	char *kdump, *kore;
	int kfc, cfc;

	kdump = "/dev/tf13";
	kore = "kore";
	if((kfc = open(kdump,0)) < 0) {
		printf("can't read %s\n", kdump);
		exit();
	}
	if((cfc = creat(kore, 0666)) < 0) {
		printf("can't create %s\n", kore);
		exit();
	}
	seek(kfc, 42000, 3);
	setio(kfc, 1);
	setio(cfc, 1);
	nblks = 80;
	while(nblks > 0) {
		nread = NBLKS;
		if(nread > nblks)
			nread = nblks;
		nblks =- nread;
		nread = read(kfc, buf, nread*512);
		write(cfc, buf, nread);
	}
}
