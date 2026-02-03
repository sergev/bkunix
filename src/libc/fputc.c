/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

int
fputc(c, fp)
	FILE *fp;
{
	return putc(c, fp);
}
