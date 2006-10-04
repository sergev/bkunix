#include <unistd.h>
#include <sys/fs.h>

int
fs_isave (inode, force)
	struct inode *inode;
	int force;
{
	unsigned long offset;

	if (! inode->fs->writable)
		return 0;
	if (! force && ! inode->dirty)
		return 1;
	if (inode->number == 0 || inode->number > inode->fs->isize*16)
		return 0;
	offset = (inode->number + 31) * 32;

	time (&inode->atime);
	time (&inode->mtime);

	if (! fs_seek (inode->fs, offset))
		return 0;

	inode->size2 = inode->size >> 16;
	inode->size10 = inode->size;
printf ("**inode %d write\n", inode->number);
	if (write (inode->fs->fd, (char*) inode, FS_INODE_SIZE) != FS_INODE_SIZE)
		return 0;

	inode->dirty = 0;
	return 1;
}
