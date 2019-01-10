/*
   Author			: Adrian LaCour
   Date				: 9/18/2018
   Description: This program is the server side. it receives a request to cache a website from
                the client, and attempts to do so. This server can serve multiple hosts, one after the other.
   Compilation: gcc -o server server.c
   Execution  : ./server 9000
*/

//TODO Make sure it adds to the list.txt file after 5, and segmentation faults after 5 .html files are made. also try to change the http request to be proper and see what happens

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <error.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>

char* sendRequest(char inputValueBuffer[10000], int *valueIsUpdated);

int main(int argc, char *argv[])
{
	FILE *cache_list, *tempfile, *cacheList, *cache1;
    int sockfd, newsockfd, portno, clilen, n;
   	struct sockaddr_in serv_addr, cli_addr;
    char buffer[256], *get_request = "GET / HTTP/1.1\r\nHost:", *carriage_returns = "\r\n\r\n";//strings for creating user get requests
    char buffer1[10000];
	char full_request[strlen(buffer)+strlen(get_request)+strlen(carriage_returns)];
	char exitCompare[256];
	int responseValue;//used to get 200 OK or other responses
	int cached_sites = 0;//will be used to determine recency of site
    int option = 1;
    char *cacheContent;
    int sizeOfCacheList = 0;//Initializes the number of cache files

    cacheList = fopen("list.txt", "w+");
    fclose(cacheList);


	if(argc < 2)
	{

		printf("\nPort number is missing...\n");
		exit(0);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR opening socket");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *) &option, sizeof(int));//Resets the port, in case it crashed or was forced out
	if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
		error(EXIT_FAILURE, 0, "ERROR binding");

	printf("\n--- Welcome to the Proxy Server ---\n");
	listen(sockfd, 5);

	//Connecting with the client
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0)
		error(EXIT_FAILURE, 0, "ERROR on accept");

	printf("\nClient is connected...\n");


	while(1)//server always on
	{

		//Receiving the message from the client
		bzero(buffer,256);
		n = read(newsockfd,buffer,255);
	   	if(n < 0)
			error(EXIT_FAILURE, 0, "ERROR reading from socket");

        //Check if the client wants to exit
        bzero(exitCompare,256);
        strncpy(exitCompare, buffer, strlen(buffer));
		if(strcmp(exitCompare,"quit") == 0)//If the client wasnt to exit
		{
            printf("\nThe client has disconnected. Waiting for a new client...\n");
            close(newsockfd);

			listen(sockfd, 5);
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);//accept new client
			if(newsockfd < 0)
			{
				error(EXIT_FAILURE,0,"ERROR on accept");
			}
			printf("Client has connected...\n");
			bzero(buffer, 255);
			bzero(exitCompare,255);
		}
	   	else//If the client doesn't wish to exit
	   	{
            printf("\nClient has requested for the webpage: %s\n", buffer);

            char *returnValue = sendRequest(buffer, &responseValue);

            sprintf(buffer1, "%s", returnValue);


            //Add the requested website to the cacheList, if it is not in the list already and
            //the returned status code is "200 OK"
            if(strstr(buffer1, "200 OK") != NULL)
            {
                cacheList = fopen("list.txt", "r+");

                char ch;
                long lSize;
                char cacheListBuffer[256];
                char copyString[256];

                //Check if the cache file already exists. If so, then don't send a request
                //and just send the contents to the client
                strcpy(copyString, buffer);

                sprintf(copyString + strlen(copyString),".html");
                if(access(copyString, F_OK) != -1)
                {
					printf("\nThe web page is already stored in the cache.\n");

                    cache1 = fopen(copyString, "r+");

                    fprintf(cache1, "%s", buffer1);
                    rewind(cache1);

                    fseek(cache1 , 0L , SEEK_END);
                    lSize = ftell(cache1);
                    rewind(cache1);

                    /* allocate memory for entire content */
                    cacheContent = calloc(1, lSize+1);

                    /* copy the file into the buffer */
                    if(1!=fread(cacheContent , lSize, 1 , cache1))
                      fclose(cache1),free(cacheContent),fputs("entire read fails",stderr),exit(1);

                    fclose(cache1);

                    //Send the contents of the file, now in a buffer, to the client
                    n = write(newsockfd, cacheContent, strlen(cacheContent));
					printf("\nThe web page is successfully sent to the client.\n");

                    bzero(copyString,256);
                    free(cacheContent);
                }
                else//If the .html file does not already exist
                {

                    //If the website is not cached, write the website to the cache list,
                    //request the page from the webserver. If the returned status code is "200 OK"
                    //forward it to the client and then cache it.
                    if(sizeOfCacheList > 4)
                    {
                            //Open the cached file write the contents to a buffer
                            strcpy(copyString, buffer);
                            sprintf(copyString + strlen(copyString),".html");
                            if(access(copyString, F_OK) != -1)//If the .html file exists
                            {
								printf("\nThe web page is already stored in the cache.\n");

                                //Sends the contents of the .html cache file to the client
                                cache1 = fopen(copyString, "r+");

                                fprintf(cache1, "%s", buffer1);
                                rewind(cache1);

                                fseek(cache1 , 0L , SEEK_END);
                                lSize = ftell(cache1);
                                rewind(cache1);

                                /* allocate memory for entire content */
                                cacheContent = calloc(1, lSize+1);

                                /* copy the file into the buffer */
                                if(1!=fread(cacheContent , lSize, 1 , cache1))
                                  fclose(cache1),free(cacheContent),fputs("entire read fails",stderr),exit(1);

                                fclose(cache1);

                                //Send the contents of the file, now in a buffer, to the client
                                n = write(newsockfd, cacheContent, strlen(cacheContent));
								printf("\nThe web page is successfully sent to the client.\n");

                                bzero(copyString,256);
                                free(cacheContent);
                            }
                            else//If the file does not exist
                            {
                                cache1 = fopen(copyString, "w+");
								fclose(cache1);
								printf("\nThe web page is successfully caches, with the filename: %s\n", copyString);

								cache1 = fopen(copyString, "r+");


                                fprintf(cache1, "%s", buffer1);
                                rewind(cache1);

                                fseek(cache1 , 0L , SEEK_END);
                                lSize = ftell(cache1);
                                rewind(cache1);

                                /* allocate memory for entire content */
                                cacheContent = calloc(1, lSize+1);

                                /* copy the file into the buffer */
                                if(1!=fread(cacheContent , lSize, 1 , cache1))
                                  fclose(cache1),free(cacheContent),fputs("entire read fails",stderr),exit(1);

                                fclose(cache1);

                                //Send the contents of the file, now in a buffer, to the client
                                n = write(newsockfd, cacheContent, strlen(cacheContent));
								printf("\nThe web page is successfully sent to the client.\n");


                                bzero(copyString,256);
                                free(cacheContent);
                            }

                            //Delete the earliest cached website from list.txt
                            FILE *tempFile;
                            int deleteLine = 0, temp = 1;

                            rewind(cacheList);
                            tempFile = fopen("replica.txt", "w");

                            ch = getc(cacheList);

                            while(ch != EOF)
                            {
                                if(temp != deleteLine)
                                {
                                    putc(ch, tempFile);
                                }
                                if(ch == '\n')
                                {
                                    temp++;
                                }
                                ch = getc(cacheList);
                            }
                            rewind(cacheList);
                            fclose(cacheList);
                            fclose(tempFile);
                            remove("list.txt");
                            rename("replica.txt", "list.txt");
                            cacheList = fopen("list.txt", "r+");

                            //Print the new cached website to the list.txt
                            fprintf(cacheList, "%s\n", buffer);

                            sizeOfCacheList = 5;//Reset the cache list size

                    }
                    else//If the number of lines in list.txt is under 5, add the website to the file
                    {

                        if(sizeOfCacheList < 5)//If the cache list is not full
                        {
                            char buffer2[1000];
                            char newLineBuf[3];
                            strcpy(newLineBuf, "\n");

                            strcat(buffer2, buffer);
                            strcat(buffer2, newLineBuf);


                            printf("%s", buffer2);

                            fprintf(cacheList, "%s\n", buffer2);
                            sizeOfCacheList++;


                            //Open the cached file write the contents to a buffer
                            strcpy(copyString, buffer);
                            sprintf(copyString + strlen(copyString),".html");


                            cache1 = fopen(copyString, "w+");
							printf("\nThe web page is successfully caches, with the filename: %s\n", copyString);


                            fprintf(cache1, "%s", buffer1);
                            rewind(cache1);

                            fseek(cache1 , 0L , SEEK_END);
                            lSize = ftell(cache1);
                            rewind(cache1);

                            /* allocate memory for entire content */
                            cacheContent = calloc(1, lSize+1);

                            /* copy the file into the buffer */
                            if(1!=fread(cacheContent , lSize, 1 , cache1))
                              fclose(cache1),free(cacheContent),fputs("entire read fails",stderr),exit(1);

                            fclose(cache1);

                            //Send the contents of the file, now in a buffer, to the client
                            n = write(newsockfd, cacheContent, strlen(cacheContent));
							printf("\nThe web page is successfully sent to the client.\n");

                            bzero(copyString,256);
                            free(cacheContent);
                        }
                    }
                }//End of else for if .html exists
                fclose(cacheList);//Closes the list of cached websites
            }//End of if statement for if the status code is "200 OK"
            else//If the status code is not "200 OK", send the webpage directly to the client
            {
                n = write(newsockfd, buffer1, strlen(buffer1));
				printf("\nHTTP status code is not 200. Page is not cached. Information is directly sent tot he client.\n");

                bzero(buffer1,10000);
                bzero(buffer,256);
            }
          }
        }//End inner while loop

	return 0;
}

char* sendRequest(char inputValueBuffer[], int *valueIsUpdated)
{
    struct addrinfo hints, *results;
    //struct sockaddr_in *ip_access_temp;
    int checkValue;
    int sockfd;
	char *user_input;
	static char buffer[10000];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    bzero(buffer,10000);

    if ((checkValue = getaddrinfo(inputValueBuffer , "80" , &hints , &results)) != 0)
    {
        return inputValueBuffer;
    }

    sockfd = socket(results->ai_family, results->ai_socktype, results->ai_protocol);

    connect(sockfd, results->ai_addr, results->ai_addrlen);
    send(sockfd, "GET / HTTP/1.1\r\n\r\n",23, 0);

    int recv_length = 1;
    recv_length = recv(sockfd, &buffer, 10000, 0);

    return buffer;
}
