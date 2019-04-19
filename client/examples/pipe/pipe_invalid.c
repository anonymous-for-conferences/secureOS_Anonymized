#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main() {
	int pipefd[2], pid;
	pipe(pipefd);
	if((pid = fork()) == 0) {
		close(pipefd[1]);
		char buff[1024];
		read(pipefd[0], buff, 1024);
		close(pipefd[0]);
		int fifofd = open("tmp_fifo", O_WRONLY);
		write(fifofd, buff, strlen(buff)+1);
		close(fifofd);
	} else {
		close(pipefd[0]);
		int filefd = open("tmp_file", O_RDONLY);
		char buff[1024];
		read(filefd, buff, 1024);
		close(filefd);
		write(pipefd[1], buff, strlen(buff)+1);
		close(pipefd[1]);
	}

	return 0;
}
