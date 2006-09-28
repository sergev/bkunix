char shell[] = "/bin/sh";
char minus[] = "-";
char ctty[] = "/dev/tty8";
char hello[] = "\n** bkunix **\n";

int
main()
{
	open(ctty, 2);
	dup(0);
	dup(0);
	write(0, hello, sizeof(hello) - 1);
	chdir("/usr");
	open("/bin", 4);
	execl(shell, minus, 0);
	return 0;
}
