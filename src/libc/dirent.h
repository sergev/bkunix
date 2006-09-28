#ifndef _DIRENT_H_
#define _DIRENT_H_ 1

#include <ansidecl.h>

struct dirent {
	short d_ino;			/* inode number */
	char d_name [14];		/* file name */
};

#define DIR int

DIR *opendir PARAMS((char*));
struct dirent *readdir PARAMS((DIR*));
int closedir PARAMS((DIR*));
int dirfd PARAMS((DIR*));

#endif /* _DIRENT_H_ */
