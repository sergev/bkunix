#!/bin/sh
set - `$1 -v .o`
while ( (($#)) ) do 
shift
case $1 in
-L*) echo $1
     exit 0
     ;;
esac
done
exit 1

