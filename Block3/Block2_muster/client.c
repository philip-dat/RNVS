#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>


int main(int argc, char** argv) {
    if (argc < 3){
        fprintf(stderr,"%s\n", "No enough args provided!");
        return 1;
    }

    char* host = argv[1];
    char* service = argv[2];


    struct addrinfo hints;
    struct addrinfo* res;
    struct addrinfo* p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    int status = getaddrinfo(host, service, &hints, &res);
    if (status != 0){
        fprintf(stderr, "%s\n", "getaddrinfo() failed!");
        return 1;
    }

    int s = -1;
    for(p = res; p != NULL;p = p->ai_next){
        s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s == -1){
            continue;
        }
        status = connect(s, res->ai_addr, res->ai_addrlen);
        if (status == -1){
            close(s);
           fprintf(stderr, "%s\n", "connect() failed!");
           continue;
        }

        break;
    }

    if (s == -1){
        fprintf(stderr, "%s\n", "socket() failed!");
        return 1;
    }

    if(status != 0) return 1;


    fprintf(stderr, "Connected to %s:%s\n", host, service);


    while(true){
        int nbytes = 0;
        char buffer[512];

        nbytes = recv(s, buffer, 512, 0);
        if (nbytes == 0){
            break;
        }
        unsigned long written = 0;
        while (written != nbytes){
            unsigned long n = fwrite(buffer + written, sizeof(char), nbytes - written, stdout);

            if (n == 0){
                fprintf(stderr, "%s\n", "Write to stdout failed!");
                return 1;
            }

            written += n;
        }

    }

    return 0;
}