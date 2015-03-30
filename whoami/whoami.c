#include <stdio.h>
#include <unistd.h>

int main()
{
	char *username;
	username = getlogin();
	printf("%s\n",username);
	
	return 0;
}
