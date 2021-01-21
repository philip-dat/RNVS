#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>
#include <sys/socket.h>

#include <time.h>

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

timespec timespec_from_ntp_TS(uint32_t TS_s, uint32_t TS_f) {
    timespec time;
    time.tv_sec = TS_s - TS_DELTA;

    uint64_t fract = (((uint64_t)(TS_f * 1e9)) >> 32);
    time.tv_nsec   = (uint32_t)fract;

    return time;
}

double ntp_TS_short_to_sec(uint32_t TS_short) {
    return (double)(TS_short >> 16) + (double)(TS_short & 0xFFFF) / (double)(1LL << 16);
}

/**
 * Prints an ntp_connection_t according to the task.
 */
void print_ntp_connection_t(ntp_connection_t *ntp) {
    printf("%s;%d;%f;%f;%f;%f\n", ntp->host, ntp->n, ntp->root_disp,
           ntp->disp, ntp->delay, ntp->offset);
}

/**
 * Calculates the difference between two struct timespec
 */
timespec difference(timespec t1, timespec t2) {
    timespec result;
    result.tv_sec  = t1.tv_sec - t2.tv_sec;
    result.tv_nsec = t1.tv_nsec - t2.tv_nsec;
    return result;
}

double get_seconds(timespec timeval) {
    return timeval.tv_sec + timeval.tv_nsec / 1E9;
}

long long get_nanoseconds(timespec timeval) {
    return timeval.tv_sec * 1E9 + timeval.tv_nsec;
}

void calc_ntp_connection_time(timestmp *time, ntp_connection_t *connection){
    /*
     *  delay = RTT / 2
     *    RTT = (T4 - T1) - (T3 - T2)
     * offset = 1/2 * [(T2 - T1) -  (T4 - T3)]
     */

    timespec T1 = time->T1;
    timespec T2 = time->T2;
    timespec T3 = time->T3;
    timespec T4 = time->T4;

    timespec RTT = difference((difference(T4, T1)), (difference(T3, T2)));
    timespec delay = RTT;
    delay.tv_sec /= 2;
    delay.tv_nsec /= 2;

    timespec OFFSET_PRE = difference((difference(T2, T1)), (difference(T4, T3)));
    timespec offset = OFFSET_PRE;
    offset.tv_sec /= 2;
    offset.tv_nsec /= 2;

    connection->offset = get_seconds(offset);
    connection->delay = get_seconds(delay);
}

void send_info(timestmp *time,ntp_protocol *protocol, int sockfd, struct addrinfo *p, socklen_t addrlen){
    if (sendto(sockfd, protocol, sizeof(ntp_protocol), 0, p->ai_addr, p->ai_addrlen) == -1) {
        perror("talker: sendto");
        exit(1);
    }
    clock_gettime(CLOCK_REALTIME, &(time->T1));
}

void receive_info(timestmp *time, ntp_protocol *protocol, int sockfd, struct addrinfo *p, socklen_t addrlen){
    if (recvfrom(sockfd, protocol, sizeof(ntp_protocol), 0, p->ai_addr, &addrlen) == -1) {
        perror("recvfrom");
        exit(1);
    }
    clock_gettime(CLOCK_REALTIME, &(time->T4));
}


int main(int argc, char* argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Parameters: hostname, port, operation, key");
        exit(1);
    }

    // Amout Requests
    int n = atoi(argv[1]);

    int nserver = argc-2;
    char *ntp_server[nserver];
    char *port = "123";
    //timespec dispersion[NUM];

    // fill serverarray with values
    for (int i = 2; i < argc ; ++i) {
        ntp_server[i - 2] = argv[i];
    }

    for(int server_index = 0; server_index < nserver; server_index++){

        // parse command line arguments
        int status;
        struct addrinfo *servinfo, hints, *p;
        socklen_t addrlen = sizeof(struct sockaddr_storage);

        memset(&hints, 0, sizeof hints); // make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
        //hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

        //get address info
        if ((status = getaddrinfo(ntp_server[server_index], port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
            exit(1);
        }

        // loop through result and try to connect to the first possible
        int socketfd;
        for (p = servinfo; p!= NULL; p = p->ai_next) {
            if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
                perror("Could not establish client socket");
                continue;
            }

            if(connect(socketfd, p->ai_addr, p->ai_addrlen) == -1 ) {
                close(socketfd);
                perror("Client could not connect");
                continue;
            }
            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            exit(1);
        }

        //logic
        for(int request_index = 0; request_index < n; request_index++){
            ntp_connection_t *connection = malloc(sizeof(ntp_connection_t));
            connection->host = ntp_server[server_index];
            connection->n = request_index;
            //TODO immplement protocol
            ntp_protocol *protocol;

            timestmp *time = malloc(sizeof(timestmp));
            //send
            //---> time send T1
            send_info(time, protocol, socketfd, p, addrlen);

            //receive
            //---> time receive T4
            receive_info(time, protocol, socketfd, p, addrlen);

            //TODO unmarshal values from protocol


            time->T2 = timespec_from_ntp_TS(protocol->recvTS_s, protocol->recvTS_f);
            time->T3 = timespec_from_ntp_TS(protocol->transmTS_s, protocol->transmTS_f);

            //TODO get dispersion
            connection->disp;
            connection->root_disp = ntp_TS_short_to_sec(protocol->root_disp);

            calc_ntp_connection_time(time, connection);
            print_ntp_connection_t(connection);
            free(time);
            free(connection);
        }



        freeaddrinfo(servinfo);
        close(socketfd);
    }

    return 0;
}

