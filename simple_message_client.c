/**
 * @file simple_message_client.c
 * server process of tcp/ip
 *
 * @author Andre Schneider <ic20b106@technikum-wien.at>
 * @author Emir Sinanovic <ic17b032@technikum-wien.at>
 * @date 2020/12/12
 *
 * @version 1 
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <simple_message_client_commandline_handling.h>

/**
 * \brief gets ipv4/ipv6 address from a socket address structure
 *
 * \param sa socketaddress to get ip from
 * 
 * \return A Void pointer to the ip address
 * \retval void* to the ip address
 */
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * \brief print usage message and exists with given exit code
 *
 * \param stream Output stream to print usage on
 * \param cmnd the execution command
 * \param exitcode code with which the program exists after printing the usage
 */
void usage(FILE* stream, const char* cmnd, int exitcode) {
	fprintf(stream, "usage: %s\n", cmnd);
	fprintf(stream, "options:\n");
	fprintf(stream, "\t-s, --server <server>\tfull qualified domain name or IP address of the server\n");
	fprintf(stream, "\t-p, --port <port>\twell-kown port of the server [0..65535]\n");
	fprintf(stream, "\t-u, --user <name>\tname of the posting user\n");
	fprintf(stream, "\t-i, --image <URL>\tURL pointing to an image of the posting user\n");
	fprintf(stream, "\t-m, --message <message>\tmessage to be added to the bulletin board\n");
	fprintf(stream, "\t-v, --verbose\tverbose output\n");
	fprintf(stream, "\t-h, --help\n");
	exit(exitcode);
}

/**
 *
 * \brief The main algorithm that connects to the server and sends the data
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return returns errorcode
 * \retval 0 for success else error
 */
int main(int argc, char* argv[]) { 

	const char* server = NULL;
	const char* port = NULL;
	const char* user = NULL;
	const char* message = NULL;
	const char* img_url = NULL;
	int verbose;

	smc_parsecommandline(argc, (const char *const *)argv, usage, &server, &port, &user, &message, &img_url, &verbose);

	int sockfd;
	int numbytes;
	
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof hints);    
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version    
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	
	if (verbose != 0) {printf("Getting AddrInfo");}
	if ((rv = getaddrinfo(server, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);

	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	size_t data_length = 11 + strlen(user) + strlen(message);

	if (img_url != NULL) {
		data_length += 5 + strlen(img_url);
	}

	char data[data_length];

	strcpy(data, "user=");
	strcat(data, user);
	strcat(data, "\n");
	if (img_url != NULL) {
		strcat(strcat(data, "img="), img_url);
		strcat(data, "\n");
	}
	strcat(data, message);
	strcat(data, "\n:\n:\n");

	if ((numbytes = send(sockfd, data, sizeof(data) - 1, 0) == -1)) {
		perror("Couldn't send data");
		exit(1);
	}
	close(sockfd);
	return 0;
}


/*
 * =================================================================== eof == */
