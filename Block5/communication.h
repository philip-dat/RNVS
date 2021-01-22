#ifndef BLOCK5_COMMUNICATION_H
#define BLOCK5_COMMUNICATION_H

typedef struct timespec timespec;

typedef struct ntp_connection_t {
    char * host;
    uint32_t n;
    double root_disp;
    double disp;
    double delay;
    double offset;
}ntp_connection_t;

typedef struct timestmp{
    timespec T1;
    timespec T2;
    timespec T3;
    timespec T4;
}timestmp;

typedef struct ntp_protocol{
    uint8_t li_vn_mode;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precsion;
    ///~
    uint32_t root_delay;
    uint32_t root_disp;
    uint32_t reference_id;
    ///~
    uint32_t refTS_s;
    uint32_t refTS_f;
    uint32_t origTS_s;
    uint32_t origTS_f;
    uint32_t recvTS_s;
    uint32_t recvTS_f;
    uint32_t transmTS_s;
    uint32_t transmTS_f;
}ntp_protocol;

void send_info(timestmp *time,ntp_protocol *protocol, int sockfd, struct addrinfo *p);

void receive_info(timestmp *time, ntp_protocol *protocol, int sockfd, struct addrinfo *p, socklen_t addrlen);

void unmarshall_ntp_protocol(ntp_protocol *protocol);

void communication(timestmp *time, ntp_protocol *protocol, socketfd, p, addrlen);

#endif //BLOCK5_COMMUNICATION_H
