#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <bits/time.h>
#include <time.h>
#include <unistd.h>

const char * port = "123";
#define UNIX_OFFSET 2208988800.0
#define pow32 4294967296.0
#define pow16 65536.0

typedef struct {
    uint8_t flags;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;

    uint32_t root_delay;

    uint32_t root_dispersion;

    uint32_t referenceID;

    uint32_t reference_timestamp_seconds;
    uint32_t reference_timestamp_fractions;

    uint32_t origin_timestamp_seconds;
    uint32_t origin_timestamp_fractions;

    uint32_t receive_timestamp_seconds;
    uint32_t receive_timestamp_fractions;

    uint32_t transmit_timestamp_seconds;
    uint32_t transmit_timestamp_fractions;
}NTP_packet_format;

void int_to_bit_array(unsigned int a, int* array){
    for(int i = 15; i >= 0; i--){
        array[i] = a % 2;
        a = a / 2;
    }
}

int power(int a){
    int res = 1;
    for(int i = 0; i < a; i++){
        res *= 2;
    }
    return res;
}

char* marshalling(NTP_packet_format *packet){
    char *buffer = malloc(sizeof(NTP_packet_format));
    if(buffer== NULL){
        fprintf(stderr, "Error: Malloc in marshalling!\n");
        exit(0);
    }
    memset(buffer, 0, sizeof(NTP_packet_format));
    buffer[0] = packet->flags;
    buffer[1] = packet->stratum;
    buffer[2] = packet->poll;
    buffer[3] = packet->precision;
    return buffer;
}

NTP_packet_format* demarschalling(const char *buffer){
    NTP_packet_format *ntp = (NTP_packet_format *)malloc(sizeof(NTP_packet_format));
    if(ntp == NULL){
        perror("Error: malloc demarshalling\n");
        return NULL;
    }

    ntp->flags = buffer[0];
    ntp->stratum = buffer[1];
    ntp->poll = buffer[2];
    ntp->precision = buffer[3];

    uint32_t temp;
    memcpy(&temp, buffer+4, 4);
    ntp->root_delay = ntohl(temp);

    memcpy(&temp, buffer+8, 4);
    ntp->root_dispersion = ntohl(temp);

    memcpy(&temp, buffer+12, 4);
    ntp->referenceID = ntohl(temp);

    memcpy(&temp, buffer+16, 4);
    ntp->reference_timestamp_seconds = ntohl(temp);

    memcpy(&temp, buffer+20, 4);
    ntp->reference_timestamp_fractions = ntohl(temp);

    memcpy(&temp, buffer+24, 4);
    ntp->origin_timestamp_seconds = ntohl(temp);

    memcpy(&temp, buffer+28, 4);
    ntp->origin_timestamp_fractions = ntohl(temp);

    memcpy(&temp, buffer+32, 4);
    ntp->receive_timestamp_seconds = ntohl(temp);

    memcpy(&temp, buffer+36, 4);
    ntp->receive_timestamp_fractions = ntohl(temp);

    memcpy(&temp, buffer+40, 4);
    ntp->transmit_timestamp_seconds = ntohl(temp);

    memcpy(&temp, buffer+44, 4);
    ntp->transmit_timestamp_fractions = ntohl(temp);
    return ntp;
}

NTP_packet_format* new_packet(){
    NTP_packet_format* packet = malloc(sizeof(NTP_packet_format));
    if(packet == NULL){
        perror("Error: malloc new packet\n");
        return NULL;
    }
    memset(packet, 0, sizeof(NTP_packet_format));
    packet->flags = (1 << 5) | (1 << 1) | (1 << 0);
    return packet;
}

void print_packet(NTP_packet_format* p){
    fprintf(stderr,"flags: %u\n", p->flags);
    fprintf(stderr,"stratum: %u\n", p->stratum);
    fprintf(stderr,"poll: %u\n", p->poll);
    fprintf(stderr,"precision: %u\n", p->precision);
    fprintf(stderr,"root_delay: %u\n", p->root_delay);
    fprintf(stderr,"root_dispersion: %u\n", p->root_dispersion);
    fprintf(stderr,"referenceID: %u\n", p->referenceID);
    fprintf(stderr,"reference_timestamp_seconds: %u\n", p->reference_timestamp_seconds);
    fprintf(stderr,"reference_timestamp_fractions: %u\n", p->reference_timestamp_fractions);
    fprintf(stderr,"origin_timestamp_seconds: %u\n", p->origin_timestamp_seconds);
    fprintf(stderr,"origin_timestamp_fractions: %u\n", p->origin_timestamp_fractions);
    fprintf(stderr,"receive_timestamp_seconds: %u\n", p->receive_timestamp_seconds);
    fprintf(stderr,"receive_timestamp_fractions: %u\n", p->receive_timestamp_fractions);
    fprintf(stderr,"transmit_timestamp_seconds: %u\n", p->transmit_timestamp_seconds);
    fprintf(stderr,"transmit_timestamp_fractions: %u\n", p->transmit_timestamp_fractions);
}

