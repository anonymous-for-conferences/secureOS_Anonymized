#include <stdio.h>
#include <sys/types.h>
#include <signal.h>

int main()
{
    kill(-1,9);

    return 0;
}
