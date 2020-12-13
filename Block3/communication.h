#ifndef BLOCK_3_MSG_H
#define BLOCK_3_MSG_H

#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include "uthash.h"

#define BUF_SIZE 5

// begin structs

typedef struct header {
    uint8_t operation;
    uint16_t key_length;
    uint32_t value_length;
} header;

typedef struct message {
    header *head;
    char *key;
    char *value;
} message;

typedef struct entry {
    char *key;
    char *value;
    uint32_t value_length;
    uint16_t key_length;
    UT_hash_handle hh;
} entry;

// end structs

// begin functions for communication

char* parse_operation(message *msg){
    if((msg->head->operation >> 0 & 1) ==1) {
        return "DELETE";
    } else if ((msg->head->operation >> 1 & 1) == 1) {
        return "SET";
    } else if ((msg->head->operation >> 2 & 1) == 1) {
        return "GET";
    }
    return "ERROR";
}

int read_message ( int sock, char *buff, size_t bufflen, size_t *buffused ) {
    int result = 0;
    *buffused = 0;

    // Loop until the buffer is full, or a recv result
    // indicates there is a problem of one sort or another
    while ( *buffused < bufflen ) {
        int n = recv(sock, &buff[*buffused], 1, 0);
        if ( n > 0 ) {
            // success
            ++(*buffused);
            result = 1;
        } else if ( n < 0 ) {
            if ( errno == EAGAIN || errno == EWOULDBLOCK ) {
                // This is expected on a non-blocking socket,
                // so just return with what's available at the
                // moment.
                result = 1;
            } else {
                // Everything else is a hard error.
                // Do something with it in the caller.
                result = -errno;
            }
            break;
        } else {  // n == 0
            // closed
            result = 0;
            break;
        }
    }

    return result;
}

int write_message(char *buff, size_t bufflen, size_t *buffused){
    int result = 0;
    *buffused = 0;

    while(*buffused < bufflen) {
        int n = fwrite(&buff[*buffused], 1, 1, stdout);
        if (n > 0){
            ++(*buffused);
            result = 1;
        }else {
            result = 0;
            break;
        }
    }
    return result;
}

int send_message(message *msg, int socketfd) {
    int bytes_sent = 0;
    if(msg != NULL) {
        bytes_sent = send(socketfd, &(msg->head->operation), sizeof(uint8_t), 0 );
        // convert to network byte order
        uint16_t conv_key_length = htons(msg->head->key_length);
        uint32_t conv_value_length = htonl(msg->head->value_length);

        bytes_sent += send(socketfd, &(conv_key_length), sizeof(uint16_t), 0);
        bytes_sent += send(socketfd, &(conv_value_length), sizeof(uint32_t), 0);
        bytes_sent += send(socketfd, msg->key, sizeof(char) * msg->head->key_length, 0);
        bytes_sent += send(socketfd, msg->value, sizeof(char) * msg->head->value_length, 0);
    }
    return bytes_sent;
}

message *message_template() {
    message *temp= malloc(sizeof(message));
    temp->head = malloc(sizeof(header));
    temp->head->value_length = 0;
    temp->head->key_length = 0;
    temp->value = NULL;
    temp->key = NULL;
    return temp;
}

message *receive_message(int socketfd) {
    // receive header first
    message *msg = message_template();

    recv(socketfd, &(msg->head->operation), sizeof(uint8_t), 0);
    recv(socketfd, &(msg->head->key_length), sizeof(uint16_t), 0);
    recv(socketfd, &(msg->head->value_length), sizeof(uint32_t), 0);

    // convert to host byte order
    msg->head->key_length = ntohs(msg->head->key_length);
    msg->head->value_length = ntohl(msg->head->value_length);

    // receive the rest
    msg->key = malloc(sizeof(char) * msg->head->value_length);
    msg->value = malloc(sizeof(char) * msg->head->value_length);
    recv(socketfd, msg->key, sizeof(char) * msg->head->key_length, 0);

    // if operation is 'GET'
    if (msg->head->value_length > 0) {
        size_t bufferusage = 0;
        read_message(socketfd, msg->value,sizeof(char) * msg->head->value_length, &bufferusage );
        if(bufferusage > 0){
            msg->value[bufferusage] = '\0';
        } else {
            fprintf(stderr,"Something went wrong receiving the message\n");
            exit(1);
        }
    }
    return msg;
}

