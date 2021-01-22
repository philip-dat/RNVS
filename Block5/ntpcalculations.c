#include <inttypes.h>
#include <stdlib.h>

#include "ntpcalculations.h"

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

double calc_ntp_connection_time(timestmp *time, ntp_connection_t *connection){
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
    /*timespec delay = RTT;
    delay.tv_sec /= 2;
    delay.tv_nsec /= 2;*/

    timespec OFFSET_PRE = difference((difference(T2, T1)), (difference(T4, T3)));
/*    timespec offset = OFFSET_PRE;
    offset.tv_sec /= 2;
    offset.tv_nsec /= 2;*/

    connection->offset = get_seconds(OFFSET_PRE) / 2.0;
    connection->delay = get_seconds(RTT) / 2.0;

    return get_seconds(RTT);
}

void calc_max_min_RTT(double RTTarr[], double *minRTT, double *maxRTT, int request_index){
    int index = (request_index - 8) <= 0 ? 0 : request_index - 8;

    for(int i = index; i < request_index; i++){
        if (RTTarr[i] > *maxRTT) {
            *maxRTT = RTTarr[i];
        } else if (RTTarr[i] < *minRTT) {
            *minRTT = RTTarr[i];
        }
    }
}

void calculate_dispersion(ntp_connection_t *connection, double RTTarr[], int request_index){
    double minRTT, maxRTT = 0;
    calc_max_min_RTT(RTTarr, &minRTT, &maxRTT, request_index);
    connection->disp = minRTT - maxRTT;
}
