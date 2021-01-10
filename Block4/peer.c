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
#include "uthash.h"
#include "lookup.h"
#include <signal.h>
#include "peer.h"
#include "client_request.h"
#include "communication.h"

typedef struct value {
    int incomingfd;
    peer *self;
    peer *next;
    peer *prev;
    fd_set master;
    client_request *client_requests;
    entry *hashtable;
}passingval;

void sigchld_handler(int s) {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void assign_peer_info(peer *self, char *ID, char *IP, char *port){
    self->ID = atoi(ID);
    self->IP = malloc(sizeof(IP));
    memcpy(self->IP, IP, sizeof(IP) + 1);
    self->port = atoi(port);
}

void handle_peer_reply(passingval *val, protocol_lookup *recv_p){
    // get request from internal hash table
    client_request *c = find_client_request(recv_p->hash_ID, &(val->client_requests));
    if (c != NULL) {
        //send the client's original message to the responsible peer
        int responsible_p_fd;
        peer *responsible_p = malloc(sizeof(peer));
        responsible_p->ID = recv_p->node_ID;
        responsible_p->port = recv_p->node_port;
        responsible_p->IP = recv_p->node_IP;
        server_send_connection(responsible_p, &responsible_p_fd);
        send_message(c->msg, responsible_p_fd);

        //blocking wait for reply
        message *reply_msg = receive_message(responsible_p_fd, 1);

        // send to client
        send_message(reply_msg, c->socketfd);

        //remove from internal table
        remove_client_request(recv_p->hash_ID, &(val->client_requests));
        free(responsible_p);
    }
    else{
        fprintf(stderr, "Oooooops, client request got lost");
        exit(1);
    }
}

void handle_lookup(uint8_t header, passingval *val){
    int incoming_fd = val->incomingfd;

    // is a lookup protocol
    protocol_lookup *recv_p = receive_protocol(incoming_fd);
    recv_p->header = header;

    if (is_peer_reply(header))
    {
        handle_peer_reply(val, recv_p);
    }
/*    {
        // get request from internal hash table
        client_request *c = find_client_request(recv_p->hash_ID, &(val->client_requests));
        if (c != NULL) {
            //send the client's original message to the responsible peer
            int responsible_p_fd;
            peer *responsible_p = malloc(sizeof(peer));
            responsible_p->ID = recv_p->node_ID;
            responsible_p->port = recv_p->node_port;
            responsible_p->IP = recv_p->node_IP;
            server_send_connection(responsible_p, &responsible_p_fd);
            send_message(c->msg, responsible_p_fd);

            //blocking wait for reply
            message *reply_msg = receive_message(responsible_p_fd, 1);

            // send to client
            send_message(reply_msg, c->socketfd);

            //remove from internal table
            remove_client_request(recv_p->hash_ID, &(val->client_requests));
            free(responsible_p);

        } else {
            fprintf(stderr, "Oooooops, client request got lost");
            exit(1);
        }
    }*/
    else {
        // is the next peer responsible?
        // check if in range from our to next peer
        int in_next = in_Range(recv_p->hash_ID, (val->self->ID) + 1, val->next->ID);
        int in_my_range = in_Range(recv_p->hash_ID, (val->prev->ID) + 1, val->self->ID);
        if (in_next || in_my_range) {
            //next peer responsible for hash, send this info to origin peer
            peer *origin = malloc(
                    sizeof(peer));                                      //address of origin sender
            origin->ID = recv_p->node_ID;
            origin->port = recv_p->node_port;
            origin->IP = malloc(sizeof(recv_p->node_IP));
            memcpy(origin->IP, recv_p->node_IP, sizeof(recv_p->node_IP));

            int originfd;
            peer *responsible = in_my_range ? val->self : val->next;
            protocol_lookup *reply_p = create_reply_protocol(recv_p->hash_ID, responsible);
            server_send_connection(origin, &originfd);
            send_lookup(reply_p, originfd);                     //send to origin peer
            close(originfd);
            free_peer(origin);
            free(reply_p);

        }
        else {
            //send lookup request to the next peer
            int nextfd;
            server_send_connection(val->next, &nextfd);
            send_lookup(recv_p, nextfd);
            close(nextfd);
        }
    }
    free(recv_p);
}

void handle_operation(passingval *val, message *recv_msg){
    int incoming_fd = val->incomingfd;

    // handle operation and send reply message
    message *reply;
    char *op = parse_operation(recv_msg);

    if (strncmp(op, "GET", 3) == 0) {
        reply = get(&(val->hashtable), recv_msg);
    }
    else if (strncmp(op, "DELETE", 6) == 0) {
        reply = delete(&(val->hashtable), recv_msg);
    }
    else if (strncmp(op, "SET", 3) == 0) {
        reply = set(&(val->hashtable), recv_msg);
    }
    else {
        fprintf(stderr, "Unknown operation");
    }
    int sent = send_message(reply, incoming_fd);
    free_message(reply);
}

void handle_message(uint8_t header, passingval *val){
    int incoming_fd = val->incomingfd;
    // is a message
    message *recv_msg = receive_message(incoming_fd, 0);
    recv_msg->head->operation = header;
    uint16_t hash = hash_key(recv_msg->key);

    if (in_Range(hash, val->prev->ID + 1, val->self->ID)) {     // I am responsible
        handle_operation(val, recv_msg);
        }
    else {
            // I am not resposnsible but look for the one who is
            //Store received message in local hash table
            add_client_request(hash, incoming_fd, recv_msg, &(val->client_requests));

            // Create a lookup request
            protocol_lookup *pl = malloc(sizeof(protocol_lookup));
            set_lookup(pl, val->self, hash);

            //Send lookup request to next peer
            int nextfd;
            server_send_connection(val->next, &nextfd);
            send_lookup(pl, nextfd);
            free(pl);
            close(nextfd);

            //should not close up connection to the client
        }
    free_message(recv_msg);
}

void handle_received_data(passingval *val) {
    int incoming_fd = val->incomingfd;
    fd_set master = val->master;

    uint8_t header;
    int recv_bytes;
    // If error or
    if ((recv_bytes = receive_header(incoming_fd, &header)) <= 0) {
        // got error or connection closed by client
        if (recv_bytes == 0) {
            // connection closed
            printf("select server: socket %d hung up\n", incoming_fd);
        }
        else {
            perror("recv");
        }
        close(incoming_fd); // bye!
        FD_CLR(incoming_fd, &master); // remove from master set

    }
    else {
        if (is_lookup(header)) {
            handle_lookup(header, val);
        }
        else {
            handle_message(header, val);
        }
    }
}

void worker (int *fdmax, int listener, int *newfd, struct sockaddr_storage remoteaddr, passingval *val) {
    int incoming_fd = val->incomingfd;
    fd_set master = val->master;

    if (incoming_fd == listener) {
        // handle new connections
        int addrlen = sizeof(remoteaddr);
        *newfd = accept(listener,
                          (struct sockaddr *) &remoteaddr,
                          &addrlen);

        if (*newfd == -1) {
            perror("accept");
        }
        else {
            FD_SET(*newfd, &master); // add to master set
            if (*newfd > *fdmax) {    // keep track of the max
                *fdmax = *newfd;
            }
        }
    }
    else {
        handle_received_data(val);
    }
}

void set_val(passingval *val, int incoming_fd, peer *self, peer *next,peer *prev, fd_set master, client_request *client_requests, entry *hash_table){
    val->incomingfd = incoming_fd;
    val->self = self;
    val->next = next;
    val->prev = prev;
    val->master = master;
    val->client_requests = client_requests;
    val->hashtable = hash_table;
}

int main(int argc, char *argv[]) {

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;
    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

    // init empty hashtables
    client_request *client_requests = NULL;
    entry *hash_table = NULL;

    // Get command line input
    if (argc % 3 != 0 && argc != 10) {
        fprintf(stderr, "Please provide a suitable id, port and host");
        exit(1);
    }

    peer *self = malloc(sizeof(peer));
    peer *prev = malloc(sizeof(peer));
    peer *next = malloc(sizeof(peer));

    // assign peer info
    assign_peer_info(self, argv[1], argv[2], argv[3]);
    assign_peer_info(prev, argv[4], argv[5], argv[6]);
    assign_peer_info(next, argv[7], argv[8], argv[9]);

/*    self->ID = atoi(argv[1]);
    self->IP = malloc(sizeof(argv[2]));
    memcpy(self->IP, argv[2], sizeof(argv[2]) + 1);
    self->port = atoi(argv[3]);

    prev->ID = atoi(argv[4]);
    prev->IP = malloc(sizeof(argv[5]));
    memcpy(prev->IP, argv[5], sizeof(argv[5]) + 1);
    prev->port = atoi(argv[6]);

    next->ID = atoi(argv[7]);
    next->IP = malloc(sizeof(argv[8]));
    memcpy(next->IP, argv[8], sizeof(argv[8]) + 1);
    next->port = atoi(argv[9]);*/

    struct addrinfo hints, *servinfo, *p;

    listen_connection(self, &listener, &hints, servinfo, p);

    // add the listener to the master set
    FD_SET(listener, &master);
    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one


    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // infinite loop
    while (1) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        for (int incoming_fd = 0; incoming_fd <= fdmax; incoming_fd++) {
            if (FD_ISSET(incoming_fd, &read_fds))
            /*{
                passingval *val = malloc(sizeof(passingval));
                set_val(val, incoming_fd, self, next, prev, master, client_requests, hash_table);
                worker((&fdmax), listener, (&newfd), remoteaddr, val);
                free(val);
            }*/
           { // we got one!!
                if (incoming_fd == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                                   (struct sockaddr *) &remoteaddr,
                                   &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    }
                    else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                    }
                }
                else {

                    // Handle received data

                    uint8_t header;
                    int recv_bytes;
                    // If error or
                    if ((recv_bytes = receive_header(incoming_fd, &header)) <= 0) {
                        // got error or connection closed by client
                        if (recv_bytes == 0) {
                            // connection closed
                            printf("select server: socket %d hung up\n", incoming_fd);
                        }
                        else {
                            perror("recv");
                        }
                        close(incoming_fd); // bye!
                        FD_CLR(incoming_fd, &master); // remove from master set

                    } else {

                        if (is_lookup(header)) {
                            // is a lookup protocol
                            protocol_lookup *recv_p = receive_protocol(incoming_fd);
                            recv_p->header = header;

                            if (is_peer_reply(header)) {
                                // get request from internal hash table
                                client_request *c = find_client_request(recv_p->hash_ID, &client_requests);
                                if (c != NULL) {
                                    //send the client's original message to the responsible peer
                                    int responsible_p_fd;
                                    peer *responsible_p = malloc(sizeof(peer));
                                    responsible_p->ID = recv_p->node_ID;
                                    responsible_p->port = recv_p->node_port;
                                    responsible_p->IP = recv_p->node_IP;
                                    server_send_connection(responsible_p, &responsible_p_fd);
                                    send_message(c->msg, responsible_p_fd);

                                    //blocking wait for reply
                                    message *reply_msg = receive_message(responsible_p_fd, 1);

                                    // send to client
                                    send_message(reply_msg, c->socketfd);

                                    //remove from internal table
                                    remove_client_request(recv_p->hash_ID, &client_requests);
                                    free(responsible_p);

                                } else {
                                    fprintf(stderr, "Oooooops, client request got lost");
                                    exit(1);
                                }
                            } else {
                                // is the next peer responsible?
                                // check if in range from our to next peer
                                int in_next = in_Range(recv_p->hash_ID, (self->ID) + 1, next->ID);
                                int in_my_range = in_Range(recv_p->hash_ID, (prev->ID) + 1, self->ID);
                                if (in_next || in_my_range) {
                                    //next peer responsible for hash, send this info to origin peer
                                    peer *origin = malloc(
                                            sizeof(peer));                                      //address of origin sender
                                    origin->ID = recv_p->node_ID;
                                    origin->port = recv_p->node_port;
                                    origin->IP = malloc(sizeof(recv_p->node_IP));
                                    memcpy(origin->IP, recv_p->node_IP, sizeof(recv_p->node_IP));

                                    int originfd;
                                    peer *responsible = in_my_range ? self : next;
                                    protocol_lookup *reply_p = create_reply_protocol(recv_p->hash_ID, responsible);
                                    server_send_connection(origin, &originfd);
                                    send_lookup(reply_p, originfd);                     //send to origin peer
                                    close(originfd);
                                    free_peer(origin);
                                    free(reply_p);

                                } else {
                                    //send lookup request to the next peer
                                    int nextfd;
                                    server_send_connection(next, &nextfd);
                                    send_lookup(recv_p, nextfd);
                                    close(nextfd);
                                }
                            }
                            free(recv_p);

                        } else {
                            // is a message
                            message *recv_msg = receive_message(incoming_fd, 0);
                            recv_msg->head->operation = header;
                            uint16_t hash = hash_key(recv_msg->key);

                            if (in_Range(hash, prev->ID + 1, self->ID)) {     // I am responsible

                                // handle operation and send reply message
                                message *reply;
                                char *op = parse_operation(recv_msg);
                                if (strncmp(op, "GET", 3) == 0) {
                                    reply = get(&hash_table, recv_msg);
                                } else if (strncmp(op, "DELETE", 6) == 0) {
                                    reply = delete(&hash_table, recv_msg);
                                } else if (strncmp(op, "SET", 3) == 0) {
                                    reply = set(&hash_table, recv_msg);
                                } else {
                                    fprintf(stderr, "Unknown operation");
                                }
                                send_message(reply, incoming_fd);
                                free_message(reply);

                            } else if(in_Range(hash, self->ID + 1, next->ID)) {
                                int nextfd;
                                server_send_connection(next, &nextfd);
                                send_message(recv_msg, nextfd);
                                //blocking wait for reply
                                message *reply_msg = receive_message(nextfd, 1);
                                close(nextfd);
                                send_message(reply_msg, incoming_fd);

                            }


                                else {
                                // I am not resposnsible but look for the one who is
                                //Store received message in local hash table
                                add_client_request(hash, incoming_fd, recv_msg, &client_requests);

                                // Create a lookup request
                                protocol_lookup *pl = malloc(sizeof(protocol_lookup));
                                set_lookup(pl, self, hash);

                                //Send lookup request to next peer
                                int nextfd;
                                server_send_connection(next, &nextfd);
                                send_lookup(pl, nextfd);
                                free(pl);
                                close(nextfd);

                                //should not close up connection to the client
                            }
                            free_message(recv_msg);
                        }
                    }
                }
            }
        }
    }
    free_peer(self);
    free_peer(next);
    free_peer(prev);
    close(listener);
}