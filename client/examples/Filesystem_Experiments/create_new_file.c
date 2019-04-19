#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int main()
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    FILE *fd = fopen("temp_file", "w");
    gettimeofday(&end, NULL);
    unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("Time taken: %llu millisec\n",t1);
    printf("Time taken: %llu microsec\n",t2);

    fclose(fd);

    return 0;
}
