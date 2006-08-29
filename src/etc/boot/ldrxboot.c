# 
/* This program is used to load the second level floppy bootstrap
 * into floppy blocks 0 (minor blocks 1 - 3) and 500 (minor blocks 0 - 1).
 * The first level bootstrap is located in rxboot.s (/etc/rxboot).
 * The second level bootstrap is uboot.s with the rx flag set and is usually
 * kept in /usr/util/rxboot2.
 * usage : ldrxboot 0or1 file
 */

#define FLOPBLK 128
int	buf[512];
char	floppy[]	"/dev/fdX";

main(argc, argv)
int argc;
char *argv[];
{
	register int i,j,k;

	if (argc != 3) {
		printf("USAGE: ldrxboot 0or1 file\n");
		exit(1);
	}
	if ((i = open(argv[2], 0)) == -1) {
		printf("can`t open: %s\n", argv[2]);
		exit(1);
	}
	floppy[7] = *argv[1];
	if ((j = open(floppy, 1)) == -1) {
		printf("can`t open: %s\n", floppy);
		exit(1);
	}
	if (read(i, buf, 020) != 020) {
		printf("can`t read: %s\n", argv[2]);
		exit(1);
	}
	if(buf[0] != 0407) {
		printf("bad format: %s\n", argv[2]);
		exit(1);
	}
	k = buf[1] + buf[2];
	if (k > 5 * FLOPBLK) {
		printf("%s: too big!\n", argv[2]);
		exit(1);
	}
	if (read(i, buf, k) != k) {
		printf("can`t read: %s\n", argv[2]);
		exit(1);
	}
	seek(j, FLOPBLK, 0);
	if (write(j, buf, 3 * FLOPBLK) != 3 * FLOPBLK) {
		printf("can`t write: %s\n", argv[2]);
		exit(1);
	}
	seek( j, 500, 3 );
	if( write( j, &buf[3 * FLOPBLK/2 ] , 2 * FLOPBLK) != 2 * FLOPBLK )
		printf("can't write %s\n", argv[2]);

	printf("done!\n");
}
