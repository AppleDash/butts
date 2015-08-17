#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

void mysend(int sock, const char *data, int data_length) {
	int res;

	if ((res = send(sock, data, data_length, 0)) == -1) {
		printf("Failed to send data!\n");
		exit(1);
	}
}

char *very_dumb_recv(int sock) {
	char *str = NULL;
	char c;
	int length = 0;

	while (1) {
		if (recv(sock, &c, 1, 0) == -1) {
			printf("An error occured receiving!\n");
			exit(1);
		}

		if (c == '\n') {
			break;
		}

		length++;

		str = (char *)realloc(str, length + 1);

		str[length - 1] = c;
		str[length] = 0;
	}

	return str;
}

int main(int argc, char *argv[]) {
	int res;
	int sock;
	char *line;
	struct addrinfo *p;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	char curip[INET_ADDRSTRLEN];

	const char *handshake = "NICK AppleDasht\r\nUSER AppleDasht * * :AppleDasht\r\n";

	memset(&hints, 0, sizeof(hints));	

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((res = getaddrinfo("irc.canternet.org", "6667", &hints, &servinfo)) != 0) {
		printf("Some error occured: %s\n", gai_strerror(res));
		return 1;
	}

	inet_ntop(servinfo->ai_family, &(((struct sockaddr_in *)servinfo->ai_addr)->sin_addr), curip, sizeof(curip));
	printf("IP address for canternet is: %s\n", curip);

	if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		printf("Failed to create the socket!\n");
		return 1;
	}



	printf("Created socket, the file descriptor is %d.\n", socket);

	if ((res = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		printf("An error occured connecting!\n");
		return 1;
	}

	freeaddrinfo(servinfo);

	printf("Connected!\n");


	mysend(sock, handshake, strlen(handshake));


	while (1) {
		line = very_dumb_recv(sock);

		printf("%s\n", line);

		free(line);
	}

	return 0;
}