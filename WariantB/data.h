#ifndef DATA_H_INCLUDED
#define DATA_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>


//Struktura przechowująca informacje o samochodzie
typedef struct car {
    pthread_t thread;
    int timesCrossed;
    int state;
    int city;
} car;
//Struktura przechowująca informacje o moście, czy coś na nim jest
typedef struct bridgeInfo {
    int carNumber;
    int direction;
} bridgeInfo;

#endif
