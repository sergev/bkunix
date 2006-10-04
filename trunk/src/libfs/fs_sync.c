#include <unistd.h>
#include <sys/fs.h>

int
fs_sync (fs, force)
	struct filesys *fs;
	int force;
{
	if (! fs->writable)
		return 0;
	if (! force && ! fs->dirty)
		return 1;

	time (&fs->time);
	if (! fs_seek (fs, 512L))
		return 0;

printf ("**superblock write\n");
	if (write (fs->fd, (char*) fs, FS_SUPERB_SIZE) != FS_SUPERB_SIZE)
		return 0;

	fs->dirty = 0;
	return 1;
}
