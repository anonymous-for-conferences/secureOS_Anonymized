#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    char buf[1024];
    int n, fd1, fd2;
    printf("\nStarting execution.....\n");

    fd2 = open("unprotected_file", O_WRONLY);
    if(fd2<0){
        printf("\nFailed to open unprotected_file\n");
        exit(1);
    }else{
        n = write(fd2, buf, n);
		if(n<0) {
			printf("\nWriting unprotected_file failed!\n");
			exit(1);
		}
        close(fd2);
        printf("\nWriting unprotected_file done.\n");
    }
    printf("\nTerminating.......\n");

	return 0;
}

