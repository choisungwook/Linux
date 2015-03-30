#include <stdio.h>
#include <unistd.h>
#define SIZE 100

int main()
{
	char hostname[SIZE] = {0,};
	gethostname(hostname,SIZE);
	printf("%s\n",hostname);

	return 0;
}
