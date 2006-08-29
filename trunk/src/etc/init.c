char shell[] "/bin/sh";
char minus[] "-";
char ctty[] "/dev/tty8";

main()
{

	open(ctty, 2);
	dup(0);
	dup(0);
	chdir("/usr");
	open("/bin", 4);
	execl(shell, minus, 0);
	exit();
}
