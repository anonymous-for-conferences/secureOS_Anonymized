#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
    if(kill(atoi(argv[1]), SIGKILL) == -1)
		perror("Signal Error");
    if(sigqueue(atoi(argv[2]), SIGKILL, (const union sigval)NULL) == -1)
		perror("Signal Error");

    return 0;
}
