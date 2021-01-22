#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <netdb.h>
#include <sys/socket.h>

#include "communication.h"


void send_info(timestmp *time,ntp_protocol *protocol, int sockfd, struct addrinfo *p){
    clock_gettime(CLOCK_REALTIME, &(time->T1));

    if (sendto(sockfd, protocol, sizeof(ntp_protocol), 0, p->ai_addr, p->ai_addrlen) == -1) {
        perror("talker: sendto");
        exit(1);
    }

}

void receive_info(timestmp *time, ntp_protocol *protocol, int sockfd, struct addrinfo *p, socklen_t addrlen){
    if (recvfrom(sockfd, protocol, sizeof(ntp_protocol), 0, p->ai_addr, &addrlen) == -1) {
        perror("recvfrom");
        exit(1);
    }
    clock_gettime(CLOCK_REALTIME, &(time->T4));
}

void unmarshall_ntp_protocol(ntp_protocol *protocol) {
    protocol->root_delay      = ntohl(protocol->root_delay);
    protocol->root_disp = ntohl(protocol->root_disp);
    protocol->recvTS_s        = ntohl(protocol->recvTS_s);
    protocol->recvTS_f        = ntohl(protocol->recvTS_f);
    protocol->transmTS_s      = ntohl(protocol->transmTS_s);
    protocol->transmTS_f      = ntohl(protocol->transmTS_f);
}

void communication(timestmp *time, ntp_protocol *protocol, socketfd, p, addrlen){
    //send
    //---> time send T1
    send_info(time, protocol, socketfd, p);

    //receive
    //---> time receive T4
    receive_info(time, protocol, socketfd, p, addrlen);
    unmarshall_ntp_protocol(protocol);
}
