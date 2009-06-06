/*
 * Utility for dealing with unix v6 filesystem images.
 *
 * Copyright (C) 2006 Serge Vakulenko, <vak@cronyx.ru>
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the GNU General Public License (GPL).
 * See the accompanying file "COPYING" for more details.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <getopt.h>
#include "u6fs.h"

int verbose;
int extract;
int add;
int newfs;
int check;
int fix;
int flat;
unsigned long bytes;
char *boot_sector;
char *boot_sector2;

static const char *program_version =
	"LSX file system utility, version 1.1\n"
	"Copyright (C) 2002-2009 Serge Vakulenko";

static const char *program_bug_address = "<serge@vak.ru>";

static struct option program_options[] = {
	{ "help",	no_argument,		0,	'h' },
	{ "version",	no_argument,		0,	'V' },
	{ "verbose",	no_argument,		0,	'v' },
	{ "add",	no_argument,		0,	'a' },
	{ "extract",	no_argument,		0,	'x' },
	{ "check",	no_argument,		0,	'c' },
	{ "fix",	no_argument,		0,	'f' },
	{ "new",	no_argument,		0,	'n' },
	{ "size",	required_argument,	0,	's' },
	{ "boot",	required_argument,	0,	'b' },
	{ "boot2",	required_argument,	0,	'B' },
	{ "flat",	no_argument,		0,	'F' },
	{ 0 }
};

static void print_help (char *progname)
{
	printf ("%s\n", program_version);
	printf ("This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
		"see the GNU General Public License for more details.\n");
	printf ("\n");
	printf ("Usage:\n");
	printf ("    %s filesys.bkd\n", progname);
	printf ("    %s --add filesys.bkd files...\n", progname);
	printf ("    %s --extract filesys.bkd\n", progname);
	printf ("    %s --check [--fix] filesys.bkd\n", progname);
	printf ("    %s --new --size=bytes filesys.bkd\n", progname);
	printf ("\n");
	printf ("Options:\n");
	printf ("  -a, --add          Add files to filesystem.\n");
	printf ("  -x, --extract      Extract all files.\n");
	printf ("  -c, --check        Check filesystem, use -c -f to fix.\n");
	printf ("  -f, --fix          Fix bugs in filesystem.\n");
	printf ("  -n, --new          Create new filesystem, -s required.\n");
	printf ("  -s NUM, --size=NUM Size in bytes for created filesystem.\n");
	printf ("  -b FILE, --boot=FILE Boot sector, -B required if not -F.\n");
	printf ("  -B FILE, --boot2=FILE Secondary boot sector, -b required.\n");
	printf ("  -F, --flat         Flat mode, no sector remapping.\n");
	printf ("  -v, --verbose      Print verbose information.\n");
	printf ("  -V, --version      Print version information and then exit.\n");
	printf ("  -h, --help         Print this message.\n");
	printf ("\n");
	printf ("Report bugs to \"%s\".\n", program_bug_address);
}

void print_inode (u6fs_inode_t *inode,
	char *dirname, char *filename, FILE *out)
{
	fprintf (out, "%s/%s", dirname, filename);
	switch (inode->mode & INODE_MODE_FMT) {
	case INODE_MODE_FDIR:
		fprintf (out, "/\n");
		break;
	case INODE_MODE_FCHR:
		fprintf (out, " - char %d %d\n",
			inode->addr[0] >> 8, inode->addr[0] & 0xff);
		break;
	case INODE_MODE_FBLK:
		fprintf (out, " - block %d %d\n",
			inode->addr[0] >> 8, inode->addr[0] & 0xff);
		break;
	default:
		fprintf (out, " - %lu bytes\n", inode->size);
		break;
	}
}

void print_indirect_block (u6fs_t *fs, unsigned int bno, FILE *out)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	fprintf (out, " [%d]", bno);
	if (! u6fs_read_block (fs, bno, data)) {
		fprintf (stderr, "read error at block %d\n", bno);
		return;
	}
	for (i=0; i<LSXFS_BSIZE-2; i+=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			fprintf (out, " %d", nb);
	}
}

void print_double_indirect_block (u6fs_t *fs, unsigned int bno, FILE *out)
{
	unsigned short nb;
	unsigned char data [LSXFS_BSIZE];
	int i;

	fprintf (out, " [%d]", bno);
	if (! u6fs_read_block (fs, bno, data)) {
		fprintf (stderr, "read error at block %d\n", bno);
		return;
	}
	for (i=0; i<LSXFS_BSIZE-2; i+=2) {
		nb = data [i+1] << 8 | data [i];
		if (nb)
			print_indirect_block (fs, nb, out);
	}
}

void print_inode_blocks (u6fs_inode_t *inode, FILE *out)
{
	int i;

	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FCHR ||
	    (inode->mode & INODE_MODE_FMT) == INODE_MODE_FBLK)
		return;

	fprintf (out, "    ");
	if (inode->mode & INODE_MODE_LARG) {
		for (i=0; i<7; ++i) {
			if (inode->addr[i] == 0)
				continue;
			print_indirect_block (inode->fs, inode->addr[i], out);
		}
		if (inode->addr[7] != 0)
			print_double_indirect_block (inode->fs,
				inode->addr[7], out);
	} else {
		for (i=0; i<8; ++i) {
			if (inode->addr[i] == 0)
				continue;
			fprintf (out, " %d", inode->addr[i]);
		}
	}
	fprintf (out, "\n");
}

void extract_inode (u6fs_inode_t *inode, char *path)
{
	int fd, n;
	unsigned long offset;
	unsigned char data [512];

	fd = open (path, O_CREAT | O_WRONLY, inode->mode & 0x777);
	if (fd < 0) {
		perror (path);
		return;
	}
	for (offset = 0; offset < inode->size; offset += 512) {
		n = inode->size - offset;
		if (n > 512)
			n = 512;
		if (! u6fs_inode_read (inode, offset, data, n)) {
			fprintf (stderr, "%s: read error at offset %ld\n",
				path, offset);
			break;
		}
		if (write (fd, data, n) != n) {
			fprintf (stderr, "%s: write error\n", path);
			break;
		}
	}
	close (fd);
}

void extractor (u6fs_inode_t *dir, u6fs_inode_t *inode,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	if (verbose)
		print_inode (inode, dirname, filename, out);

	if ((inode->mode & INODE_MODE_FMT) != INODE_MODE_FDIR &&
	    (inode->mode & INODE_MODE_FMT) != 0)
		return;

	path = alloca (strlen (dirname) + strlen (filename) + 2);
	strcpy (path, dirname);
	strcat (path, "/");
	strcat (path, filename);

	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		if (mkdir (path, 0775) < 0)
			perror (path);
		/* Scan subdirectory. */
		u6fs_directory_scan (inode, path, extractor, arg);
	} else {
		extract_inode (inode, path);
	}
}

