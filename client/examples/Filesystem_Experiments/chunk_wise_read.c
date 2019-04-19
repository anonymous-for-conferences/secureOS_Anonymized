#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#define CHUNK_SIZE 32000

int main()
{
    int m, fd = open("temp_file", O_RDONLY);
    char c[CHUNK_SIZE];
    struct timeval start, end;
    gettimeofday(&start, NULL);
    while((m=read(fd, &c, CHUNK_SIZE))>0);
    gettimeofday(&end, NULL);
    unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("Time taken: %llu millisec\n",t1);
    printf("Time taken: %llu microsec\n",t2);
    close(fd);

    return 0;
}
