#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
        struct stat buf;
        stat("/tmp",&buf);

        return 0;
}
