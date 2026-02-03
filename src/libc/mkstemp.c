/*
 * Generate a unique temporary file name and open it.
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Generate a unique temporary file name from TEMPLATE.
 * The last six characters of TEMPLATE must be "XXXXXX";
 * they are replaced with a string that makes the filename unique.
 * Returns a file descriptor open on the file for reading and writing,
 * or -1 on error.
 */
int
mkstemp (template)
	char *template;
{
	static int value;
	char *XXXXXX;
	int len, count;
	long now[2];

	len = strlen (template);
	if (len < 6 || strncmp (&template[len - 6], "XXXXXX", 6)) {
		return -1;
	}
	XXXXXX = &template[len - 6];
	time (now);
	value += getpid () + (int)now[0] + (int)now[1];

	for (count = 0; count < 100; ++count) {
		register int v = value & 077777;
		int fd;

		XXXXXX[0] = (v & 31) + 'A';
		v >>= 5;
		XXXXXX[1] = (v & 31) + 'A';
		v >>= 5;
		XXXXXX[2] = (v & 31) + 'A';
		fd = creat (template, 0600);
		if (fd >= 0) {
			return fd;
		}

		value += 7777;
	}

	template[0] = '\0';
	return -1;
}
