#!/bin/sh

touch dummy$$.o
set - `$1 -v -o /dev/null dummy$$.o 2>&1`
rm dummy$$.o

while [ $# != 0 ]; do
	shift
	case $1 in
	-L*) lib="$lib $1"
	     ;;
	esac
done

if [ -n "$lib" ]; then
	echo $lib
	exit 0
fi
exit 1
