#include <stdio.h>
#include <dirent.h>

int main()
{
    char change_directory[100] = {0,};

    scanf("%s",change_directory);

    if(chdir(change_directory) < 0)
        perror(change_directory);
    else
        printf("change directory success\n");

    return 0;

}
