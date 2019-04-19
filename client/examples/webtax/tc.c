#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


int
main(int argc, char *argv[]){
    char buf[1024];
    int n, td, db, tf;
    printf("\nStart.....\n");
    getchar();

    td = open("TD", O_RDONLY);
    if(td<0){
        printf("\nFailed to open 'TD'");
        exit(1);
    }else{
        n = read(td, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead 'TD'.");
    }

    getchar();
    db = open("DB", O_RDONLY);
    if(db<0){
        printf("\nFailed to open 'DB'");
        exit(1);
    }else{
        n = read(db, buf, 1024);
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead 'DB'.");
    }

    tf = open("TF", O_CREAT|O_RDWR|O_TRUNC, 0666);
    if(tf<0){
        printf("\nFailed to open 'TF'");
        exit(1);
    }else{
        strcpy(buf, "Nothing to compute.\n");
        n = strlen(buf);
        n = write(tf, buf, n);
    }

    close(td);
    close(db);
    close(tf);

    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

