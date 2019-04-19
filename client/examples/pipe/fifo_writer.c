#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
	int fifofd = open("tmp_fifo", O_WRONLY);
	char buff[1024];
	sprintf(buff, "Hey, I am writer");
	write(fifofd, buff, strlen(buff)+1);
	close(fifofd);

	return 0;
}
