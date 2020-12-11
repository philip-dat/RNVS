#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


struct quote {
    char* content;
    size_t len;
    struct quote* next;
};

struct quote* get_quotes(char* path, int* num){
    FILE* f = fopen(path, "r");

    if (f == NULL) {
        fprintf(stderr, "Failed to open file %s\n!", path);
        return NULL;
    }

    struct quote* quotes = NULL;
    struct quote* cur = quotes;
    (*num) = 0;

    ssize_t n = 0;
    while(true){
        char* line = NULL;
        size_t  len = 0;
        n = getline(&line, &len, f);

        if (n <= 0){
            free(line);
            break;
        }

        if (line[n-1] != '\n'){
            free(line);
            break;
        }

        struct quote* new_quote = (struct quote*) malloc(sizeof(struct quote));
        new_quote->content = line;
        new_quote->len = n;
        new_quote->next = NULL;

        (*num)++;

        if (quotes == NULL){
            quotes = new_quote;
            cur = new_quote;
        } else {
            cur->next = new_quote;
            cur = new_quote;
        }

    }

    return quotes;
}

struct quote* choose_rnd_quote(struct quote* quotes, int len){
    long index = (random()) % len;
    struct quote* q;
    int i;
    for (q = quotes, i=0; q != NULL; q=q->next, i++){
       if (i == index){
            return q;
       }
    }

    return NULL;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("%s\n", "No enough args provided!");
        return 1;
    }

    char* port = argv[1];
    char* path = argv[2];

    int count = 0;
    struct quote* quotes = get_quotes(path, &count);
    if (quotes == NULL) {
        fprintf(stderr, "Failed to load quotes!\n");
        return 1;
    }

    struct addrinfo hints;
    struct addrinfo* res;
    struct addrinfo* p;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;


    int status = getaddrinfo(NULL, port, &hints, &res);
    if (status != 0){
        fprintf(stderr, "%s\n", "getaddrinfo() failed!");
        return 1;
    }

    int s = -1;
    for(p = res; p != NULL;p = p->ai_next){
        s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (s == -1){
            continue;
        }

        int optval = 1;
        setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));


        status = bind(s, res->ai_addr, res->ai_addrlen);
        if (status != 0){
           close(s);
           continue;
        }

        break;
    }

    if (s == -1){
        fprintf(stderr, "%s\n", "Unable to create socket!");
        return 1;
    }

    if (status != 0){
        fprintf(stderr, "%s\n", "Failed to bind socket!");
        return 1;
    }

    status = listen(s, 1);
    if (status != 0){
        fprintf(stderr, "Listen failed!\n");
        return 1;
    }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    while(true){
        struct sockaddr_storage client_addr;
        socklen_t c_addr_size = sizeof(client_addr);
        int client = accept(s, (struct sockaddr*) &client_addr, &c_addr_size);
        fprintf(stderr, "%s\n", "Client accepted!");

        struct quote* q = choose_rnd_quote(quotes, count);

        if (q == NULL){
            close(client);
            continue;
        }

        long nsent = 0;

        while(nsent != q->len - 1){
           int bytes = send(client, (q->content) + nsent, q->len - nsent - 1, 0);
           if (bytes < 0){
               fprintf(stderr, "Sending data failed!\n");
               break;
           }
           nsent += bytes;
        }
        close(client);

    }
#pragma clang diagnostic pop

    freeaddrinfo(res);
    return 0;
}
