#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//p3 reads f1, running with UID = d11 or d12. Denied in both cases.

int
main(int argc, char *argv[]){
    char buf[1024];
    int n, f1;
    printf("\nStart.....\n");
    getchar();

    f1 = open("f1", O_RDONLY);
    if(f1<0){
        printf("Failed to open 'f1' for read.\n");
        exit(1);
    }else{
        n = read(f1, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead f1.");
        close(f1);
    }

    exit(EXIT_SUCCESS);
}

