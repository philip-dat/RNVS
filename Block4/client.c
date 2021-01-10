#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "communication.h"
#include <unistd.h>

int main(int argc, char* argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Parameters: hostname, port, operation, key");
        exit(1);
    }

    // parse command line arguments
    int status;
    struct addrinfo *servinfo, hints, *p;
    char* host = argv[1];
    char* port = argv[2];
    char* operation = argv[3];
    char* key = argv[4];
   
   // allocate memory for message
    message *msg = message_template();
    set_message(key, operation, msg);

    // fwrite(msg->value, 1, msg->head->value_length, stdout);

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    //get address info
    if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        free_message(msg);
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
        free_message(msg);
        exit(1);
    }

    //send message to server in intervals
    send_message(msg, socketfd);

    // free memory for msg
    free_message(msg);

    // receive reply
    message *reply = receive_message(socketfd, 1);
    handle_reply_client(reply);

    free_message(reply);
    freeaddrinfo(servinfo);
    close(socketfd);
    return 0;
}