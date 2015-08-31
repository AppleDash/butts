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
	int bytes_sent;

	bytes_sent = 0;

	/* send() can sometimes not send all of our bytes, but it handily returns how many it actually sent.
	 * We just keep retrying send() with any bytes that didn't get send until we've sent it all.
	 */
	do {
		res = send(sock, data, data_length - bytes_sent, 0);
		if (res == -1) {
			printf("Failed to send data!\n");
			exit(1); /* We just exit for now, better error handling comes later */
		}
		bytes_sent += res;
	} while (bytes_sent < data_length);
}

/* Like printf(), except it sends to a socket instead of printing to the console. */
void sendf(int sock, char *fmt, ...) {
	va_list args;
	char buffer[513];

	/* Do some fun stuff with varargs functions to emulate the behaviour of
	 * printf() or sprintf() or similar.
	 * This just replaces %s and %d etc in the format string with the arguments provided.
	 */
	va_start(args, fmt);
	vsnprintf(buffer, 513, fmt, args);

	printf("<-- %s", buffer);

	/* Call our mysend() function above to handle sending the data to the socket. */
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
	int sock; /* This will be our socket file descriptor, which is really just a number. */
	char *line;
	struct addrinfo *p;
	struct addrinfo hints; /* Hints struct to tell getaddrinfo() what we want from it. */
	struct addrinfo *servinfo; /* Pointer to a struct (technically a linked list of structs) which will contain the results of getaddrinfo() */
	char curip[INET_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints)); /* Zero out the hints struct to make sure it contains no garbage, which it may since it was allocated on the stack. */

	/* Fill out the hints struct. */
	hints.ai_family = AF_INET; /* Internet protocol */
	hints.ai_socktype = SOCK_STREAM; /* TCP streaming sockets */
	hints.ai_flags = AI_PASSIVE; /* The kernel will fill in our local IP for us. */

	if ((res = getaddrinfo(IRC_HOST, IRC_PORT, &hints, &servinfo)) != 0) { /* Attempt to resolve our IRC server hostname to an IP address and fill it into the servinfo struct along with the IRC server port and socket() params. */
		/* getaddrinfo() returned not 0, which indicates an error. */
		printf("Some error occured: %s\n", gai_strerror(res));
		return 1;
	}

	/* Conver the IP address represented as 4 bytes into a human readable dotted string, for printing. */
	inet_ntop(servinfo->ai_family, &(((struct sockaddr_in *)servinfo->ai_addr)->sin_addr), curip, sizeof(curip));
	printf("IP address for the IRC server is: %s\n", curip);

	if ((sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) { /* Create our socket with the address family, socket type, and protocol family given to us in the results struct. (We chose these when filling out the hints struct.) */
		/* socket() returned -1, error occured. */
		printf("Failed to create the socket!\n");
		freeaddrinfo(servinfo);
		return 1;
	}

	printf("Created socket, the file descriptor is %d.\n", sock);

	if ((res = connect(sock, servinfo->ai_addr, servinfo->ai_addrlen)) == -1) { /* Actually connect our socket to the IRC server, using the IP address and port in our servinfo struct. */
		/* socket() returned -1, which indicates error. */
		printf("An error occured connecting!\n");
		freeaddrinfo(servinfo);
		return 1;
	}

	freeaddrinfo(servinfo); /* free the servinfo struct/list, which we no longer need here.*/

	printf("Connected!\n");

	/* From here, we just send our IRC handshake and then receive and print out lines infinitely. */
	sendf(sock, "NICK %s\r\n", IRC_NICK);
	sendf(sock, "USER %s * * :%s\r\n", IRC_NICK, IRC_NICK);

	while (1) {
		line = very_dumb_recv(sock);

		printf("--> %s\n", line);

		free(line);
	}

	return 0;
}
