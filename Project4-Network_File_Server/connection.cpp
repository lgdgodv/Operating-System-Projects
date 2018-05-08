// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "fs_crypt.h"
#include <netdb.h>        // gethostbyname(), struct hostent
#include <netinet/in.h>    // struct sockaddr_in
#define PORT 8000

int main(int argc, char const *argv[])
{
	struct sockaddr_in address;
	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char buffer[1024] = {0};
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

  char* user = "user1";
	char* return_string = "FS_SESSION 0 0";
	unsigned int return_size = 0;
	char* clear_header = "user1 ";
	char* password = "password1";

	//char* return_string_buf = static_cast<char *>(fs_encrypt(password, (void*)return_string, strlen(return_string), &return_size));
	char* temp;
	sprintf(temp, "%d", return_size);
	strcat(clear_header, temp);
	printf("%s\n", clear_header);
	//clear_header = clear_header + temp + '\0';

	send(sock , clear_header, strlen(clear_header) , MSG_NOSIGNAL);
	printf("Hello message sent\n");
	valread = read( sock , buffer, 1024);
	printf("%s\n",buffer );
	return 0;
}
