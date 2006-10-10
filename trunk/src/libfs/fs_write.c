/*
 * Write data to disk.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <unistd.h>
#include <sys/fs.h>

int
fs_write (fs, data, bytes)
	struct filesys *fs;
	unsigned char *data;
	int bytes;
{
	int len;

	if (! fs->writable)
		return 0;
	while (bytes > 0) {
		len = bytes;
		if (len > 512)
			len = 512;
printf ("**write %d bytes at offset %ld\n", len, fs->seek);
		if (write (fs->fd, data, len) != len)
			return 0;
		fs->seek += len;
		data += len;
		bytes -= len;
	}
	return 1;
}
