#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>

#include "ntpcalculations.c"
#include "communication.c"
#include "ntplogic.h"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Parameters: hostname, port, operation, key");
        exit(1);
    }

    // Amout Requests
    int n = atoi(argv[1]);

    int nserver = argc-2;
    char *ntp_server[nserver];
    char *port = "123";
    //timespec dispersion[NUM];

    // fill serverarray with values
    for (int i = 2; i < argc ; ++i) {
        ntp_server[i - 2] = argv[i];
    }

    for(int server_index = 0; server_index < nserver; server_index++){

        // parse command line arguments
        int status;
        struct addrinfo *servinfo, hints, *p;
        socklen_t addrlen = sizeof(struct sockaddr_storage);

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        //hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        //get address info
        if ((status = getaddrinfo(ntp_server[server_index], port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            exit(1);
        }

        // loop through result and try to connect to the first possible
        int socketfd;
        for (p = servinfo; p!= NULL; p = p->ai_next) {
            if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("Could not establish client socket");
                continue;
            }

            if(connect(socketfd, p->ai_addr, p->ai_addrlen) == -1 ) {
                close(socketfd);
                perror("Client could not connect");
                continue;
            }
            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            exit(1);
        }

        //logic
        ntp_logic(n, ntp_server[server_index], socketfd, p, addrlen);

        freeaddrinfo(servinfo);
        close(socketfd);
    }

    return 0;
}



