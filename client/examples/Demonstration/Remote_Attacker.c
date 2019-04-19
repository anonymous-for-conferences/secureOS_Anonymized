#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int main(){
    char buf[1024];
    int n, fd, fd2;
    printf("\nStarting execution.....\n");
	fd2 = open("top_secret_file", O_RDONLY);
    if(fd2<0){
        perror("Failed to open top_secret_file");
    }else{
        n = read(fd2, buf, 1024);
		buf[n] = '\0';
        close(fd2);
        printf("\nReading top_secret_file done.\n");
		printf("\nAcquired information: %s\n",buf);
    }

	getchar();

    fd = open("unprotected_file", O_RDONLY);
    if(fd<0){
        printf("\nFailed to open unprotected_file\n");
        exit(1);
    }else{
        n = read(fd, buf, 1024);
		buf[n] = '\0';
        close(fd);
        printf("\nReading unprotected_file done.\n");
		printf("\nAcquired information: %s\n",buf);
    }
    printf("\nTerminating.......\n");

    return 0;
}

