#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "communication.h"

int main(int argc, char* argv[]){
    int socketfd, new_fd;
    struct addrinfo hints, *servinfo, *p;

    if (argc != 2) {
        fprintf(stderr, "Parameter: port");
        exit(1);
    }

    char *port = argv[1];
    // init empty hashtable
    entry *hash_table = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if(getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
        fprintf(stderr, "Error on getaddrinfo");
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socketfd = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        // reuse address for debugging
        int yes = 1;
        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        //bind to socket
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    //listen to incoming messages from client
    if (listen(socketfd, 2) == -1) {
        perror("listen");
        exit(1);
    }

    // infinite loop
    while (1) {

            struct sockaddr_storage their_addr;
            socklen_t addr_size = sizeof their_addr;
            new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);

            //receive message
            message *msg = receive_message(new_fd);

            // select operation and send reply message
            message *reply;

            char *op = parse_operation(msg);

            if(strncmp(op,"GET", 3) == 0) {
                reply = get(&hash_table, msg);
            } else if(strncmp(op,"DELETE", 6) == 0) {
                reply = delete(&hash_table, msg);
            } else if(strncmp(op,"SET", 3) == 0) {
                reply = set(&hash_table, msg);
            } else {
                //No valid operation given
                fprintf(stderr,"NOT AN OPERATION\n");
                exit(1);
            }

            send_message(reply, new_fd);

            // free used space
            free_message(msg);
            free_message(reply);

            // close connection
            close(new_fd);
    }
}