void scanner (u6fs_inode_t *dir, u6fs_inode_t *inode,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	print_inode (inode, dirname, filename, out);

	if (verbose > 1) {
		/* Print a list of blocks. */
		print_inode_blocks (inode, out);
		if (verbose > 2) {
			u6fs_inode_print (inode, out);
			printf ("--------\n");
		}
	}
	if ((inode->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Scan subdirectory. */
		path = alloca (strlen (dirname) + strlen (filename) + 2);
		strcpy (path, dirname);
		strcat (path, "/");
		strcat (path, filename);
		u6fs_directory_scan (inode, path, scanner, arg);
	}
}

/*
 * Create a directory.
 */
void add_directory (u6fs_t *fs, char *name)
{
	u6fs_inode_t dir, parent;
	char buf [512], *p;

	/* Open parent directory. */
	strcpy (buf, name);
	p = strrchr (buf, '/');
	if (p)
		*p = 0;
	else
		*buf = 0;
	if (! u6fs_inode_by_name (fs, &parent, buf, 0, 0)) {
		fprintf (stderr, "%s: cannot open directory\n", buf);
		return;
	}

	/* Create directory. */
	if (! u6fs_inode_by_name (fs, &dir, name, 1,
	    INODE_MODE_FDIR | 0777)) {
		fprintf (stderr, "%s: directory inode create failed\n", name);
		return;
	}
	u6fs_inode_save (&dir, 0);

	/* Make link '.' */
	strcpy (buf, name);
	strcat (buf, "/.");
	if (! u6fs_inode_by_name (fs, &dir, buf, 3, dir.number)) {
		fprintf (stderr, "%s: dot link failed\n", name);
		return;
	}
	++dir.nlink;
	u6fs_inode_save (&dir, 1);
/*printf ("*** inode %d: increment link counter to %d\n", dir.number, dir.nlink);*/

	/* Make parent link '..' */
	strcat (buf, ".");
	if (! u6fs_inode_by_name (fs, &dir, buf, 3, parent.number)) {
		fprintf (stderr, "%s: dotdot link failed\n", name);
		return;
	}
	if (! u6fs_inode_get (fs, &parent, parent.number)) {
		fprintf (stderr, "inode %d: cannot open parent\n", parent.number);
		return;
	}
	++parent.nlink;
	u6fs_inode_save (&parent, 1);
/*printf ("*** inode %d: increment link counter to %d\n", parent.number, parent.nlink);*/
}

/*
 * Create a device node.
 */
void add_device (u6fs_t *fs, char *name, char *spec)
{
	u6fs_inode_t dev;
	int majr, minr;
	char type;

	if (sscanf (spec, "%c%d:%d", &type, &majr, &minr) != 3 ||
	    (type != 'c' && type != 'b') ||
	    majr < 0 || majr > 255 || minr < 0 || minr > 255) {
		fprintf (stderr, "%s: invalid device specification\n", spec);
		fprintf (stderr, "expected c<major>:<minor> or b<major>:<minor>\n");
		return;
	}
	if (! u6fs_inode_by_name (fs, &dev, name, 1, 0666 |
	    ((type == 'b') ? INODE_MODE_FBLK : INODE_MODE_FCHR))) {
		fprintf (stderr, "%s: device inode create failed\n", name);
		return;
	}
	dev.addr[0] = majr << 8 | minr;
	u6fs_inode_save (&dev, 1);
}

/*
 * Copy file to filesystem.
 * When name is ended by slash as "name/", directory is created.
 */
void add_file (u6fs_t *fs, char *name)
{
	u6fs_file_t file;
	FILE *fd;
	char data [512], *p;
	int len;

	if (verbose) {
		printf ("%s\n", name);
	}
	p = strrchr (name, '/');
	if (p && p[1] == 0) {
		*p = 0;
		add_directory (fs, name);
		return;
	}
	p = strrchr (name, '!');
	if (p) {
		*p++ = 0;
		add_device (fs, name, p);
		return;
	}
	fd = fopen (name, "r");
	if (! fd) {
		perror (name);
		return;
	}
	if (! u6fs_file_create (fs, &file, name, 0777)) {
		fprintf (stderr, "%s: cannot create\n", name);
		return;
	}
	for (;;) {
		len = fread (data, 1, sizeof (data), fd);
/*		printf ("read %d bytes from %s\n", len, name);*/
		if (len < 0)
			perror (name);
		if (len <= 0)
			break;
		if (! u6fs_file_write (&file, data, len)) {
			fprintf (stderr, "%s: write error\n", name);
			break;
		}
	}
	u6fs_file_close (&file);
	fclose (fd);
}

void add_boot (u6fs_t *fs)
{
	if (flat) {
		if (boot_sector2) {
			fprintf(stderr, "Secondary boot ignored\n");
		}
		if (boot_sector) {
			if (! u6fs_install_single_boot (fs, boot_sector)) {
				fprintf (stderr, "%s: incorrect boot sector\n",
				boot_sector);
				return;
			}
			printf ("Boot sector %s installed\n", boot_sector);
		}
	} else if (boot_sector && boot_sector2) {
		if (! u6fs_install_boot (fs, boot_sector,
		    boot_sector2)) {
			fprintf (stderr, "%s: incorrect boot sector\n",
				boot_sector);
			return;
		}
		printf ("Boot sectors %s and %s installed\n",
			boot_sector, boot_sector2);
	}
}

int main (int argc, char **argv)
{
	int i, key;
	u6fs_t fs;
	u6fs_inode_t inode;

	for (;;) {
		key = getopt_long (argc, argv, "vaxncfFs:b:B:",
			program_options, 0);
		if (key == -1)
			break;
		switch (key) {
		case 'v':
			++verbose;
			break;
		case 'a':
			++add;
			break;
		case 'x':
			++extract;
			break;
		case 'n':
			++newfs;
			break;
		case 'c':
			++check;
			break;
		case 'f':
			++fix;
			break;
		case 'F':
			++flat;
			break;
		case 's':
			bytes = strtol (optarg, 0, 0);
			break;
		case 'b':
			boot_sector = optarg;
			break;
		case 'B':
			boot_sector2 = optarg;
			break;
		case 'V':
			printf ("%s\n", program_version);
			return 0;
		case 'h':
			print_help (argv[0]);
			return 0;
		default:
			print_help (argv[0]);
			return -1;
		}
	}
	i = optind;
	if ((! add && i != argc-1) || (add && i >= argc-1) ||
	    (extract + newfs + check + add > 1) ||
	    (!flat && (! boot_sector ^ ! boot_sector2)) ||
	    (newfs && bytes < 5120)) {
		print_help (argv[0]);
		return -1;
	}

	if (newfs) {
		/* Create new filesystem. */
		if (! u6fs_create (&fs, argv[i], bytes)) {
			fprintf (stderr, "%s: cannot create filesystem\n", argv[i]);
			return -1;
		}
		printf ("Created filesystem %s - %ld bytes\n", argv[i], bytes);
		add_boot (&fs);
		u6fs_close (&fs);
		return 0;
	}

	if (check) {
		/* Check filesystem for errors, and optionally fix them. */
		if (! u6fs_open (&fs, argv[i], fix)) {
			fprintf (stderr, "%s: cannot open\n", argv[i]);
			return -1;
		}
		u6fs_check (&fs);
		u6fs_close (&fs);
		return 0;
	}

	/* Add or extract or info or boot update. */
	if (! u6fs_open (&fs, argv[i],
			(add != 0) || (boot_sector && boot_sector2))) {
		fprintf (stderr, "%s: cannot open\n", argv[i]);
		return -1;
	}

	if (extract) {
		/* Extract all files to current directory. */
		if (! u6fs_inode_get (&fs, &inode, 1)) {
			fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
			return -1;
		}
		u6fs_directory_scan (&inode, ".", extractor, (void*) stdout);
		u6fs_close (&fs);
		return 0;
	}

	add_boot (&fs);

	if (add) {
		/* Add files i+1..argc-1 to filesystem. */
		while (++i < argc)
			add_file (&fs, argv[i]);
		u6fs_sync (&fs, 0);
		u6fs_close (&fs);
		return 0;
	}

	/* Print the structure of flesystem. */
	u6fs_print (&fs, stdout);
	if (verbose) {
		printf ("--------\n");
		if (! u6fs_inode_get (&fs, &inode, 1)) {
			fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
			return -1;
		}
		if (verbose > 1) {
			u6fs_inode_print (&inode, stdout);
			printf ("--------\n");
			printf ("/\n");
			print_inode_blocks (&inode, stdout);
		}
		u6fs_directory_scan (&inode, "", scanner, (void*) stdout);
	}
	u6fs_close (&fs);
	return 0;
}
