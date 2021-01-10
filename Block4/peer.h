

#ifndef BLOCK_3_PEER_H
#define BLOCK_3_PEER_H

#include <unistd.h>
typedef struct peer{
    uint16_t ID;
    char* IP;
    uint16_t port;
} peer;

void free_peer(peer *p){
    if(p != NULL){
        if(p->IP != NULL){
            free(p->IP);
        }
        free(p);
    }
}


void listen_connection(peer *self, int *socketfd, struct addrinfo *hints, struct addrinfo *servinfo, struct addrinfo *p ){
        char port[30];
        sprintf(port, "%d", self->port);

        memset(hints, 0, sizeof(*hints));
        hints->ai_family = AF_UNSPEC;
        hints->ai_socktype = SOCK_STREAM;
        hints->ai_flags = AI_PASSIVE; // use my IP

        if(getaddrinfo(NULL, port , hints, &servinfo) != 0) {
            fprintf(stderr, "Error on getaddrinfo");
            exit(1);
        }

        // loop through all the results and bind to the first we can
        for(p = servinfo; p != NULL; p = p->ai_next) {
            if ((*socketfd = socket(p->ai_family, p->ai_socktype,
                                   p->ai_protocol)) == -1) {
                perror("server: socket");
                continue;
            }

            // reuse address for debugging
            int yes = 1;
            setsockopt(*socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

            //bind to socket
            if (bind(*socketfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(*socketfd);
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
        if (listen(*socketfd, 2) == -1) {
            perror("listen");
            exit(1);
        }

};

void server_send_connection(peer *destination , int *socketfd){
    int status;
    struct addrinfo *servinfo, hints, *p;

    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
    hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

    char port[30];
    sprintf(port, "%d", destination->port);
    //get address info
    if ((status = getaddrinfo(destination->IP, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // loop through result and try to connect to the first possible
    for (p = servinfo; p!= NULL; p = p->ai_next) {
        if ((*socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("Could not establish client socket");
            continue;
        }

        if(connect(*socketfd, p->ai_addr, p->ai_addrlen) == -1 ) {
            close(*socketfd);
            perror("Client could not connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(1);
    }
}

int is_peer_request(uint8_t header){
    if(header >> 7 & 1 == 1) {
        return 1;        //control bit set
    }else{
        return 0;
    }
}


/*char *shorten_key(char *key) {
    unsigned char short_key[16];
    for (int i = 0; i < 16 ; ++i) {
        short_key[i] = '\0';
    }

    for(int j = 0; j< 16; j++) {
        short_key[j] = key[j];
    }
    memset(key, short_key, 16);
}*/


uint16_t hash_key(char* input)
{
    const size_t N = 16;
    char s[N];
    snprintf(s, sizeof(s), "%s", input);

    uint8_t new[2];
    for (int i = 0; i < 2 ; ++i) {
        new[i] = (uint8_t) s[i];
    }

    uint16_t hash = (new[0] & 0xff) | ((new[1] & 0xff) << 8);
    return hash;

}

int in_Range(int hash, int start, int end) {
    if(hash <= end && hash > start || (hash <= end && hash >= 0) || (hash > start && hash <= 65536) ) {
        return 1;
    }
    return 0;
}

int receive_header(int socketfd, uint8_t *header){
    int bytes;
    if((bytes = recv(socketfd, header, sizeof(uint8_t), 0)) < 0){
        perror("Error while receiving header");
    }
    return bytes;
}


#endif //BLOCK_3_PEER_H
