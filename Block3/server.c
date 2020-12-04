/*
** server.c -- a stream socket server demo
*/
#define _POSIX_C_SOURCE 200809L
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
#include <signal.h>
#include <stdbool.h>
#include <math.h>
#define BACKLOG 10
#include "uthash.h"



typedef struct my_struct {
    char *key;
    char *value;
    unsigned short valueLen;
    UT_hash_handle hh; /* makes this structure hashable */
}element;

typedef struct headerP
{
    char order[1];
    char transId[1];
    unsigned short keyLM;
    unsigned short valueLM;
}header;



element* add_elem (element* table, char* key, char* value, unsigned short valueLength) {
    element* elem;
    elem = calloc(1, sizeof *elem);
    elem->key = key;
    elem->value = value;
    memcpy(elem->value, value, valueLength);
    elem->valueLen = valueLength;
    return elem;
}


ssize_t rec(int sockfd, void *buf, size_t len, int flags)
{
    ssize_t leng;
    if ((leng = recv(sockfd, buf, len, flags)) == -1)
    {
        perror("recv");
        close(sockfd);
        exit(1);
    }
    return leng;
}

int toBit(char byte, int index)
{
    index = 7 - index;
    return (int)((byte >> index));
}

void convertNumberIntoArray(unsigned int number,char * arr) {
    int i = 0;
    do {
        arr[i] = number % 10;
        printf("arr: %d num : %d\n",arr[i],number %10);
        number /= 10;
        i++;
    } while (number != 0);
}

void sendprocess(){

}

void process(element * table, int new_fd){
    header* head = malloc(sizeof(header));
    char buf[2];
    while(rec(new_fd, head->order, sizeof(head->order), 0) != 0) {
        // receiving
        rec(new_fd, head->transId, sizeof(head->transId), 0);
        rec(new_fd, buf, sizeof(buf), 0);
        head->keyLM = buf[0] + buf[1];
        rec(new_fd, buf, sizeof(buf), 0);
        head->valueLM = buf[0] + buf[1];

        char *key = calloc(head->keyLM +1, sizeof(char));
        char *value = calloc(head->valueLM +1, sizeof(char));

        rec(new_fd, key, head->keyLM, 0);

        if((head->order[0] & 0b00001111) == 2) {
            rec(new_fd, value, head->valueLM, 0);
        }

        int get = toBit(head->order[0], 5);
        int set = toBit(head->order[0], 6);
        int del = toBit(head->order[0], 7);

        element *elem = NULL;
        HASH_FIND_STR(table, key, elem);

        bool get1 = false;
        bool success = true;

        // working
        if(get == 1) {
            get1 = true;
            if(elem != NULL) {
                head->keyLM = strlen(elem->key);
                head->valueLM = elem->valueLen;
                value = calloc(head->valueLM, sizeof(char));
                memcpy(value, elem->value, head->valueLM);
            }
            else {
                success = false;
            }
        }
        else if(set == 1) {
            if(elem != NULL) {
                HASH_DEL(table, elem);
            }
            elem = add_elem(table, key, value, head->valueLM);
            HASH_ADD_KEYPTR(hh, table, elem->key, strlen(elem->key), elem);
        }
        else if(del == 1) {
            if(elem != NULL) {
                HASH_DEL(table, elem);

                free(elem);

                head->valueLM = 0;
                head->keyLM = 0;
            }
        }

        // sending
        head->order[0] = head->order[0] + 00001000;
        send(new_fd, head->order, sizeof(head->order), 0);
        send(new_fd, head->transId, sizeof(head->transId), 0);

        buf[0] = head->keyLM >> 8;
        buf[1] = (head->keyLM << 8) >> 8;

        if (!success || get1)
            send(new_fd, buf, sizeof(buf), 0);
        else {
            buf[0] = 0;
            buf[1] = 0;
            send(new_fd, buf, sizeof(buf), 0);
        }
        if (success && get1) {
            buf[0] = head->valueLM >> 8;
            buf[1] = (head->valueLM << 8) >> 8;
            send(new_fd, buf, sizeof(buf), 0);
        }else{
            buf[0] = 0;
            buf[1] = 0;
            send(new_fd, buf, sizeof(buf), 0);
        }

        if (!success || get1)
            send(new_fd, key, head->keyLM, 0);
        if (success && get) {
            send(new_fd, value, head->valueLM, 0);
        }
    }
    close(new_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr,"Error you need a ort as parameter\n");
        exit(1);
    }

    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    
    int yes = 1;
    int rv;
    char *PORT;
    PORT = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            fprintf(stderr,"server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            fprintf(stderr,"setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            fprintf(stderr,"server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        fprintf(stderr,"listen");
        exit(1);
    }

    element * table = NULL;
    sin_size = sizeof their_addr;
   
    printf("server: waiting for connections...\n");

    while (1) { // main accept() loop
        
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            fprintf(stderr,"accept");
            continue;            
        }
        process(table, new_fd);
    }
        close(sockfd);  
        return 0;
}
