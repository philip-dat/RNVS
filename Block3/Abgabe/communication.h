#ifndef BLOCK_3_MSG_H
#define BLOCK_3_MSG_H

#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include "uthash.h"

#define BUF_SIZE 5

// begin structs

typedef struct header {
    uint8_t command_bits;
    uint32_t len_of_value;
    uint16_t len_of_key;
} header;

typedef struct message {
    header *meta_data;
    char *value;
    char *key;

} message;

typedef struct entry {
    char *key;
    char *value;
    uint32_t len_of_value;
    uint16_t len_of_key;
    UT_hash_handle hh;
} entry;

// end structs

// begin functions for communication

char* get_command(message *msg){
    if((msg->meta_data->command_bits >> 0 & 1) == 1) {
        return "DELETE";
    } else if ((msg->meta_data->command_bits >> 1 & 1) == 1) {
        return "SET";
    } else if ((msg->meta_data->command_bits >> 2 & 1) == 1) {
        return "GET";
    }
    return "ERROR";
}

int read_message (int sock, char *buff, size_t buff_len, size_t *buff_used ) {
    int result = 0;
    *buff_used = 0;

    // Loop until the buffer is full, or a recv result
    // indicates there is a problem of one sort or another
    while (*buff_used < buff_len ) {
        int n = recv(sock, &buff[*buff_used], 1, 0);
        if ( n > 0 ) {
            // success
            ++(*buff_used);
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

int write_message(char *buff, size_t buff_len, size_t *buff_used){
    int result = 0;

    *buff_used = 0;

    while(*buff_used < buff_len) {
        int n = fwrite(&buff[*buff_used], 1, 1, stdout);
        if (n > 0){
            ++(*buff_used);
            result = 1;
        }else {
            result = 0;
            break;
        }
    }
    return result;
}

int send_message(message *msg, int socketfd) {
    int sent_bytes = 0;
    if(msg) {
        sent_bytes = send(socketfd, &(msg->meta_data->command_bits), sizeof(uint8_t), 0 );

        // change byte order
        uint16_t conv_key_length = htons(msg->meta_data->len_of_key);
        uint32_t conv_value_length = htonl(msg->meta_data->len_of_value);

        sent_bytes += send(socketfd, &(conv_key_length), sizeof(uint16_t), 0);
        sent_bytes += send(socketfd, &(conv_value_length), sizeof(uint32_t), 0);
        sent_bytes += send(socketfd, msg->key, sizeof(char) * msg->meta_data->len_of_key, 0);
        sent_bytes += send(socketfd, msg->value, sizeof(char) * msg->meta_data->len_of_value, 0);
    }
    return sent_bytes;
}

message *create_new_message() {
    //allocate storage for message
    message *new_message= malloc(sizeof(message));
    new_message->meta_data = malloc(sizeof(header));

    new_message->meta_data->len_of_value = 0;
    new_message->meta_data->len_of_key = 0;
    new_message->value = NULL;
    new_message->key = NULL;

    return new_message;
}

message *receive_message(int socketfd) {
    // receive header first
    message *msg = create_new_message();

    recv(socketfd, &(msg->meta_data->command_bits), sizeof(uint8_t), 0);
    recv(socketfd, &(msg->meta_data->len_of_key), sizeof(uint16_t), 0);
    recv(socketfd, &(msg->meta_data->len_of_value), sizeof(uint32_t), 0);

    // convert to host byte order
    msg->meta_data->len_of_key = ntohs(msg->meta_data->len_of_key);
    msg->meta_data->len_of_value = ntohl(msg->meta_data->len_of_value);

    // receive the rest
    msg->key = malloc(sizeof(char) * msg->meta_data->len_of_value);
    msg->value = malloc(sizeof(char) * msg->meta_data->len_of_value);
    recv(socketfd, msg->key, sizeof(char) * msg->meta_data->len_of_key, 0);

    // if command_bits is 'GET'
    if (msg->meta_data->len_of_value > 0) {
        size_t bufferusage = 0;
        read_message(socketfd, msg->value,sizeof(char) * msg->meta_data->len_of_value, &bufferusage );
        if(bufferusage > 0){
            msg->value[bufferusage] = '\0';
        }else {
            fprintf(stderr,"Something went wrong receiving the message\n");
            exit(1);
        }
    } return msg;
}

void *free_message(message *message) {
    if (message){
        if(message->value) {
            free(message->value);
        } if(message->key) {
            free(message->key);
        } free(message->meta_data);
        free(message);
    }
}

// end functions for communication

// begin functions for client

uint8_t set_bits(char *command) {
    uint8_t command_as_bits = 0b00000000;
    //for get, third bit is set
    if(strcmp(command, "GET") == 0){
        command_as_bits |= 1 << 2;
    }//for set, second bit is set
    else if(strcmp(command, "SET") == 0){
        command_as_bits |= 1 << 1;
    }//for delete, first bit is set
    else if(strcmp(command, "DELETE") == 0){
        command_as_bits |= 1 << 0;
    }else {
        fprintf(stderr,"value for command_bits isn't valid\n");
        exit(1);
    }
    return command_as_bits;
}

void get_value_from_stdin(message *msg){
    char buffer[BUF_SIZE];
    size_t chunk = 0;
    while( (chunk = fread(buffer, sizeof(char), BUF_SIZE, stdin)) > 0) {
        int end_of_file = msg->meta_data->len_of_value;
        msg->meta_data->len_of_value += chunk;
        msg->value = realloc(msg->value, msg->meta_data->len_of_value * sizeof(char));
        memcpy(&(msg->value[end_of_file]), buffer, chunk);
    }
}

void create_message_content(char *key, char *operation, message *msg){
    msg->meta_data->len_of_key = strlen(key);
    msg->key = malloc(msg->meta_data->len_of_key);
    memcpy(msg->key, key, strlen(key));
    msg->meta_data->command_bits = set_bits(operation);
    msg->value = malloc(sizeof(char) * BUF_SIZE);

    if(strncmp(get_command(msg),"SET", 3) == 0) {
        get_value_from_stdin(msg);
    }
}

void handle_reply_from_server(message *reply){
    uint32_t value_len = reply->meta_data->len_of_value;
    bool ack_bit_is_set = (((reply->meta_data->command_bits >> 3) & 1) == 1);



    if(!ack_bit_is_set) {
        fwrite(reply->value, 1, value_len, stderr);
    } else if(value_len != 0) {
            size_t buffer_spaceleft = 0;
            write_message(reply->value, value_len, &buffer_spaceleft);
            //exit if writing message fails
            if(buffer_spaceleft <= 0){
                fprintf(stderr, "WriteMessage failed");
                exit(1);
            }
    }//if client says set
    else {
        fprintf(stderr, "Ok");
    }
}

// end functions for client

// begin functions for server

entry *create_add_entry(entry **table, message *msg) {
    entry *new_entry = malloc(sizeof(entry));
    new_entry->len_of_key = msg->meta_data->len_of_key;
    new_entry->len_of_value = msg->meta_data->len_of_value;
    new_entry->key = malloc(sizeof(char) * new_entry->len_of_key + 1);
    new_entry->value = malloc(sizeof(char) * new_entry->len_of_value);
    memcpy(new_entry->key, msg->key, new_entry->len_of_key);
    memcpy(new_entry->value, msg->value, new_entry->len_of_value);
    new_entry->key[new_entry->len_of_key] = '\0';
    HASH_ADD_KEYPTR(hh, *table, new_entry->key, new_entry->len_of_key, new_entry);
    return new_entry;
}

message *set_entry(entry **table, message *msg){
    entry *h = NULL;
    HASH_FIND(hh, *table, msg->key, msg->meta_data->len_of_key, h);
    if(h == NULL) {
        h = create_add_entry(table, msg);
    }
    // else update the value
    if(msg->meta_data->len_of_value != h->len_of_value) {
        h->len_of_value = msg->meta_data->len_of_value;
        h-> value = realloc(h->value, h->len_of_value);
        memcpy(h->value, msg->value, h->len_of_value);
    }

    // create and return a answer
    message *reply = create_new_message();
    reply->meta_data->command_bits = msg->meta_data->command_bits;
    reply->meta_data->command_bits |= 1 << 3;
    return reply;
}

message *delete_entry(entry **table, message *msg){
    entry *h;
    HASH_FIND(hh, *table, msg->key, msg->meta_data->len_of_key, h);
    message *answer_for_client = create_new_message();
    answer_for_client->meta_data->command_bits =  msg->meta_data->command_bits;

    //if entry doesnt exist
    if (h == NULL) {
        char *empty_msg = "";
        answer_for_client->meta_data->len_of_value = strlen(empty_msg);

        //reallocate
        answer_for_client->value = realloc(answer_for_client->value, answer_for_client->meta_data->len_of_value);

        memcpy(answer_for_client->value, empty_msg, answer_for_client->meta_data->len_of_value);
        return answer_for_client;
    }

    HASH_DEL(*table, h);

    //free this entry
    free(h->key);
    free(h->value);
    free(h);


    answer_for_client->meta_data->command_bits |= 1 << 3;
    return answer_for_client;
}

message *get_entry(entry **table, message *msg) {

    entry *h;
    message *answer_for_client = create_new_message();
    answer_for_client->meta_data->command_bits = msg->meta_data->command_bits;
    HASH_FIND(hh, *table, msg->key, msg->meta_data->len_of_key, h);

    //nothing with this key is in hashtable
    if(h == NULL) {
        char * empty_msg = "";
        answer_for_client->meta_data->len_of_value = strlen(empty_msg);

        //reallocate
        answer_for_client->value = realloc(answer_for_client->value, answer_for_client->meta_data->len_of_value);
        memcpy(answer_for_client->value, empty_msg, answer_for_client->meta_data->len_of_value);
        return answer_for_client;
    }

    //save everything static in answer
    answer_for_client->meta_data->command_bits |= 1 << 3;
    answer_for_client->meta_data->len_of_value = h->len_of_value;
    answer_for_client->meta_data->len_of_key = h->len_of_key;

    //save storage for answer
    answer_for_client->value = malloc(sizeof(char) * answer_for_client->meta_data->len_of_value);
    answer_for_client->key = malloc(sizeof(char) * answer_for_client->meta_data->len_of_key);

    //save per memcpy, because not only real strings
    memcpy(answer_for_client->value, h->value, h->len_of_value);
    memcpy(answer_for_client->key, h->key, h->len_of_key);
    return answer_for_client;
}

// end functions for server

#endif //BLOCK_3_MSG_H
