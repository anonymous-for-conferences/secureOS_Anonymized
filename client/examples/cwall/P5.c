#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//p5 reads D11 and reads D12, running with UID of any of the 8 users. Denied in all cases.

int
main(int argc, char *argv[]){
    char buf[1024];
    int n, d11, d12;
    printf("\nStart.....\n");
    getchar();

    d11 = open("D11", O_RDONLY);
    if(d11<0){
        printf("\nFailed to open 'D11' for read.");
        exit(1);
    }else{
        n = read(d11, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead D11.");
        close(d11);
    }

    getchar();

    d12 = open("D12", O_RDONLY);
    if(d12<0){
        printf("\nFailed to open 'D12' for read.");
        exit(1);
    }else{
        n = read(d12, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead D12.");
        close(d12);
    }

    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

