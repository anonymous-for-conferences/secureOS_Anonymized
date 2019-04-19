#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, char *argv[]){
    char buf[1024];
    int n, fd1, fd2;
    printf("\nStart.....\n");

    strcpy(buf, "Writing directly here before opening any other file.\n");
    n = strlen(buf);

    getchar();
    fd2 = open("file2", O_WRONLY);
    if(fd2<0){
        printf("\nFailed to open '2'");
        exit(1);
    }else{
        n = write(fd2, buf, n);
        close(fd2);
        printf("\nWrite done.");
    }
    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

