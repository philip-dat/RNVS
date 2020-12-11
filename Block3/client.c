#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "msg.h"
#include "uthash.h"
#include <ctype.h>
#include <unistd.h>

#define BUF_SIZE 5

void lower_case_string(char *string){
    for(int i=0; string[i]; i++){
        string[i] = tolower(string[i]);
    }
}

uint8_t set_operation_bit(char *operation) {
    char get[] = "get", set[] = "set", del[] = "delete";

    lower_case_string(operation);

    uint8_t operation_bit = 0b00000000;
    if(strcmp(operation,get)==0){
        operation_bit |= 1 << 2;            //Drittes Bit -> GET
    }else if(strcmp(operation,set)==0){
        operation_bit |= 1 << 1;            //Zweites Bit -> SET
    }else if(strcmp(operation,del)==0){
        operation_bit |= 1 << 0;            //Erstes Bit -> DEL
    }else {
        printf("Error on Operation Input\n");
        exit(1);
    }
    return operation_bit;
}

void get_value_from_stdin(message *msg){
    char buffer[BUF_SIZE];
    size_t chunk = 0;
    while( (chunk = fread(buffer, sizeof(char), BUF_SIZE, stdin)) > 0) {
        int endoffile = msg->head->value_length;
        msg->head->value_length += chunk;
        msg->value = realloc(msg->value, msg->head->value_length * sizeof(char));
        memcpy(&(msg->value[endoffile]), buffer, chunk);
    }
}

void set_message(char *key, char *operation, message *msg){
    msg->head->key_length = strlen(key);
    msg->key = malloc(msg->head->key_length);
    memcpy(msg->key, key, strlen(key));
    msg->head->operation = set_operation_bit(operation);
    msg->value = malloc(sizeof(char) * BUF_SIZE);

    if(strncmp(parseOperation(msg),"SET", 3) == 0) {
        get_value_from_stdin(msg);
    }
}


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
    message *msg = empty_message();
    set_message(key, operation, msg);

    fwrite(msg->value, 1, msg->head->value_length, stdout);

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
    message *reply = receive_message(socketfd);
    handle_reply(reply);

    free_message(reply);
    freeaddrinfo(servinfo);
    close(socketfd);
    return 0;
}