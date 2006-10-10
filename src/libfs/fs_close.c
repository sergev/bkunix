/*
 * Close filesystem.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
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
