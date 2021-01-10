#ifndef BLOCK_4_CLIENT_REQUEST_H
#define BLOCK_4_CLIENT_REQUEST_H

#include "communication.h"

typedef struct client_request {
    int hash_key;
    int socketfd;
    message *msg;
    UT_hash_handle hh;
} client_request;

void *add_client_request(uint16_t hash, int socket, message *msg, client_request **table){
    client_request *cr;
    HASH_FIND_INT(*table, &hash, cr);
    if( cr == NULL) {
        cr = malloc(sizeof(client_request));
        cr->msg = copy_msg(msg);
        cr->hash_key = hash;
        cr->socketfd = socket;
        HASH_ADD_INT(*table, hash_key, cr);
    } else {
        fprintf(stderr, "Client request already exists");
        exit(1);
    }
}

client_request *find_client_request(uint16_t hash, client_request **table){
    client_request *c;
    int h = (int) hash;          // cast to int to look up in hash tbl
    HASH_FIND_INT(*table, &h, c);
    return c;
}

void remove_client_request(uint16_t hash, client_request **table){
    client_request *cr = find_client_request(hash, table);

    if (cr == NULL) {
        fprintf(stderr,"Record does not exist");
        exit(1);
    }
    HASH_DEL(*table, cr);
    free_message(cr->msg);
    free(cr);
}

#endif //BLOCK_4_CLIENT_REQUEST_H
