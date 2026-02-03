/*
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <stdio.h>

int
fgetc(fp)
	FILE *fp;
{
	return getc(fp);
}
