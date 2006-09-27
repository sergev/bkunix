#include <unistd.h>

char message[] = "Hello, World!\n";

int main ()
{
	write (0, message, sizeof(message) - 1);
	return 0;
}
