#include <unistd.h>
#include <sys/fs.h>

/*
 * Get a block from free list.
 */
int
fs_balloc (fs, bno)
	struct filesys *fs;
	unsigned int *bno;
{
	int i;
	unsigned short buf [256];
again:
	if (fs->nfree == 0)
		return 0;
	fs->nfree--;
	*bno = fs->free [fs->nfree];
/*	printf ("allocate new block %d from slot %d\n", *bno, fs->nfree);*/
	fs->free [fs->nfree] = 0;
	fs->dirty = 1;
	if (fs->nfree <= 0) {
		if (! fs_bread (fs, *bno, (unsigned char*) buf))
			return 0;
		fs->nfree = buf[0];
		for (i=0; i<100; i++)
			fs->free[i] = buf[i+1];
	}
	if (*bno == 0)
		goto again;
	return 1;
}
