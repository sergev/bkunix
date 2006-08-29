#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <argp.h>
#include "lsxfs.h"

int verbose;
int extract;
int newfs;
int check;
int fix;
unsigned long bytes;
char *boot_sector;
char *boot_sector2;

const char *argp_program_version =
	"LSX file system information, version 1.0\n"
	"Copyright (C) 2002 Serge Vakulenko\n"
	"This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
	"see the GNU General Public License for more details.";

const char *argp_program_bug_address = "<vak@cronyx.ru>";

struct argp_option argp_options[] = {
	{"verbose",	'v', 0,		0,	"Print verbose information" },
	{"extract",	'x', 0,		0,	"Extract all files" },
	{"check",	'c', 0,		0,	"Check filesystem, use -c -f to fix" },
	{"fix",		'f', 0,		0,	"Fix bugs in filesystem" },
	{"new",		'n', 0,		0,	"Create new filesystem, -s required" },
	{"size",	's', "NUM",	0,	"Size in bytes for created filesystem" },
	{"boot",	'b', "FILE",	0,	"Boot sector for created filesystem" },
	{"boot2",	'B', "FILE",	0,	"Secondary boot sector" },
	{ 0 }
};

/*
 * Parse a single option.
 */
int argp_parse_option (int key, char *arg, struct argp_state *state)
{
	switch (key) {
	case 'v':
		++verbose;
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
	case 's':
		bytes = strtol (arg, 0, 0);
		break;
	case 'b':
		boot_sector = arg;
		break;
	case 'B':
		boot_sector2 = arg;
		break;
	case ARGP_KEY_END:
		if (state->arg_num < 1)		/* Not enough arguments. */
			argp_usage (state);
		break;
	default:
		return ARGP_ERR_UNKNOWN;
	}
	return 0;
}

/*
 * Our argp parser.
 */
const struct argp argp_parser = {
	/* The options we understand. */
	argp_options,

	/* Function to parse a single option. */
	argp_parse_option,

	/* A description of the arguments we accept. */
	"infile.dsk",

	/* Program documentation. */
	"\nPrint LSX file system information"
};

void print_file (lsxfs_inode_t *file,
	char *dirname, char *filename, FILE *out)
{
	fprintf (out, "%s/%s", dirname, filename);
	switch (file->mode & INODE_MODE_FMT) {
	case INODE_MODE_FDIR:
		fprintf (out, "/\n");
		break;
	case INODE_MODE_FCHR:
		fprintf (out, " - char %d %d\n",
			file->addr[0] >> 8, file->addr[0] & 0xff);
		break;
	case INODE_MODE_FBLK:
		fprintf (out, " - block %d %d\n",
			file->addr[0] >> 8, file->addr[0] & 0xff);
		break;
	default:
		fprintf (out, " - %lu bytes\n", file->size);
		break;
	}
}

void extract_file (lsxfs_inode_t *file, char *path)
{
	int fd, n;
	unsigned long offset;
	unsigned char data [512];

	fd = open (path, O_CREAT | O_WRONLY, file->mode & 0x777);
	if (fd < 0) {
		perror (path);
		return;
	}
	for (offset = 0; offset < file->size; offset += 512) {
		n = file->size - offset;
		if (n > 512)
			n = 512;
		if (! lsxfs_file_read (file, offset, data, n)) {
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

void extractor (lsxfs_inode_t *dir, lsxfs_inode_t *file,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	if (verbose)
		print_file (file, dirname, filename, out);

	if ((file->mode & INODE_MODE_FMT) != INODE_MODE_FDIR &&
	    (file->mode & INODE_MODE_FMT) != 0)
		return;

	path = alloca (strlen (dirname) + strlen (filename) + 2);
	strcpy (path, dirname);
	strcat (path, "/");
	strcat (path, filename);

	if ((file->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		if (mkdir (path, 0775) < 0)
			perror (path);
		/* Scan subdirectory. */
		lsxfs_directory_scan (file, path, extractor, arg);
	} else {
		extract_file (file, path);
	}
}

void scanner (lsxfs_inode_t *dir, lsxfs_inode_t *file,
	char *dirname, char *filename, void *arg)
{
	FILE *out = arg;
	char *path;

	print_file (file, dirname, filename, out);

	if (verbose) {
		lsxfs_inode_print (file, out);
		printf ("--------\n");
	}
	if ((file->mode & INODE_MODE_FMT) == INODE_MODE_FDIR) {
		/* Scan subdirectory. */
		path = alloca (strlen (dirname) + strlen (filename) + 2);
		strcpy (path, dirname);
		strcat (path, "/");
		strcat (path, filename);
		lsxfs_directory_scan (file, path, scanner, arg);
	}
}

int main (int argc, char **argv)
{
	int i;
	lsxfs_t fs;
	lsxfs_inode_t inode;

	argp_parse (&argp_parser, argc, argv, 0, &i, 0);
	if (i != argc-1 || (extract + newfs + check > 1) ||
	    (newfs && bytes < 5120)) {
		argp_help (&argp_parser, stderr, ARGP_HELP_USAGE, argv[0]);
		return -1;
	}
	if (newfs) {
		/* Create new filesystem. */
		if (! lsxfs_create (&fs, argv[i], bytes)) {
			fprintf (stderr, "%s: cannot create filesystem\n", argv[i]);
			return -1;
		}
		printf ("Created filesystem %s - %ld bytes\n", argv[i], bytes);
		if (boot_sector && boot_sector2) {
			if (! lsxfs_install_boot (&fs, boot_sector,
			    boot_sector2)) {
				fprintf (stderr, "%s: incorrect boot sector\n",
					boot_sector);
				return -1;
			}
			printf ("Boot sectors %s and %s installed\n",
				boot_sector, boot_sector2);
		}
		lsxfs_close (&fs);
		return 0;
	}

	if (check) {
		/* Check filesystem for errors, and optionally fix them. */
		if (! lsxfs_open (&fs, argv[i], fix)) {
			fprintf (stderr, "%s: cannot open\n", argv[i]);
			return -1;
		}
		lsxfs_check (&fs);
		lsxfs_close (&fs);
		return 0;
	}

	if (! lsxfs_open (&fs, argv[i], 0)) {
		fprintf (stderr, "%s: cannot open\n", argv[i]);
		return -1;
	}

	if (extract) {
		/* Extract all files to current directory. */
		if (! lsxfs_inode_get (&fs, &inode, 1)) {
			fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
			return -1;
		}
		lsxfs_directory_scan (&inode, ".", extractor, (void*) stdout);
		lsxfs_close (&fs);
		return 0;
	}

	/* Print the structure of flesystem. */
	lsxfs_print (&fs, stdout);
	if (verbose) {
		printf ("--------\n");
		lsxfs_inode_print (&inode, stdout);
		if (verbose > 1) {
			printf ("--------\n");
			lsxfs_directory_scan (&inode, "", scanner, (void*) stdout);
		}
	}
	lsxfs_close (&fs);
	return 0;
}
