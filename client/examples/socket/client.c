#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in server_addr;
	char buff[1024];
	socklen_t server_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[1]));
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int ret = connect(sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
	ret = recv(sockfd, buff, 1024, 0);
	printf("Received bytes: %d\n", ret);
	if(ret > 0)
	    printf("%s\n", buff);
	else
		printf("Receive failed!");
	close(sockfd);

	return 0;
}
