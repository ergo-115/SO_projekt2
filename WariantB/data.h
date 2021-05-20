#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>


typedef struct car {
    pthread_t thread;
    int timesCrossed;
    int state;
    int city;
} car;

typedef struct bridgeInfo {
    int carNumber;
    int direction;
} bridgeInfo;

#endif