void *free_message(message *msg) {
    if (msg != NULL){
        if(msg->value != NULL) {
            free(msg->value);
        }
        if(msg->key != NULL) {
            free(msg->key);
        }
        free(msg->head);
        free(msg);
    }
}

// end functions for communication

// begin functions for client

void lower_case_string(char *string){
    for(int i=0; string[i]; i++){
        string[i] = tolower(string[i]);
    }
}

uint8_t set_operation_bit(char *operation) {
    char get[] = "get", set[] = "set", del[] = "delete";

    lower_case_string(operation);

    uint8_t operation_bit = 0b00000000;

    //Third Bit -> GET
    if(strcmp(operation, get) == 0){
        operation_bit |= 1 << 2;
    }//Second Bit -> SET
    else if(strcmp(operation, set) == 0){
        operation_bit |= 1 << 1;
    }//First Bit -> DEL
    else if(strcmp(operation, del) == 0){
        operation_bit |= 1 << 0;
    }else {
        printf("NOT AN OPERATION\n");
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

    if(strncmp(parse_operation(msg),"SET", 3) == 0) {
        get_value_from_stdin(msg);
    }
}

void handle_reply_client(message *reply){
    bool isAck = (((reply->head->operation >> 3) & 1) == 1);
    char * value = reply->value;
    uint32_t value_length = reply->head->value_length;

    if(!isAck) {
        fwrite(value, 1, value_length, stderr);
    } else {
        // if 'GET' operation
        if(value_length != 0) {
            // write value from os buffer
            size_t bufferusage = 0;
            write_message(value, value_length, &bufferusage);

            if(bufferusage <= 0){
                fprintf(stderr, "Something went wrong writing message\n");
                exit(1);
            }
        }
        // if 'SET' operation
        else {
            fprintf(stdout, "DONE");
        }
    }
}

// end functions for client

// begin functions for server

entry *add_entry(entry **table, message *msg) {
    entry *new_entry = malloc(sizeof(entry));
    new_entry->key_length = msg->head->key_length;
    new_entry->value_length = msg->head->value_length;
    new_entry->key = malloc(sizeof(char) * new_entry->key_length + 1);
    new_entry->value = malloc(sizeof(char) * new_entry->value_length);
    memcpy(new_entry->key, msg->key, new_entry->key_length);
    memcpy(new_entry->value, msg->value, new_entry->value_length);
    new_entry->key[new_entry->key_length] = '\0';
    HASH_ADD_KEYPTR(hh, *table, new_entry->key, new_entry->key_length, new_entry);
    return new_entry;
}

message *set(entry **table, message *msg){
    entry *h = NULL;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);
    if(h == NULL) {
        h = add_entry(table, msg);
    }
    // else update the value
    if(msg->head->value_length != h->value_length) {
        h->value_length = msg->head->value_length;
        h-> value = realloc(h->value, h->value_length);
        memcpy(h->value, msg->value, h->value_length);
    }

    // craft and return the reply
    message *reply = message_template();
    reply->head->operation = msg->head->operation;
    reply->head->operation |= 1 << 3; //set the acknowledgement bit
    return reply;

}

message *delete(entry **table, message *msg){
    entry *h;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);
    message *reply = message_template();
    reply->head->operation =  msg->head->operation;
    if (h == NULL) {
        char * err = "";
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

message *get(entry **table, message *msg) {
    entry *h;
    message *reply = message_template();
    reply->head->operation = msg->head->operation;
    HASH_FIND(hh, *table, msg->key, msg->head->key_length, h);

    // No entry found, message contains error
    if(h == NULL) {
        char * err = "";
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

// end functions for server

#endif //BLOCK_3_MSG_H
