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

int main()
{
	int sockfd;
	struct sockaddr_in server_addr;
	char buff[3];
	socklen_t server_addr_sz;

    scanf("%s",buff);
    buff[2]='\0';

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7777);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;

	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	connect(sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
    fflush(stdin);
    fflush(stdout);
	send(sockfd, buff, sizeof buff, 0);
    recv(sockfd, buff, sizeof buff, 0);
	printf("Server said: %s",buff);
	close(sockfd);

	return 0;
}
