#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

int main()
{
	int listen_sockfd, connection_sockfd, psswdfd;
	struct sockaddr_in server_addr, client_addr;
	char buff[3], file_buff[5];
    char zerobuff[2], onebuff[2];
    zerobuff[0] = '0';
    zerobuff[1] = '\0';
    onebuff[0] = '1';
    onebuff[1] = '\0';
	socklen_t server_addr_sz, client_addr_sz;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(7777);
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(server_addr.sin_zero, '\0', sizeof server_addr.sin_zero);
	server_addr_sz = sizeof server_addr;
	client_addr_sz = sizeof client_addr;

	listen_sockfd = socket(PF_INET, SOCK_STREAM, 0);
	bind(listen_sockfd, (struct sockaddr *) &server_addr, server_addr_sz);
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

	connection_sockfd = accept(listen_sockfd, (struct sockaddr *) &client_addr, &client_addr_sz);
    recv(connection_sockfd, buff, sizeof buff, 0);
    psswdfd = open("psswd.txt",O_RDWR);
    read(psswdfd, file_buff, 1);
    if(file_buff[0]!=buff[0])
    {
        send(connection_sockfd,zerobuff,sizeof zerobuff, 0);
        close(connection_sockfd);
	    close(listen_sockfd);
        close(psswdfd);
        return 0;
    }
    lseek(psswdfd, 0, SEEK_SET);
    write(psswdfd, &buff[1], 1);
	send(connection_sockfd,onebuff,sizeof onebuff, 0);
	close(connection_sockfd);
	close(listen_sockfd);
    close(psswdfd);

	return 0;
}
