#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#define CHUNK_SIZE 32000

#define FILE_SIZE 10000000

int main()
{
    int n, fd = open("temp_file", O_WRONLY);
    char c[2*CHUNK_SIZE];
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for(n=0;n<(FILE_SIZE/CHUNK_SIZE);n++)
    {
        strcpy(c,"");
        for(int i=0;i<CHUNK_SIZE/sizeof(long int);i++)
        {
            char t[9];
            sprintf(t,"%ld", random());
            t[9]='\0';
            strcat(c,t);
        }
        write(fd, c, CHUNK_SIZE);
    }
    gettimeofday(&end, NULL);
    unsigned long long t1 = 1000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
    unsigned long long t2 = 1000000 * (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
    printf("Time taken: %llu millisec\n",t1);
    printf("Time taken: %llu microsec\n",t2);

    close(fd);

    return 0;
}
