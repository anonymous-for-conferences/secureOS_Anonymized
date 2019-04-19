#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
	int fifofd = open("tmp_fifo", O_RDONLY);
	char buff[1024];
	int ret = read(fifofd, buff, 1024);
	if(ret == -1)
		printf("Read failed!");
	else
		printf("Writer said: %s\n", buff);
	close(fifofd);

	return 0;
}
