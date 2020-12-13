/**
 * @file simple_message_server.c
 * server process of tcp/ip
 *
 * @author Andre Schneider <ic20b106@technikum-wien.at>
 * @date 2020/12/12
 *
 * @version 1 
 */

/*
 * -------------------------------------------------------------- includes --
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>
#include <assert.h>
/*
 * --------------------------------------------------------------- defines --
 */
#define BACKLOG 10 // how many pending connections queue will hold

/**
 * \brief handels dead child processes
 *
 * \param s is ignored
 */
void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

/**
 * \brief gets ipv4/ipv6 address from a socket address structure
 *
 * \param sa socketaddress to get ip from
 * 
 * \return A Void pointer to the ip address
 * \retval void* to the ip address
 */
void* get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 * \brief print usage message and exists with given exit code
 *
 * \param command the execution command
 * \param exit_code code with which the program exists after printing the usage
 */
void usage(char command[], int exit_code) {
    printf("usage: %s option\n", command);
    printf("options:\n");
    printf("\t-p, --port <port>\n");
    printf("\t-h, --help\n");
    exit(exit_code);
}

/**
 * \brief parses given command line arguments and acts upon them
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return A char Pointer to the port string
 * \retval char* to the port string
 */
char* parse_command_line(int argc, char* argv[]) {
    char* port;

    static struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"port", required_argument, NULL, 'p'},
        {NULL, 0, NULL, 0}
    };

    int ch;
    while((ch = getopt_long(argc, argv, "hp:", long_options, NULL)) != -1) {  
        switch(ch) {  
          case 'h':
            usage(argv[0], 0);
            break;
          case 'p':  
            port = optarg;
            break;
          case ':': 
            usage(argv[0], 1);
            break;
          case '?':
            usage(argv[0], 1);
            break;
          default:
            assert(0);
        }  
    }  
    
    for(; optind < argc; optind++){
        printf("unkown argument '%s'\n", argv[optind]);     
        usage(argv[0], 1);
    } 

    return port;
}

/**
 *
 * \brief The main algorithm that accept and passes on clients to the server logic
 *
 * This is the main entry point for any C program.
 *
 * \param argc the number of arguments
 * \param argv the arguments itselves (including the program name in argv[0])
 *
 * \return always "success"
 * \retval 0 always
 */
int main(int argc, char* argv[]) {
    char* PORT = parse_command_line(argc, argv);
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");
    while(1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
        printf("server: got connection from %s\n", s);
        int p = fork();
        if (p == -1) {
            perror("Forking failed!\n");
            close(new_fd);
        } else if (p == 0) { // this is the child process
            char* logic_args[] = {"./simple_message_server_logic", NULL};

            dup2(new_fd, STDIN_FILENO);
            dup2(new_fd, STDOUT_FILENO);
            close(new_fd);
            
            if (execl(logic_args[0], "simple_message_server_logic", (char*)NULL) == -1) {
                perror("Error executing server logic\n");
            }

            close(new_fd);

            exit(0);
        } else {
            close(new_fd);
        }
    }
    return 0;
}
/*
 * =================================================================== eof ==
 */