int main(int argc, char** argv) {
    if(argc < 3){
        fprintf(stderr, "invlaid input\n");
        exit(0);
    }

    const int n = (int)strtol(argv[1], NULL, 10);
    struct addrinfo hints;
    struct addrinfo *serverinfo;
    int broadcast = 1, receive, send, start;
    struct timespec t1, t4;
    double offset, delay[n], dispersion_average, min_delay, max_delay, root_disp;
    double T1, T2, T3, T4;

    char** DNS;
    DNS = (char **)malloc(sizeof(char *) * (argc - 2));
    for(int i = 2; i < argc; i++){
        DNS[i-2] = (char *)malloc(sizeof(char) * 20);
        strcpy(DNS[i-2], argv[i] );
    }
    NTP_packet_format *NTP_Paket;
    FILE *f;
    f = fopen("data.csv", "w");
    for(int dns = 0; dns < argc-2; dns++){

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;

        int status;
        if((status = getaddrinfo(DNS[dns], port, &hints, &serverinfo)) != 0){
            fprintf(stderr,"getaddrinfo error: %s\n", gai_strerror(status));
            exit(0);
        }

        int socke = 0;
        struct addrinfo *a;
        for(a = serverinfo; a != NULL; a->ai_next){
            if((socke = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol)) < 0){
                continue;
            }
            break;
        }
        if(a == NULL){
            fprintf(stderr, "socket error\n");
            exit(0);
        }

        if(setsockopt(socke, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0){
            fprintf(stderr, "set socket error\n");
            exit(0);
        }

        for(int i = 0; i < n; i++){
            NTP_Paket = new_packet();
            char* buffer = marshalling(NTP_Paket);
            free(NTP_Paket);

            clock_gettime(CLOCK_REALTIME, &t1);
            if((send = sendto(socke, buffer, sizeof(NTP_packet_format), 0, a->ai_addr, a->ai_addrlen)) < 0){
                fprintf(stderr, "send error: %s\n", gai_strerror(send));
                exit(0);
            }

            while((receive = recvfrom(socke, buffer, sizeof(NTP_packet_format), 0, a->ai_addr, &a->ai_addrlen)) <= 0);
            if(receive < 0){
                fprintf(stderr, "receive error: %s\n", gai_strerror(receive));
                exit(0);
            }
            clock_gettime(CLOCK_REALTIME, &t4);

            NTP_Paket = demarschalling(buffer);

            T1 = (double)(t1.tv_sec + (double)(t1.tv_nsec) / 1000000000);
            T4 = (double)(t4.tv_sec + (double)(t4.tv_nsec) / 1000000000);
            T2 = (double)(NTP_Paket->receive_timestamp_seconds - UNIX_OFFSET + NTP_Paket->receive_timestamp_fractions / pow32);
            T3 = (double)(NTP_Paket->transmit_timestamp_seconds - UNIX_OFFSET + NTP_Paket->transmit_timestamp_fractions / pow32);
            delay[i] = ((T4 - T1) - (T3 - T2)) / 2;
            offset = ((T2 - T1) + (T3 - T4)) / 2;

            start = i < 7 ? 0 : i - 7;
            max_delay = delay[i];
            min_delay = delay[i];
            for(int q = start; q <= i; q++){
                max_delay = max_delay < delay[q] ? delay[q] : max_delay;
                min_delay = min_delay > delay[q] ? delay[q] : min_delay;
            }
            dispersion_average = (max_delay - min_delay) * 2;
            root_disp = (double)(NTP_Paket->root_dispersion >> 16) + ((double)(NTP_Paket->root_dispersion & 0xFFFF) / pow16);
            printf("%s;%d;%.6lf;%.6lf;%.6lf;%.6lf\n", DNS[dns], i, root_disp , dispersion_average, delay[i], offset);
            fprintf(f,"%s;%d;%.6lf;%.6lf;%.6lf;%.6lf\n", DNS[dns], i, root_disp, dispersion_average, delay[i], offset);
            free(NTP_Paket);
            free(buffer);
            sleep(8);
        }
        freeaddrinfo(serverinfo);
        shutdown(socke, 2);
    }
    for(int i = 2; i < argc; i++){
        free(DNS[i-2]);
    }
    fclose(f);
    free(DNS);
    return 0;
}
