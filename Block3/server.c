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
#include <unistd.h>
#include "msg.h"
#include "uthash.h"


typedef struct record {
    char *key;
    char *value;
    uint32_t value_length;
    uint16_t key_length;
    UT_hash_handle hh;
} record;

record *add_record(record **table, message *msg) {
    record *new_record = malloc(sizeof(record));
    new_record->key_length = msg->head->key_length;
    new_record->value_length = msg->head->value_length;
    new_record->key = malloc(sizeof(char) * new_record->key_length + 1);
    new_record->value = malloc(sizeof(char) * new_record->value_length);
    memcpy(new_record->key, msg->key, new_record->key_length);
    memcpy(new_record->value, msg->value, new_record->value_length);
    new_record->key[new_record->key_length] = '\0';
    HASH_ADD_KEYPTR(hh, *table, new_record->key, new_record->key_length, new_record);
    return new_record;
}

message *set(record **table, message *msg){
    record *h = NULL;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);
    if(h == NULL) {
        h = add_record(table, msg);
    }
    // else update the value
    if(msg->head->value_length != h->value_length) {
        h->value_length = msg->head->value_length;
        h-> value = realloc(h->value, h->value_length);
        memcpy(h->value, msg->value, h->value_length);
    }

    // craft and return the reply
    message *reply = empty_message();
    reply->head->operation = msg->head->operation;
    reply->head->operation |= 1 << 3; //set the acknowledgement bit
    return reply;

}

message *delete(record **table, message *msg){
    record *h;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);
    message *reply = empty_message();
    reply->head->operation =  msg->head->operation;
    if (h == NULL) {
        char * err = "Record does not exist";
        reply->head->value_length = strlen(err);
        reply->value = realloc(reply->value, reply->head->value_length);
        memcpy(reply->value, err, reply->head->value_length);
        return reply;
    }
    HASH_DEL(*table, h);
    free(h->value);
    free(h->key);
    free(h);
    reply->head->operation |= 1 << 3; //set the acknowledgement bit
    return reply;
}

message *get(record **table, message *msg) {
    record *h;
    message *reply = empty_message();
    reply->head->operation = msg->head->operation;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);

    // No record found, message contains error
    if(h == NULL) {
        char * err = "Record does not exist";
        reply->head->value_length = strlen(err);
        reply->value = realloc(reply->value, reply->head->value_length);
        memcpy(reply->value, err, reply->head->value_length);
        return reply;
    }

    reply->head->operation |= 1 << 3; //set the acknowledgement bit
    reply->head->value_length = h->value_length;
    reply->head->key_length = h->key_length;
    reply->value = malloc(sizeof(char) * reply->head->value_length);
    reply->key = malloc(sizeof(char) * reply->head->key_length);
    memcpy(reply->value, h->value, h->value_length);
    memcpy(reply->key, h->key, h->key_length);
    return reply;
}

int main(int argc, char* argv[]){
    int socketfd, new_fd;
    struct addrinfo hints, *servinfo, *p;

    if (argc != 2) {
        fprintf(stderr, "Please provide a suitable port");
        exit(1);
    }

    char *port = argv[1];
    // init empty hashtable
    record *hash_table = NULL;

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
            message *rec_msg = receive_message(new_fd);

            // handle operation and send reply message
            message *reply;
            char *op = parseOperation(rec_msg);
            if(strncmp(op,"GET", 3) == 0) {
                reply = get(&hash_table, rec_msg);
            } else if(strncmp(op,"DELETE", 6) == 0) {
                reply = delete(&hash_table, rec_msg);
            } else if(strncmp(op,"SET", 3) == 0) {
                reply = set(&hash_table, rec_msg);
            } else {
                fprintf(stderr,"Unknown operation");
            }
            send_message(reply, new_fd);
            free_message(rec_msg);
            free_message(reply);

            // close connection
            close(new_fd);

    }

    close(socketfd);

}