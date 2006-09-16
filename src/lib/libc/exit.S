/ C library -- exit

/ exit(code)
/ code is return in r0 to system

exit = 1

	.globl	_exit
_exit:
	mov	2(sp),r0
	sys	exit
