#ifndef BLOCK_4_LOOKUP_H
#define BLOCK_4_LOOKUP_H

#include "peer.h"

typedef struct protocol_lookup {
    uint8_t header;
    uint16_t hash_ID;
    uint16_t node_ID;
    char node_IP[32];
    uint16_t node_port;
} protocol_lookup;



void set_lookup(protocol_lookup *l, peer *p, u_int16_t hash) {
    l->header = 0b00000000;
    l->header |= 1 << 7; //set control bit
    l->header |= 1 << 0; //set lookup bit
    l->hash_ID = hash;
    l->node_ID = p->ID;
    memcpy(l->node_IP, p->IP, sizeof(l->node_IP));
    l->node_port = p->port;

}

protocol_lookup *receive_protocol(int socketfd) {
    protocol_lookup *p = malloc(sizeof(protocol_lookup));
    recv(socketfd, &(p->hash_ID), sizeof(uint16_t), 0);
    recv(socketfd, &(p->node_ID), sizeof(uint16_t), 0);
    recv(socketfd, &(p->node_port), sizeof(uint16_t), 0);
    recv(socketfd, &(p->node_IP), sizeof(char)*32, 0);


    // convert to host byte order
    p->hash_ID = ntohs(p->hash_ID);
    p->node_ID = ntohs(p->node_ID);
    p->node_port = ntohs(p->node_port);

    return p;

}

protocol_lookup *create_reply_protocol(uint16_t hash, peer *next){

    protocol_lookup *reply = malloc(sizeof(protocol_lookup));
    reply->header = 0b00000000;
    reply->header |= 1 << 7;        // set control bit
    reply->header |= 1 << 1;        //set reply bit
    reply->header |= 1 << 0;
    reply->hash_ID = hash;
    reply->node_ID = next->ID;
    reply->node_port = next->port;
    memcpy(reply->node_IP, next->IP, 32);

    return reply;
}

int is_lookup_request(uint8_t header){
    if(header >> 0 & 1 == 1) {
        return 1;        //lookup bit set
    }else{
        return 0;
    }
}

int is_peer_reply(uint8_t header){
    if(header >> 1 & 1 == 1) {
        return 1;        //reply bit set
    }else{
        return 0;
    }
}

int is_lookup(uint8_t header) {
    if(header >> 7 & 1 == 1) {
        return 1;        //reply control set
    }else{
        return 0;
    }
}

int get_lookup_status(protocol_lookup *l, uint8_t header, peer *self, peer *prev){
    /*
     1 : peer ID received
     2 : peer found
     3 : searching for peer
     */
    if (is_peer_reply(header)) {
        return 1;
    } else {
        if (in_Range(l->hash_ID, self->ID+1, prev->ID)){
            return 2;
        } else {
            return 3;
        }
    }
}

void send_lookup(protocol_lookup *l, int socket){
    send(socket, &(l->header), sizeof(uint8_t), 0);

    //marshalling
    uint16_t conv_hash_ID = htons(l->hash_ID);
    uint16_t conv_ID = htons(l->node_ID);
    uint16_t conv_port = htons(l->node_port);
    send(socket, &conv_hash_ID, sizeof(uint16_t), 0);
    send(socket, &conv_ID, sizeof(uint16_t), 0);
    send(socket, &conv_port, sizeof(uint16_t), 0);
    send(socket, &(l->node_IP), sizeof(l->node_IP), 0);

}

#endif //BLOCK_4_LOOKUP_H
