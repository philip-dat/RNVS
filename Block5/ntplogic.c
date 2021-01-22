#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include "ntpcalculations.c"
#include "communication.c"
#include "ntplogic.h"

/**
 * Prints an ntp_connection_t according to the task.
 */
void print_ntp_connection_t(ntp_connection_t *ntp) {
    printf("%s;%d;%f;%f;%f;%f\n", ntp->host, ntp->n, ntp->root_disp,
           ntp->disp, ntp->delay, ntp->offset);
}

void ntp_logic(int n, char *host, int socketfd, struct addrinfo *p ,socklen_t addrlen){
    double RTTarr[n];
    for(int request_index = 0; request_index < n; request_index++){
        ntp_connection_t *connection = malloc(sizeof(ntp_connection_t));
        connection->host = host;
        connection->n = request_index;

        ntp_protocol protocol = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        memset(&protocol, 0, sizeof( ntp_protocol ) );
        // set li = 0, vn = 4, mode = 3
        *( ( char * ) &protocol + 0 ) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.


        timestmp *time = malloc(sizeof(timestmp));
        // sets T1 and T4
        communication(time, &protocol, socketfd, p, addrlen);
        time->T2 = timespec_from_ntp_TS(protocol.recvTS_s, protocol.recvTS_f);
        time->T3 = timespec_from_ntp_TS(protocol.transmTS_s, protocol.transmTS_f);

        //calc of RTT and Offset
        RTTarr[request_index] = calc_ntp_connection_time(time, connection);

        //get dispersion
        // connection->disp;
        calculate_dispersion(connection, RTTarr, request_index);
        connection->root_disp = ntp_TS_short_to_sec(protocol.root_disp);

        print_ntp_connection_t(connection);
        free(time);
        free(connection);
        sleep(8); // Wait for 8 seconds
    }

}
