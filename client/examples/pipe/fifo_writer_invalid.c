#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
	int fifofd = open("tmp_fifo", O_WRONLY);
	int filefd = open("tmp_file", O_RDONLY);
	char buff[1024];
	read(filefd, buff, 1024);
	write(fifofd, buff, strlen(buff)+1);
	close(fifofd);
	close(filefd);

	return 0;
}
