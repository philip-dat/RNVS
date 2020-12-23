/*
** server.c -- a stream socket server demo
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
#include <time.h>
#include <limits.h>

#define BACKLOG 10	 // how many pending connections queue will hold


char *getrandomline(char *filename, int linecounter, ssize_t doclen);

void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

struct linesnddoclen{
    int lines;
    ssize_t doclen;
};

typedef struct linesnddoclen Struct;

Struct countlines(char *filename);

int main(int argc, char* argv[])
{
    //checking for passing enough arguments
    if(argc != 3){
        printf("error: usage: port file\n");
        return 1;
    }

    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
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

    char* PORT = argv[1];


    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            fprintf(stderr,"server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            fprintf(stderr,"setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            fprintf(stderr,"server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        fprintf(stderr,"listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr,"sigaction");
        exit(1);
    }

    Struct lenNlines = countlines(argv[2]);
    ssize_t doclen = lenNlines.doclen;
    int linecounter = lenNlines.lines;

    while(1) {  // main accept() loop
        char* buffer = getrandomline(argv[2], linecounter, doclen);
        size_t len = strlen(buffer);

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            fprintf(stderr,"accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        if (send(new_fd, buffer, len, 0) == -1) {
            fprintf(stderr, "send");
            close(new_fd);
            free(buffer);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
        free(buffer);
    }
}

char* getrandomline(char  * filename, int linecounter, ssize_t doclen){
    FILE *fptr = fopen(filename, "r");
    size_t buffer_size = 0;
    char *buffer = NULL;

    if (!fptr) {
        perror ("File open error!\n");
        exit(1);
    }

    //get random number
    srand(time(0));
    int randomnr = rand() % (linecounter + 1);

    //get sentence from rnd line number
    int index = 0;
    ssize_t len = 0;
    char *concatstring = calloc(doclen, sizeof(char));

    while((len = getline(&buffer, &buffer_size, fptr)) > 0) {
        if(index == randomnr) {
            memcpy(concatstring, buffer, buffer_size);
            while(!strchr(buffer,'\n')){
                buffer = NULL;
                len += getline(&buffer, &buffer_size, fptr);
                snprintf(concatstring,doclen * sizeof (char), "%s",buffer);
            }
            break;
        }
        buffer = NULL;
        index++;
    }
    free(buffer);

    fclose(fptr);
    char *buffer_no_lf = calloc(len,sizeof(char));
    memcpy(buffer_no_lf, concatstring, len - 1);
    buffer_no_lf[len - 1] = '\0';
    free(concatstring);
    return buffer_no_lf;
}


Struct countlines(char *filename) {
    // count the number of lines in the file called filename
    FILE *fp = fopen(filename,"r");
    char *buffer = NULL;
    size_t buffer_size = 0;

    int lines=0;

    if (fp == NULL){
        perror("File open error or empty file\n");
        exit(1);
    }

    lines++;
    ssize_t doclen = 0;
    ssize_t len = 0;
    while((len = getline(&buffer, &buffer_size, fp)) > 0) {
        doclen += len;
        if(strchr(buffer, '\n')) {
            lines++;
        }
        buffer = NULL;
    }

    fclose(fp);
    free(buffer);

    Struct s;
    s.lines = lines;
    s.doclen = doclen;

    if( lines == 0){
        perror("File is empty or no LF in file\n");
        exit(1);
    }
    return s;
}