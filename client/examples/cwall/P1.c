#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int
main(int argc, char *argv[]){
    char buf[1024];
    int n, f1, d2;
    printf("\nStart.....\n");
    getchar();

    d2 = open("D2", O_RDONLY);
    if(d2<0){
        printf("\nFailed to open 'D2'");
        exit(1);
    }else{
        n = read(d2, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead done.");
        close(d2);
    }

    getchar();
    f1 = open("f1", O_CREAT|O_RDWR|O_TRUNC, 0666);
    if(f1<0){
        printf("\nFailed to create 'f1'");
        exit(1);
    }else{
        //n = write(fd2, buf, n);
        close(f1);
        printf("\nClosed f1.");
    }
    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

