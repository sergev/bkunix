#include <unistd.h>
#include <sys/fs.h>

void
fs_close (fs)
	struct filesys *fs;
{
	if (fs->fd < 0)
		return;

	close (fs->fd);
	fs->fd = -1;
}
