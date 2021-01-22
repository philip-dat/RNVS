#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>

#include "inc/ntplogic.h"

int main(int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Parameters: hostname, port, operation, key");
        exit(1);
    }

    // Amout Requests
    int n = atoi(argv[1]);

    int nserver = argc-2;
    char *ntp_list[nserver];
    char *port = "123";
    //timespec dispersion[NUM];

    // fill serverarray with values
    for (int i = 2; i < argc ; ++i) {
        ntp_list[i - 2] = argv[i];
    }

    int sockfd;
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    for(int server_index = 0; server_index < nserver; server_index++){
        struct addrinfo hints = {0};
        hints.ai_family       = AF_INET;
        hints.ai_socktype     = SOCK_DGRAM;

        struct addrinfo *ntp_server;

        int status;
        if ((status = getaddrinfo(ntp_list[server_index], port, &hints, &ntp_server)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
            exit(EXIT_FAILURE);
        }

        //logic
        ntp_logic(n, ntp_list[server_index], sockfd, ntp_server);

        freeaddrinfo(ntp_server);
        close(sockfd);
    }

    return 0;
}



