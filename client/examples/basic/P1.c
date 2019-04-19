#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


int
main(int argc, char *argv[]){
    char buf[1024];
    int n, fd1, fd2;
    printf("\nStart.....\n");
    getchar();

/*
    fd1 = open("/tmp/ptmp", O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if(fd1<0)
        printf("\nFailed to open '/tmp/ptmp'");
    else
        close(fd1);
*/
    fd1 = open("file1", O_RDONLY);
    if(fd1<0){
        printf("\nFailed to open '1'");
        exit(1);
    }else{
        n = read(fd1, buf, 1024);
		if(n<0) {
			printf("\nRead failed!\n");
			exit(1);
		}
        buf[n]='\0';
        printf("%s\n", buf);
        printf("\nRead done.");
        close(fd1);
    }

    getchar();
    fd2 = open("file2", O_WRONLY);
    if(fd2<0){
        printf("\nFailed to open '2'");
        exit(1);
    }else{
        n = write(fd2, buf, n);
		if(n<0) {
			printf("\nWrite failed!\n");
			exit(1);
		}
        close(fd2);
        printf("\nWrite done.");
    }
    getchar();
    printf("Bye.......\n");
    exit(EXIT_SUCCESS);
}

