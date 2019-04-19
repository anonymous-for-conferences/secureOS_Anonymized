#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//p2 running with UID = u1 reads D11, creates f2, reads f1 and writes f2.

int
main(int argc, char *argv[]){
    char buf[1024];
    int n, d11, f1, f2;
    printf("\nStart.....\n");
    getchar();

    d11 = open("D11", O_RDONLY);
    if(d11<0){
        printf("\nFailed to open 'D2'");
        exit(1);
    }else{
        n = read(d11, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead done.");
        close(d11);
    }

    getchar();

    f2 = open("f2", O_CREAT|O_RDWR|O_TRUNC, 0666);
    if(f2<0){
        printf("\nFailed to create 'f2'");
        exit(1);
    }else{
        printf("\nCreated f2.");
        close(f2);
        getchar();
    }

    f1 = open("f1", O_RDONLY);
    if(f1<0){
        printf("\nFailed to open 'f1'");
        exit(1);
    }else{
        n = read(f1, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead f1.");
        getchar();
        close(f1);
    }

    f2 = open("f2", O_WRONLY);
    if(f2<0){
        printf("\nFailed to open 'f2' for writing.");
        exit(1);
    }else{
        n = write(f2, buf, n);
        printf("\nWritten to f2.");
        close(f2);
    }

    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

