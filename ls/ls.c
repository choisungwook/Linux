#include <unistd.h>
#include <dirent.h>
#include <string.h>

//function
void error_handling(char *msg);

char *cmd;

int main()
{
    while(1)
    {
        printf("input path >>>> \n");
        scanf("%s",cmd);
    }
}

void ls()
{
    struct dirent *dirp;
    int flag = 0;
    char path[100] = {0,};
    char *get_path;
    DIR *dp = NULL;

    getcwd(path,sizeof(path));

    get_path = strtok(cmd," \t\n");
    if((dp = opendir(path)) == NULL)
        error_handling("failed path\n");

    do{

    }while(get_path = strtok(NULL," \t\n");
}

void get_name_len(DIR *dp, int *max)
{
    char dirent *dirp;
    int name_len = 0;

    *max = 0;

    while((dirp = readdir(dp)) != NULL){
        strlen(dipr->d_name);
        if(name_len > *max)
            *max = name_len;
    }
}

void error_handling(char *msg)
{
    fprintf(stderr,"%s\n",msg);
    exit(-1);
}
