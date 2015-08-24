#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define IRC_HOST "irc.canternet.org"
#define IRC_PORT "6667"
#define IRC_NICK "AppleDasht"

/* Sends data to the given socket, exiting the program if there was an error. */
void mysend(int sock, const char *data, int data_length) {
	int res;

	if ((res = send(sock, data, data_length, 0)) == -1) {
		printf("Failed to send data!\n");
		exit(1);
	}
}

/* Like printf(), except it sends to a socket instead of printing to the console. */
void sendf(int sock, char *fmt, ...) {
	va_list args;
	char buffer[513];

	va_start(args, fmt);
	vsnprintf(buffer, 513, fmt, args);

	printf("<-- %s", buffer);

	mysend(sock, buffer, strlen(buffer));
}


/* Receives a line from an IRC server socket, using a very dumb method of reading
 * one character/byte at a time
 */
char *very_dumb_recv(int sock) {
	char *str = NULL;
	char c;
	int length = 0;

	while (1) {
		if (recv(sock, &c, 1, 0) == -1) { /* Perform the actual recv, reading one byte into our char variable `c`. `recv()` will block (wait at this line of code) until we get a byte on the socket. */
			/* If recv() returned -1, some error happened. Just exit. */
			printf("An error occured receiving!\n");
			exit(1);
		}

		if (c == '\n') { /* If we've hit a newline, we know we have received an entire IRC line, so we leave our infinite loop. */
			break;
		}

		length++; /* Since we just received a character (byte), increase the number of characters we've received */

		str = (char *)realloc(str, length + 1); /* Reallocate the buffer containing the received line, to hold the number of bytes that we've now received, leaving one extra for the null terminator. */

		str[length - 1] = c; /* Append the character to one before end of our newly reallocated buffer, overwriting the old null (byte 0) terminator */
		str[length] = 0; /* Set the null terminator at the end of our buffer so C knows where our string ends. */
	}

	return str; /* Return the entire line to the caller once we leave the loop. */
}

int main(int argc, char *argv[]) {
	int res;
	int sock;
	char *line;
	struct addrinfo *p;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	char curip[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));	

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((res = getaddrinfo(IRC_HOST, IRC_PORT, &hints, &servinfo)) != 0) {
		printf("Some error occured: %s\n", gai_strerror(res));
		return 1;
	}

	inet_ntop(servinfo->ai_family, &(((struct sockaddr_in *)servinfo->ai_addr)->sin_addr), curip, sizeof(curip));
	printf("IP address for the IRC server is: %s\n", curip);

	if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		printf("Failed to create the socket!\n");
		freeaddrinfo(servinfo);
		return 1;
	}

	printf("Created socket, the file descriptor is %d.\n", socket);

	if ((res = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) {
		printf("An error occured connecting!\n");
		freeaddrinfo(servinfo);
		return 1;
	}

	freeaddrinfo(servinfo);

	printf("Connected!\n");

	sendf(sock, "NICK %s\r\n", IRC_NICK);
	sendf(sock, "USER %s * * :%s\r\n", IRC_NICK, IRC_NICK);

	while (1) {
		line = very_dumb_recv(sock);

		printf("--> %s\n", line);

		free(line);
	}

	return 0;
}
