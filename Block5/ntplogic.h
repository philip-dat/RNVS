#ifndef BLOCK5_NTPLOGIC_H
#define BLOCK5_NTPLOGIC_H

void print_ntp_connection_t(ntp_connection_t *ntp);

void ntp_logic(int n, char *host, int socketfd, struct addrinfo *p ,socklen_t addrlen);

#endif //BLOCK5_NTPLOGIC_H
