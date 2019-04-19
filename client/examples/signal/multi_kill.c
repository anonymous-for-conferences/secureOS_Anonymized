#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
    int cpid1, cpid2, cpid3;
    if((cpid1=fork())==0)
        while(1);
    if((cpid2=fork())==0)
        while(1);
    if((cpid3=fork())==0)
        while(1);

    printf("c1:%d c2:%d c3:%d parent:%d\n",cpid1,cpid2,cpid3,getpid());
    sleep(3);
    kill(0,9);

    return 0;
}
