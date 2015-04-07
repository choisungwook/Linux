#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int main()
{
        char dst[100], src[100];
        struct stat st_buf;
        int src_stat, dst_stat;

        printf("input [src] [dst] :  ");
        scanf("%s %s",src,dst);

        if( (stat(src,&st_buf) < 0) ||
                ((src_stat = open(src, O_RDONLY) < 0)) ||
                ((dst_stat = creat(dst, st_buf.st_mode)) < 0))
                return 0;
        return 0;
}
