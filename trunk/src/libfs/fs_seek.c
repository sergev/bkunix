#include <unistd.h>
#include <sys/fs.h>

int
fs_seek (fs, offset)
	struct filesys *fs;
	unsigned long offset;
{
	int r;

/*printf ("  seek %ld, block %ld\n", offset, offset >> 9);*/
#ifdef __pdp11__
	r = offset >> 9;
	r = seek (fs->fd, r, 3);
	if (r < 0)
		goto error;
	r = (int) offset & 0777;
	if (r != 0)
		r = seek (fs->fd, r, 1);
#else
	r = lseek (fs->fd, offset, 0);
#endif
	if (r < 0) {
error:		printf ("error seeking %ld, block %ld\n",
			offset, offset >> 9);
		return 0;
	}
	fs->seek = offset;
	return 1;
}
