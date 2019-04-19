#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int
main(int argc, char *argv[]){
    char buf[1024];
    int n, fd1, fd2;
    printf("\nStart.....\n");
    getchar();

    fd1 = open("f2", O_RDONLY);
    if(fd1<0){
        printf("\nFailed to open 'f2'");
        exit(1);
    }else{
        n = read(fd1, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead done.");
        close(fd1);
    }

    getchar();
    fd2 = open("f1", O_WRONLY);
    if(fd2<0){
        printf("\nFailed to open 'f1'");
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

