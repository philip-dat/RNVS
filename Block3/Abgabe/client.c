#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "communication.h"
#include <unistd.h>

int main(int argc, char* argv[]) {

    if (argc != 5) {
        fprintf(stderr, "Parameters: hostname, port, command_bits, key");
        exit(1);
    }

    //useful variables, we need later
    struct addrinfo *servinfo, hints, *p;

    //parameter into variables
    char* host = argv[1];
    char* port = argv[2];
    char* command = argv[3];
    char* key = argv[4];

    // allocate memory for new message to send
    message *message_for_server = create_new_message();
    create_message_content(key, command, message_for_server);

    fwrite(message_for_server->value, 1, message_for_server->meta_data->len_of_value, stdout);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    //get address info: If wrong, put error
    int status;
    if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        free_message(message_for_server);
        exit(1);
    }

    //get the first possible connection
    int socketfd;
    for (p = servinfo; p!= NULL; p = p->ai_next) {
        if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("socket wasnt created");
            continue;
        }

        if(connect(socketfd, p->ai_addr, p->ai_addrlen) == -1 ) {
            close(socketfd);
            perror("CLient: connect failed");
            continue;
        }
        break;
    }

    //if no connection was possible
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        free_message(message_for_server);
        exit(1);
    }

    //transfer message and free storage afterwards
    send_message(message_for_server, socketfd);
    free_message(message_for_server);

    //get an answer from server
    message *reply = receive_message(socketfd);
    handle_reply_from_server(reply);

    //answer must be freed
    free_message(reply);
    freeaddrinfo(servinfo);

    //shutdown connection
    close(socketfd);
    return 0;
}
