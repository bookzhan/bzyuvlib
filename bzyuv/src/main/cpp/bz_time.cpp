/**
 * Created by zhandalin on 2017-04-21 11:26.
 * 说明:
 */

#include "bz_time.h"


int64_t getCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int64_t getMicrosecondTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}