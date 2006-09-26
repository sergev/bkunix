#include <stdlib.h>

void printint (a)
{
	int b;

	b = a / 10;
	if (b > 0) {
		printint (b);
		a -= b * 10;
	}
	printf ("%c", a + '0');
}

void testint (a, b, op, exp)
	char *op, *exp;
{
	int r = 0;

	switch (op[0] + op[1]) {
	case '*': r = a * b; break;
	case '/': r = a / b; break;
	case '%': r = a % b; break;
	case '*'+'=': r = a; r *= b; break;
	case '/'+'=': r = a; r /= b; break;
	case '%'+'=': r = a; r %= b; break;
	}
	printf ("Integer: ");
	printint (a);
	printf (" %s ", op);
	printint (b);
	printf (" => ");
	printint (r);
	printf (" (expecting %s)\n", exp);
}

void testu (a, b, op, exp)
	unsigned int a, b;
	char *op, *exp;
{
	unsigned int r = 0;

	switch (op[0] + op[1]) {
	case '*': r = a * b; break;
	case '/': r = a / b; break;
	case '%': r = a % b; break;
	case '*'+'=': r = a; r *= b; break;
	case '/'+'=': r = a; r /= b; break;
	case '%'+'=': r = a; r %= b; break;
	}
	printf ("Unsigned: ");
	printint (a);
	printf (" %s ", op);
	printint (b);
	printf (" => ");
	printint (r);
	printf (" (expecting %s)\n", exp);
}

void testlong (a, b, op, exp)
	long a, b;
	char *op, *exp;
{
	long r = 0;

	switch (op[0] + op[1]) {
	case '*': r = a * b; break;
	case '/': r = a / b; break;
	case '%': r = a % b; break;
	case '*'+'=': r = a; r *= b; break;
	case '/'+'=': r = a; r /= b; break;
	case '%'+'=': r = a; r %= b; break;
	}
	printf ("Long: ");
	printint ((int) a);
	printf (" %s ", op);
	printint ((int) b);
	printf (" => ");
	printint ((int) r);
	printf (" (expecting %s)\n", exp);
}

void testulong (a, b, op, exp)
	unsigned long a, b;
	char *op, *exp;
{
	unsigned long r = 0;

	switch (op[0] + op[1]) {
	case '*': r = a * b; break;
	case '/': r = a / b; break;
	case '%': r = a % b; break;
	case '*'+'=': r = a; r *= b; break;
	case '/'+'=': r = a; r /= b; break;
	case '%'+'=': r = a; r %= b; break;
	}
	printf ("Unsigned long: ");
	printint ((int) a);
	printf (" %s ", op);
	printint ((int) b);
	printf (" => ");
	printint ((int) r);
	printf (" (expecting %s)\n", exp);
}

int main ()
{
	testint (67, 89, "*", "5963");
	testint (7654, 34, "/", "225");
 	testint (6789, 54, "%", "39");
 	testint (1, 10, "%", "1");
	testint (67, 89, "*=", "5963");
	testint (7654, 34, "/=", "225");
 	testint (6789, 54, "%=", "39");

	testu (67, 89, "*", "5963");
	testu (7654, 34, "/", "225");
 	testu (6789, 54, "%", "39");
 	testu (1, 10, "%", "1");
	testu (67, 89, "*=", "5963");
	testu (7654, 34, "/=", "225");
 	testu (6789, 54, "%=", "39");

	testlong (67L, 89L, "*", "5963");
	testlong (7654L, 34L, "/", "225");
 	testlong (6789L, 54L, "%", "39");
 	testlong (1L, 10L, "%", "1");
	testlong (67L, 89L, "*=", "5963");
	testlong (7654L, 34L, "/=", "225");
 	testlong (6789L, 54L, "%=", "39");

	testulong (67L, 89L, "*", "5963");
	testulong (7654L, 34L, "/", "225");
 	testulong (6789L, 54L, "%", "39");
 	testulong (1L, 10L, "%", "1");
	testulong (67L, 89L, "*=", "5963");
	testulong (7654L, 34L, "/=", "225");
 	testulong (6789L, 54L, "%=", "39");

	return 0;
}
