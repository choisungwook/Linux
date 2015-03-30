#include <unistd.h>
#include <stdio.h>
#define SIZE 100

int main()
{
	char path[SIZE] = {0,};

	getcwd(path,SIZE);
	printf("%s\n",path);
	return 0;
}

