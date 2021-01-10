#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include "communication.h"

int main(int argc, char* argv[]){
    struct addrinfo hints, *servinfo, *p;

    if (argc != 2) {
        fprintf(stderr, "Parameter: port");
        exit(1);
    }

    // initialise our hashtable for saving entries
    entry *hash_table = NULL;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char *port = argv[1];
    if(getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
        fprintf(stderr, "Error on getaddrinfo");
        exit(1);
    }
    int socketfd;
    // bind to the first possible result
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((socketfd = socket(p->ai_family, p->ai_socktype,
                               p->ai_protocol)) == -1) {
            fprintf(stderr,"server: socket");
            continue;
        }

        // reuse address for debugging
        int temp = 1;
        setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(int));

        //bind server to socket
        if (bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(socketfd);
            fprintf(stderr,"server: bind");
            continue;
        } break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    //listen on created socket for client
    if (listen(socketfd, 2) == -1) {
        perror("listen");
        exit(1);
    }

    int new_fd;


    while (1) {

        struct sockaddr_storage their_addr;
        socklen_t addr_size = sizeof their_addr;
        new_fd = accept(socketfd, (struct sockaddr *)&their_addr, &addr_size);

        //receive message from client
        message *recvd_message = receive_message(new_fd);

        // check which command_bits in header
        char *command = get_command(recvd_message);
        message *reply_message;

        //get, set or delete entry from hashtable
        if(strncmp(command, "GET", 3) == 0) {
            reply_message = get_entry(&hash_table, recvd_message);
        } else if(strncmp(command, "DELETE", 6) == 0) {
            reply_message = delete_entry(&hash_table, recvd_message);
        } else if(strncmp(command, "SET", 3) == 0) {
            reply_message = set_entry(&hash_table, recvd_message);
        } else {
            fprintf(stderr, "not a possible command_bits");
            exit(1);
        }

        //reply_message client, if received a valid message and everything was saved/gotten/deleted correctly
        send_message(reply_message, new_fd);

        // free storage of received and replied message
        free_message(recvd_message);
        free_message(reply_message);

        // shutdown connection
        close(new_fd);
    }
}