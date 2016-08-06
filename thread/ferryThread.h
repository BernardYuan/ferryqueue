//
// Created by Bernard Yuan on 2016-08-02.
//

#ifndef FERRYQUEUE_FERRY_H
#define FERRYQUEUE_FERRY_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#define MAX_LOAD 11
#define SIZE_FERRY 6
#define SIZE_TRUCK 2
#define SIZE_CAR 1
#define MAX_TRUCK 2
#define SAIL_TIME 2e6

#define FERRY_LOADING   1
#define FERRY_BOARDED   2
#define FERRY_SAILING   3
#define FERRY_UNLOADING 4
#endif //FERRYQUEUE_FERRY_H
