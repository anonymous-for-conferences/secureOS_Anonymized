#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main() {
	int pipefd[2], pid;
	pipe(pipefd);
	if((pid = fork()) == 0) {
		close(pipefd[1]);
		char buff[1024];
		read(pipefd[0], buff, 1024);
		printf("Parent said: %s\n", buff);
		close(pipefd[0]);
	} else {
		close(pipefd[0]);
		char buff[1024];
		sprintf(buff, "Hello child, your pid is %d", pid);
		write(pipefd[1], buff, strlen(buff)+1);
		close(pipefd[1]);
	}
	return 0;
}
