#include <string.h>
#include <unistd.h>
#include <mtab.h>
#include <a.out.h>

#define NMOUNT	2	/* look sys/param.h */
#define ROOTDEV	0

static char kernel[] = "/lsx";
static char mtabsym[] = "_mount";
static int *mtabptr;

static void
outerr(s)
	char *s;
{
	write(2, kernel, sizeof(kernel) - 1);
	write(2, ": ", 2);
	write(2, s, strlen(s));
}

int
openmtab()
{
	struct exec hdr;
	struct nlist sym;
	int fd, i;

	fd = open(kernel, 0);
	if (fd < 0) {
		outerr("cannot open\n");
		return 0;
	}
	if (read(fd, (char*) &hdr, sizeof(hdr)) != sizeof(hdr) ||
	    N_BADMAG(hdr)) {
		outerr("bad format\n");
		goto error;
	}
	if (hdr.a_syms == 0) {
		outerr("no name list\n");
		goto error;
	}
	if (seek(fd, N_RELOFF(hdr), 0) < 0) {
		outerr("seek error\n", kernel);
		goto error;
	}
	for (i=0; i<hdr.a_syms; i+=sizeof(sym)) {
		if (read(fd, (char*) &sym, sizeof(sym)) != sizeof(sym)) {
			outerr("error reading symbol table\n");
			goto error;
		}
		if (strncmp (sym.n_name, mtabsym, sizeof(sym.n_name)) == 0) {
			close(fd);
			mtabptr = (int*) sym.n_value;
/*printf("openmtab returned %#x\n", sym.n_value);*/
			return sym.n_value;
		}
	}
	outerr("symbol _mount not found\n");
error:	close(fd);
	return 0;
}

void
resetmtab(mt)
{
	mtabptr = (int*) mt;
}

int
readmtab(mt, fs_dev, fs_ronly, on_dev, on_ino)
	int *fs_dev, *fs_ronly, *on_dev, *on_ino;
{
	int *bufp, *inodp, *fs;

	while (mtabptr < (int*) (mt + NMOUNT * 6)) {
		/* device mounted */
		*fs_dev = *mtabptr++;

		/* pointer to superblock - struct buf */
		bufp = (int*) *mtabptr++;

		/* pointer to mounted on inode - struct inode */
		inodp = (int*) *mtabptr++;

/*printf("dev %#x bufp %p inodp %p\n", *fs_dev, bufp, inodp);*/
		if (! bufp)
			continue;

		fs = (int*) bufp[3];			/* b_addr */
		*fs_ronly = fs[205] >> 8;		/* s_ronly */
		if (*fs_dev == ROOTDEV) {
			*on_dev = ROOTDEV;
			*on_ino = 1;
		} else {
			*on_dev = inodp[1];		/* i_dev */
			*on_ino = inodp[2];		/* i_number */
		}
		return 1;
	}
	return -1;
}
