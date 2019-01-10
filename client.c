#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <error.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	char httpFile[10000];
	if (argc < 2)
	{
		printf("\nPort number is missing...\n");
		exit(0);
	}

	portno = atoi(argv[1]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");
	server = gethostbyname("129.120.151.94"); //IP address of server
	//server = gethostbyname("localhost"); //Both in the same machine [IP address 127.0.0.1]

	if (server == NULL)
	{
		printf("\nERROR, no such host...\n");
		exit(0);
	}

	//Connecting with the server
	bzero((char *) &serv_addr, sizeof(serv_addr));
	bzero(httpFile, 10000);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0)
		error(EXIT_FAILURE, 0, "ERROR connecting the server...");
	char exitCopmare[256];
	while(1)
	{
		//Sending the message to the server
		printf("\nEnter URL of the website you wish to visit (or enter 'quit' without the quotations to exit): ");
		bzero(buffer,256);
		scanf("%s", buffer);
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0)
		{
			error(EXIT_FAILURE, 0, "ERROR writing to socket");
		}

		bzero(exitCopmare, 256);

		strncpy(exitCopmare, buffer, strlen(buffer));//used to see if client wants to quit
		if(strcmp(exitCopmare,"quit") == 0)//exitCopmare message
		{
			close(sockfd);
			break;
		}

		bzero(buffer,256);

		n = read(sockfd, httpFile, 10000);
		if (n < 0)
			error(EXIT_FAILURE, 0, "ERROR reading from socket");
		printf("\nThe content received from proxy server:\n%s\n", httpFile);

		bzero(httpFile, 10000);
	}

	return 0;
}
