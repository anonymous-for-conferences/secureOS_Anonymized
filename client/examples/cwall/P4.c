#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//p4 running with UID = u1 reads D11 and writes D2. Write denied by RWFM

int
main(int argc, char *argv[]){
    char buf[1024];
    int n, d11, d2;
    printf("\nStart.....\n");
    getchar();

    d11 = open("D11", O_RDONLY);
    if(d11<0){
        printf("\nFailed to open 'D11'");
        exit(1);
    }else{
        n = read(d11, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead D11");
        close(d11);
    }

    getchar();
    d2 = open("D2", O_WRONLY);
    if(d2<0){
        printf("\nFailed to write 'D2'");
        exit(1);
    }else{
        n = write(d2, buf, n);
        close(d2);
        printf("\nWrite done to D2.");
    }
    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

