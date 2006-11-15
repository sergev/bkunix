int
getchar ()
{
        register int c;

	asm ("mov r5,-(sp)");
	c = ((int(*)()) 0101010) ();
	asm ("mov (sp)+,r5");
	return c;
}

int
putchar (c)
        int c;
{
	asm ("mov 4(r5),r0");
	asm ("mov r5,-(sp)");
	asm ("jsr pc,*$0102234");
	asm ("mov (sp)+,r5");
}

void
printf (str)
	char *str;
{
	while (*str)
		putchar (*str++);
}

void
phexdigit (val)
{
	val &= 15;
	if (val <= 9)
		val += '0';
	else
		val += 'a' - 10;
	putchar (val);
}

asm (".text");
asm ("_rol4: .globl _rol4");
asm ("mov 2(sp),r0");
asm ("mov r0,r1");
asm ("rol r1");
asm ("rol r0");
asm ("rol r1");
asm ("rol r0");
asm ("rol r1");
asm ("rol r0");
asm ("rol r1");
asm ("rol r0");
asm ("rts pc");

void
printhex (val)
{
	val = rol4 (val);
	phexdigit (val);
	val = rol4 (val);
	phexdigit (val);
	val = rol4 (val);
	phexdigit (val);
	val = rol4 (val);
	phexdigit (val);
}
