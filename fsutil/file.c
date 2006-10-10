/*
 * File i/o routines for unix v6 filesystem.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <string.h>
#include "u6fs.h"

extern int verbose;

int u6fs_file_create (u6fs_t *fs, u6fs_file_t *file, char *name, int mode)
{
	if (! u6fs_inode_by_name (fs, &file->inode, name, 1, mode)) {
		fprintf (stderr, "%s: inode open failed\n", name);
		return 0;
	}
	if ((file->inode.mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Cannot open directory on write. */
		return 0;
	}
	u6fs_inode_truncate (&file->inode);
	u6fs_inode_save (&file->inode, 0);
	file->writable = 1;
	file->offset = 0;
	return 1;
}

int u6fs_file_open (u6fs_t *fs, u6fs_file_t *file, char *name, int wflag)
{
	if (! u6fs_inode_by_name (fs, &file->inode, name, 0, 0)) {
		fprintf (stderr, "%s: inode open failed\n", name);
		return 0;
	}
	if (wflag && (file->inode.mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Cannot open directory on write. */
		return 0;
	}
	file->writable = wflag;
	file->offset = 0;
	return 1;
}

int u6fs_file_read (u6fs_file_t *file, unsigned char *data, unsigned long bytes)
{
	if (! u6fs_inode_read (&file->inode, file->offset, data, bytes)) {
		fprintf (stderr, "inode %d: file write failed\n",
			file->inode.number);
		return 0;
	}
	file->offset += bytes;
	return 1;
}

int u6fs_file_write (u6fs_file_t *file, unsigned char *data, unsigned long bytes)
{
	if (! file->writable)
		return 0;
	if (! u6fs_inode_write (&file->inode, file->offset, data, bytes)) {
		fprintf (stderr, "inode %d: file write failed\n",
			file->inode.number);
		return 0;
	}
	file->offset += bytes;
	return 1;
}

int u6fs_file_close (u6fs_file_t *file)
{
	if (file->writable) {
		if (! u6fs_inode_save (&file->inode, 0)) {
			fprintf (stderr, "inode %d: file close failed\n",
				file->inode.number);
			return 0;
		}
	}
	return 1;
}
