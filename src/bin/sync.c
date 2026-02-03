/*
 * Force completion of pending disk writes (flush cache).
 *
 * This file is part of BKUNIX project, which is distributed
 * under the terms of the MIT License.
 * See the accompanying file "LICENSE" for more details.
 */
#include <unistd.h>

int
main()
{
	sync();
	return 0;
}
