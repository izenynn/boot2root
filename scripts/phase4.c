#include <stdio.h>

#define MAX 1000000

int function(int n)
{
	int n1;
	int n2;

	if (n < 2) {
		n2 = 1;
	} else {
		n1 = function(n - 1);
		n2 = function(n - 2);
		n2 = n2 + n1;
	}
	return n2;
}

int main(void)
{
	int i;
	int ret;

	for (i = 1; i < MAX; ++i) {
		ret = function(i);
		if (ret == 55) {
			break;
		}
	}

	if (i == MAX) {
		printf("Exhausted.");
		return 1;
	}

	printf("Cracked: %d\n", i);

	return 0;
}
