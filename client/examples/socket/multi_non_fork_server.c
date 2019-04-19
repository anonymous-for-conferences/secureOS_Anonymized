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
	int listen_sockfd, connection_sockfd, childpid;
	struct sockaddr_in server_addr, client_addr;
	char buff[1024];
	socklen_t server_addr_sz, client_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(0);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;
	client_addr_sz = sizeof client_addr;

	listen_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	bind(listen_sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
	getsockname(listen_sockfd, (struct sockaddr *) &server_addr, &server_addr_sz);
	if(listen(listen_sockfd,5)==0)
	{
		printf("Listening on %s:%u\n",inet_ntoa(server_addr.sin_addr),ntohs(server_addr.sin_port));
	}
	else
	{
		printf("Error in listening!\n");
		perror("listen");
		exit(1);
	}

	while(1)
	{
		connection_sockfd = accept(listen_sockfd, (struct sockaddr *) &client_addr, &client_addr_sz);
//		if((childpid=fork())==0)
//		{
//			close(listen_sockfd);
			strcpy(buff,"Hello ");
			strcat(buff,inet_ntoa(client_addr.sin_addr));
			strcat(buff,", You are listening on port ");
			char port[10];
			sprintf(port,"%u",ntohs(client_addr.sin_port));
			strcat(buff,port);
			send(connection_sockfd,buff,sizeof buff, 0);
			close(connection_sockfd);
			
//			return 0;
//		}
//		else
//			close(connection_sockfd);
	}

	return 0;
}
