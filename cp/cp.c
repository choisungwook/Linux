#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
void error_handling(char *msg);
int main()
{
        struct stat s_buf;
        char src[100],dst[100];
        int src_id, dst_id;
        char buf[1000];
        int len;

        printf("input [src] [dst] : ");
        scanf("%s %s",src,dst);

        if( stat(src,&s_buf) < 0)
                error_handling("stat error");
        if( (src_id = open(src,O_RDONLY)) < 0)
                error_handling("open error");
        if( (dst_id = creat(dst,s_buf.st_mode)) < 0)
                error_handling("creat error");

        while( (len = read(src_id,buf,sizeof(buf))) > 0){
                if( write(dst_id,buf,len) != len){
                        error_handling("write error");
                }
        }
        
        close(src_id);
        close(dst_id);
        return 0;
}

void error_handling(char *msg)
{
        fprintf(stderr,"%s",msg);
        exit(-1);
}

