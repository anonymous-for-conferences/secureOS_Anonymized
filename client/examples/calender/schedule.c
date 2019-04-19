#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int
main(int argc, char *argv[]){
    char buf[1024];
    int n, ac, bc, mt;
    printf("\nStart.....\n");
    getchar();

    ac = open("AC", O_RDONLY);
    if(ac<0){
        printf("\nFailed to open 'AC'");
        exit(1);
    }else{
        n = read(ac, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead 'AC'.");
    }

    getchar();

    bc = open("BC", O_RDONLY);
    if(bc<0){
        printf("\nFailed to open 'BC'");
        close(ac);
        exit(1);
    }else{
        n = read(bc, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead 'BC'.");
    }


    getchar();

    mt = open("MT", O_CREAT|O_RDWR|O_TRUNC, 0666);
    if(mt<0){
        printf("\nFailed to create 'MT'");
        close(ac);
        close(bc);
        exit(1);
    }else{
        strcpy(buf, "Alice and bob can meet anytime.\n");
        n = strlen(buf);
        n = write(mt, buf, n);
    }

    close(ac);
    close(bc);
    close(mt);

    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

