#pragma once

#include <time.h>

struct timespec t1, t2;
static inline void start(){ clock_gettime(CLOCK_REALTIME, &t1); }
static inline void stop(){
    clock_gettime(CLOCK_REALTIME, &t2);
    double diff = (t2.tv_sec-t1.tv_sec)*1E3 + (t2.tv_nsec-t1.tv_nsec)/1E6;
    fprintf(stderr, "%lf ms\n\n", diff);
}

