

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXDATASIZE 512 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    if (argc != 3) {
        fprintf(stderr,"usage: hostname/IP Port\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    //checks spelling mistakes of hostname/IP-address
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        //creates socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            fprintf(stderr,"client: socket");
            continue;
        }

        //connects to this socket
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            fprintf(stderr, "client: connect");
            continue;
        }

        break;
    }

    //if there is no server found to connect to, error
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    //free memory of server struct
    freeaddrinfo(servinfo); // all done with this structure

    //save received message
    int data_length = MAXDATASIZE-1;
    char chunk[MAXDATASIZE-1];

    while(data_length == MAXDATASIZE-1) {
        memset(chunk, 0, MAXDATASIZE-1);
        if ((data_length = recv(sockfd,  chunk, MAXDATASIZE-1, 0)) == -1) {
            fprintf(stderr, "recv");
            exit(1);
        } else {
            if (data_length > 0){
                for(int c=0;c<data_length; c++){
                    if(chunk[c]=='\0'){
                        printf("\%c", chunk[c]);
                    } else{
                        printf("%c",chunk[c]);
                    }
                }
            }
        }
    }
    //close connection via socket
    close(sockfd);

    return 0;
}