#include <stdio.h>
#include <unistd.h>
#include <dirent.h>

int main()
{
	DIR *dir_info;
	struct dirent *dir_entry;

	dir_info = opendir("/tmp");
	if(dir_info){
		while( dir_entry = readdir(dir_info))
			printf("%s\n",dir_entry->d_name);
		closedir(dir_info);
	}
	return 0;
}
