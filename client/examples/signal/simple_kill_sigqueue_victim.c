#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{
    printf("Process started with pid:%d\n",getpid());
    while(1);
}
