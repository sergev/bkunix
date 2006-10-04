/*
 * Free a double indirect block.
 */
int fs_bdifree (struct filesys *fs, unsigned int bno)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	if (! fs_read_block (fs, bno, data)) {
		fprintf (stderr, "inode_clear: read error at block %d\n", bno);
		return 0;
	}
	for (i=LSXFS_BSIZE-2; i>=0; i-=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			fs_indirect_block_free (fs, nb);
	}
	fs_block_free (fs, bno);
	return 1;
}
